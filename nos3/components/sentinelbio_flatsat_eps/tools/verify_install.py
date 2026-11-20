#!/usr/bin/env python3
"""Verify the NOS3 source and generated artifacts use the custom EPS."""

from __future__ import annotations

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


COMPONENT = "sentinelbio_flatsat_eps"
FSW_MODULE = "sentinelbio_eps"
SIMULATOR = "sentinelbio_flatsat_eps_sim"


def text(path: Path) -> str:
    return path.read_text(encoding="utf-8") if path.is_file() else ""


def check_file(path: Path, checks: list[tuple[str, bool]], failures: list[str]) -> None:
    if not path.is_file():
        failures.append(f"missing: {path}")
        return
    for description, condition in checks:
        if not condition:
            failures.append(f"failed: {description} ({path})")


def verify(root: Path, built: bool, generated: bool = False) -> list[str]:
    failures: list[str] = []
    component = root / "components" / COMPONENT
    if not component.is_dir():
        failures.append(f"missing component: {component}")
        return failures

    targets = text(root / "cfg/nos3_defs/targets.cmake")
    check_file(
        root / "cfg/nos3_defs/targets.cmake",
        [
            ("custom cFS target is enabled", f"{COMPONENT}/fsw/cfs" in targets),
            ("generic EPS cFS target is removed", "generic_eps/fsw/cfs" not in targets),
        ],
        failures,
    )

    startup = text(root / "cfg/nos3_defs/cpu1_cfe_es_startup.scr")
    check_file(
        root / "cfg/nos3_defs/cpu1_cfe_es_startup.scr",
        [
            ("custom EPS startup line is present", f"CFE_APP, {FSW_MODULE}, EPS_AppMain" in startup),
            ("generic EPS startup line is removed", "CFE_APP, generic_eps" not in startup),
        ],
        failures,
    )

    simulator_candidates = set((root / "cfg/sims").glob("*.xml"))
    simulator_candidates.update((root / "cfg/build/sims").glob("*.xml"))
    simulator_paths = [
        path
        for path in sorted(simulator_candidates)
        if path.is_file() and ("<name>generic-eps-sim</name>" in text(path) or
                               "<name>sentinelbio_flatsat_eps_sim</name>" in text(path))
    ]
    if not simulator_paths:
        failures.append(f"no simulator XML found under {root / 'cfg/sims'}")
    for path in simulator_paths:
        if not path.is_file():
            failures.append(f"missing: {path}")
            continue
        try:
            content = text(path)
            custom_matches = re.findall(
                r"(?ms)<simulator>\s*<name>sentinelbio_flatsat_eps_sim</name>.*?</simulator>", content
            )
            generic_matches = re.findall(
                r"(?ms)<simulator>\s*<name>generic-eps-sim</name>.*?</simulator>", content
            )
            custom = [ET.fromstring(match) for match in custom_matches]
            generic = generic_matches
            if len(custom) != 1:
                failures.append(f"{path}: expected one {SIMULATOR}, found {len(custom)}")
            if generic:
                failures.append(f"{path}: generic-eps-sim is still present")
            if custom:
                node = custom[0]
                if node.findtext("library") != f"lib{COMPONENT}_sim.so":
                    failures.append(f"{path}: custom simulator library is wrong")
                if node.findtext("hardware-model/type") != "SENTINELBIO_FLATSAT_EPS":
                    failures.append(f"{path}: custom hardware model type is wrong")
                i2c = [
                    connection
                    for connection in node.findall("hardware-model/connections/connection")
                    if connection.findtext("type") == "i2c"
                ]
                expected = [("i2c_1", "0x40", "12v"), ("i2c_2", "0x41", "5v"), ("i2c_3", "0x44", "3v3")]
                actual = [
                    (item.findtext("bus-name"), item.findtext("bus-address"), item.findtext("rail"))
                    for item in i2c
                ]
                if actual != expected:
                    failures.append(f"{path}: I2C connections are {actual}, expected {expected}")
        except ET.ParseError as exc:
            failures.append(f"{path}: invalid XML: {exc}")

    configure = text(root / "scripts/cfg/configure.py")
    check_file(
        root / "scripts/cfg/configure.py",
        [(f"configure.py recognizes {SIMULATOR}", f"{SIMULATOR}</name>" in configure)],
        failures,
    )

    for relative in (
        "scripts/fsw/fsw_cfs_launch.sh",
        "scripts/fsw/fsw_fprime_launch.sh",
        "scripts/fsw/fsw_cfs_launch_multiple_sc.sh",
        "scripts/fsw/fsw_cfs_multipleGSW_launch.sh",
        "scripts/fsw/launch_sat.sh",
        "scripts/ci_launch.sh",
    ):
        path = root / relative
        if path.is_file() and "generic-eps-sim" in text(path):
            failures.append(f"{path}: launch script still starts generic-eps-sim")

    table_root = root / "cfg/nos3_defs/tables"
    if table_root.is_dir():
        for path in table_root.glob("*.c"):
            content = text(path)
            if "GENERIC_EPS" in content or "generic_eps" in content:
                failures.append(f"{path}: stored table still references generic EPS")

    yamcs_paths = list((root / "scripts/cfg").glob("yamcs*.yaml"))
    yamcs_dir = root / "gsw/yamcs"
    if yamcs_dir.is_dir():
        yamcs_paths.extend(yamcs_dir.rglob("*.yaml"))
    if yamcs_paths and not any(COMPONENT in text(path) for path in yamcs_paths):
        failures.append("YAMCS configuration does not reference the custom EPS XTCE")
    for path in yamcs_paths:
        if "generic_eps/gsw/generic_eps.xtce" in text(path):
            failures.append(f"{path}: YAMCS still references generic EPS XTCE")

    for relative in (
        "scripts/cfg/declare_cosmos_target.py",
        "scripts/cfg/configure_cosmos_target.py",
        "scripts/cfg/configure_test_runner.py",
    ):
        path = root / relative
        if path.is_file():
            content = text(path)
            expected_entry = f'"{COMPONENT}_test": "eps"' if path.name == "configure_test_runner.py" else f'"{COMPONENT}": "eps"'
            generic_entry = '"generic_eps_test": "eps"' if path.name == "configure_test_runner.py" else '"generic_eps": "eps"'
            if expected_entry not in content:
                failures.append(f"{path}: custom EPS component map entry is missing")
            if generic_entry in content:
                failures.append(f"{path}: generic EPS component map entry remains")

    for relative in (
        "gsw/cosmos/config/system/stash/system.txt",
        "gsw/cosmos/config/system/system.txt",
        "gsw/cosmos/config/tools/cmd_tlm_server/stash/cmd_tlm_server.txt",
        "gsw/cosmos/config/tools/cmd_tlm_server/cmd_tlm_server.txt",
    ):
        path = root / relative
        if path.is_file():
            content = text(path)
            if "TARGET GENERIC_EPS" in content or "COMPONENTS/GENERIC_EPS" in content:
                failures.append(f"{path}: COSMOS still declares generic EPS")
            if "SENTINELBIO_FLATSAT_EPS" not in content:
                failures.append(f"{path}: COSMOS custom EPS target is missing")

    for relative in (
        "gsw/cosmos/config/tools/test_runner/stash/test_runner.txt",
        "gsw/cosmos/config/tools/test_runner/test_runner.txt",
    ):
        path = root / relative
        if path.is_file() and "REQUIRE_UTILITY 'generic_eps_test'" in text(path):
            failures.append(f"{path}: generic EPS test utility is still required")

    if generated or built:
        for rel, token in [("nos3_defs/targets.cmake", COMPONENT+"/fsw/cfs"),
                           ("nos3_defs/cpu1_cfe_es_startup.scr", "CFE_APP, sentinelbio_eps, EPS_AppMain"),
                           ("nos3_defs/tables/sch_def_msgtbl.c", "SENTINELBIO_FLATSAT_EPS_REQ_HK_MID"),
                           ("nos3_defs/tables/to_config.c", "SENTINELBIO_FLATSAT_EPS_HK_TLM_MID"),
                           ("launch.sh", SIMULATOR)]:
            path = root/"cfg/build"/rel
            content = text(path)
            active = content.split("!",1)[0] if rel.endswith("startup.scr") else content
            if token not in active: failures.append(f"{path}: missing active custom EPS selection")
            if "generic_eps" in active or "generic-eps-sim" in active:
                failures.append(f"{path}: stale generic EPS selection")
        path = root/"cfg/build/sims/nos3-simulator.xml"
        try:
            blocks = re.findall(r"(?s)<simulator>\s*<name>sentinelbio_flatsat_eps_sim</name>.*?</simulator>", text(path))
            nodes = [ET.fromstring(block) for block in blocks]
            if len(nodes) != 1 or nodes[0].findtext("active") != "true":
                failures.append(f"{path}: custom simulator must be active")
        except (OSError, ET.ParseError) as exc: failures.append(str(exc))

    if built:
        fsw_candidates = [
            root / "fsw/build/exe/cpu1/cf/sentinelbio_eps.so",
            root / "fsw/build/exe/cpu1/cf/sentinelbio_eps.dll",
        ]
        if not any(path.is_file() for path in fsw_candidates):
            failures.append("built cFS module not found under fsw/build/exe/cpu1/cf")
        sim_candidates = [
            root / "sims/build/lib/libsentinelbio_flatsat_eps_sim.so",
            root / "sims/build/lib/libsentinelbio_flatsat_eps_sim.dylib",
            root / "sims/build/bin/libsentinelbio_flatsat_eps_sim.so",
        ]
        if not any(path.is_file() for path in sim_candidates):
            failures.append("built simulator library not found under sims/build")

    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("nos3_root", nargs="?", type=Path, default=Path.cwd())
    parser.add_argument("--built", action="store_true", help="also require the FSW and simulator libraries")
    parser.add_argument("--generated", action="store_true", help="require regenerated active runtime selections")
    args = parser.parse_args()
    failures = verify(args.nos3_root.resolve(), args.built, args.generated)
    if failures:
        print("VERIFY FAILED")
        for failure in failures:
            print(f"- {failure}")
        return 1
    print(f"VERIFY OK: {COMPONENT} is the configured EPS component")
    if args.built:
        print("VERIFY OK: cFS and simulator libraries are present")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
