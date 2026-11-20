#!/usr/bin/env python3
"""Install the Sentinel Bio FlatSat EPS into an existing NOS3 checkout.

The installer edits the existing checkout in place.  It never moves the
generic component directory and it never relies on a backup rename.  That is
intentional: a NOS3 checkout may be bind-mounted or otherwise unable to rename
directories even when individual files can be updated.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
import textwrap
import tempfile
import datetime
import json
import os
from pathlib import Path


COMPONENT_NAME = "sentinelbio_flatsat_eps"
FSW_MODULE_NAME = "sentinelbio_eps"
SIMULATOR_NAME = "sentinelbio_flatsat_eps_sim"
SIMULATOR_TYPE = "SENTINELBIO_FLATSAT_EPS"
APP_STARTUP_LINE = (
    "CFE_APP, sentinelbio_eps, EPS_AppMain, SB_EPS, 63, 32768, 0x0, 0;"
)
LAUNCH_FILES = (
    "scripts/fsw/fsw_cfs_launch.sh",
    "scripts/fsw/fsw_fprime_launch.sh",
    "scripts/fsw/fsw_cfs_launch_multiple_sc.sh",
    "scripts/fsw/fsw_cfs_multipleGSW_launch.sh",
    "scripts/fsw/launch_sat.sh",
    "scripts/ci_launch.sh",
)
COSMOS_FILES = (
    "gsw/cosmos/config/system/stash/system.txt",
    "gsw/cosmos/config/system/system.txt",
    "gsw/cosmos/config/tools/cmd_tlm_server/stash/cmd_tlm_server.txt",
    "gsw/cosmos/config/tools/cmd_tlm_server/cmd_tlm_server.txt",
)
TEST_RUNNER_FILES = (
    "gsw/cosmos/config/tools/test_runner/stash/test_runner.txt",
    "gsw/cosmos/config/tools/test_runner/test_runner.txt",
)


class InstallError(RuntimeError):
    pass


def read(path: Path) -> str:
    if not path.is_file():
        raise InstallError(f"Missing NOS3 file: {path}")
    return path.read_text(encoding="utf-8")


def write_if_changed(path: Path, content: str, changed: list[Path]) -> None:
    current = read(path)
    if current == content:
        return
    try:
        path.write_text(content, encoding="utf-8")
    except PermissionError as exc:
        raise InstallError(
            f"Cannot update {path}: {exc}. The checkout may be mounted read-only; "
            "run this installer in the writable NOS3 environment."
        ) from exc
    changed.append(path)


def replace_once(path: Path, old: str, new: str, changed: list[Path]) -> None:
    content = read(path)
    if old not in content:
        if new in content:
            return
        raise InstallError(f"Expected text was not found in {path}: {old!r}")
    write_if_changed(path, content.replace(old, new, 1), changed)


def patch_targets(root: Path, changed: list[Path]) -> None:
    path = root / "cfg/nos3_defs/targets.cmake"
    lines = read(path).splitlines(keepends=True)
    has_custom = any(
        f"{COMPONENT_NAME}/fsw/cfs" in line and not line.lstrip().startswith("#")
        for line in lines
    )
    output: list[str] = []
    replacement_done = False
    for line in lines:
        if "generic_eps/fsw/cfs" in line and not line.lstrip().startswith("#"):
            if not has_custom and not replacement_done:
                newline = "\n" if line.endswith("\n") else ""
                output.append(f"        {COMPONENT_NAME}/fsw/cfs{newline}")
                replacement_done = True
            # Drop any duplicate generic EPS target left by an earlier
            # partial installation.
        else:
            output.append(line)
    if not has_custom and not replacement_done:
        anchor = next((i for i, line in enumerate(output) if "generic_css/fsw/cfs" in line), None)
        if anchor is None:
            raise InstallError(f"Could not find the EPS target insertion point in {path}")
        output.insert(anchor + 1, f"        {COMPONENT_NAME}/fsw/cfs\n")
    write_if_changed(path, "".join(output), changed)


def patch_startup(root: Path, changed: list[Path]) -> None:
    path = root / "cfg/nos3_defs/cpu1_cfe_es_startup.scr"
    lines = read(path).splitlines(keepends=True)
    output: list[str] = []
    found = False
    for line in lines:
        if (
            "CFE_APP" in line
            and "EPS_AppMain" in line
            and ("generic_eps" in line or COMPONENT_NAME in line or FSW_MODULE_NAME in line)
        ):
            output.append(APP_STARTUP_LINE + ("\n" if line.endswith("\n") else ""))
            found = True
        else:
            output.append(line)
    if not found and not any(FSW_MODULE_NAME in line and "EPS_AppMain" in line for line in output):
        raise InstallError(f"Could not find the generic EPS CFE_APP entry in {path}")
    write_if_changed(path, "".join(output), changed)


def replace_eps_simulator_block(path: Path, block: str, changed: list[Path]) -> None:
    content = read(path)
    pattern = re.compile(
        r"(?ms)^(?P<indent>[ \t]*)<simulator>\s*"
        r"<name>(?:generic-eps-sim|sentinelbio_flatsat_eps_sim)</name>.*?"
        r"^[ \t]*</simulator>"
    )
    matches = list(pattern.finditer(content))
    if len(matches) != 1:
        raise InstallError(
            f"Expected exactly one generic/custom EPS simulator block in {path}; found {len(matches)}"
        )
    match = matches[0]
    indentation = match.group("indent")
    replacement = textwrap.indent(block.strip(), indentation)
    new_content = content[: match.start()] + replacement + content[match.end() :]
    write_if_changed(path, new_content, changed)


def patch_simulator_xml(root: Path, component: Path, changed: list[Path]) -> None:
    block = read(component / "sim/cfg/nos3-eps-simulator.xml")
    # The standard configurations use nos3-simulator*.xml, while the
    # multi-spacecraft configurations use sc-*-nos3-simulator.xml.  Include
    # both source and already-generated configurations, but skip unrelated
    # XML files such as sim_log_config.xml.
    candidates = set((root / "cfg/sims").glob("*.xml"))
    candidates.update((root / "cfg/build/sims").glob("*.xml"))
    paths = [
        path
        for path in sorted(candidates)
        if path.is_file() and ("<name>generic-eps-sim</name>" in read(path) or
                               "<name>sentinelbio_flatsat_eps_sim</name>" in read(path))
    ]
    if not paths:
        raise InstallError(f"No EPS simulator XML files were found under {root / 'cfg'}")
    for path in paths:
        replace_eps_simulator_block(path, block, changed)


def patch_configure_and_launchers(root: Path, changed: list[Path]) -> None:
    configure = root / "scripts/cfg/configure.py"
    replace_once(
        configure,
        "if line.find('eps-sim</name>') != -1:",
        f"if line.find('{SIMULATOR_NAME}</name>') != -1:",
        changed,
    )
    for relative in LAUNCH_FILES:
        path = root / relative
        if path.is_file():
            content = read(path)
            updated = content.replace("generic-eps-sim", SIMULATOR_NAME)
            write_if_changed(path, updated, changed)


def patch_component_tables(root: Path, changed: list[Path]) -> None:
    table_roots = [root / "cfg/nos3_defs/tables", root / "cfg/build/nos3_defs/tables"]
    for table_root in table_roots:
        if not table_root.is_dir():
            continue
        for path in sorted(table_root.glob("*.c")):
            content = read(path)
            if path.name == "lc_def_wdt.c":
                # The old battery watchpoints read byte 20 of a different packet.
                for number in (27, 28):
                    pattern = rf"/\* #{number} \(EPS BATTERY_VOLTAGE.*?\*/\s*\{{.*?\}},"
                    content = re.sub(pattern, f"/* #{number} (unused: FlatSat has no battery) */\n    {{ .DataType = LC_WATCH_NOT_USED }},", content, flags=re.S)
            updated = content.replace("GENERIC_EPS", "SENTINELBIO_FLATSAT_EPS")
            updated = updated.replace("generic_eps", COMPONENT_NAME)
            write_if_changed(path, updated, changed)


def patch_yamcs(root: Path, changed: list[Path]) -> None:
    paths = [root / "scripts/cfg"]
    generated = root / "gsw/yamcs"
    if generated.is_dir():
        paths.append(generated)
    for base in paths:
        for path in sorted(base.rglob("*.yaml")):
            content = read(path)
            updated = content.replace(
                "${env.COMPONENT_DIR}/generic_eps/gsw/generic_eps.xtce",
                "${env.COMPONENT_DIR}/sentinelbio_flatsat_eps/gsw/sentinelbio_flatsat_eps.xtce",
            )
            write_if_changed(path, updated, changed)


def patch_cosmos_maps(root: Path, changed: list[Path]) -> None:
    for relative in (
        "scripts/cfg/declare_cosmos_target.py",
        "scripts/cfg/configure_cosmos_target.py",
        "scripts/cfg/configure_test_runner.py",
    ):
        path = root / relative
        if not path.is_file():
            continue
        content = read(path)
        if path.name == "configure_test_runner.py":
            content = content.replace('    "generic_eps_test": "eps",\n', "")
            component_map_entry = f'    "{COMPONENT_NAME}_test": "eps",\n'
            anchor = '    "generic_css_test": "css",\n'
        else:
            content = content.replace('    "generic_eps": "eps",\n', "")
            component_map_entry = f'    "{COMPONENT_NAME}": "eps",\n'
            anchor = '    "generic_css": "css",\n'
        if component_map_entry not in content:
            if anchor not in content:
                raise InstallError(f"Could not find the component map insertion point in {path}")
            content = content.replace(anchor, anchor + component_map_entry, 1)
        write_if_changed(path, content, changed)

    for relative in COSMOS_FILES:
        path = root / relative
        if not path.is_file():
            continue
        content = read(path)
        content = content.replace(
            "../../COMPONENTS/GENERIC_EPS_RADIO GENERIC_EPS_RADIO",
            "../../COMPONENTS/SENTINELBIO_FLATSAT_EPS_RADIO SENTINELBIO_FLATSAT_EPS_RADIO",
        )
        content = content.replace(
            "../../COMPONENTS/GENERIC_EPS GENERIC_EPS",
            "../../COMPONENTS/SENTINELBIO_FLATSAT_EPS SENTINELBIO_FLATSAT_EPS",
        )
        content = content.replace("TARGET GENERIC_EPS_RADIO", "TARGET SENTINELBIO_FLATSAT_EPS_RADIO")
        content = content.replace("TARGET GENERIC_EPS", "TARGET SENTINELBIO_FLATSAT_EPS")
        write_if_changed(path, content, changed)

    # The custom component intentionally does not ship the generic EPS test
    # utility.  Remove the stale requirement from both the stash and an
    # already-generated COSMOS test-runner file; otherwise a later `make
    # config` can fail while loading a utility that is no longer selected.
    for relative in TEST_RUNNER_FILES:
        path = root / relative
        if path.is_file():
            content = read(path)
            content = content.replace("REQUIRE_UTILITY 'generic_eps_test'\n", "")
            write_if_changed(path, content, changed)


def install(root: Path, component: Path, dry_run=False, component_only=False) -> list[Path]:
    root, component = root.resolve(), component.resolve()
    if not root.is_dir():
        raise InstallError(f"Destination does not exist: {root}")
    required = ["Makefile", "scripts/cfg/configure.py", "scripts/cfg/declare_cosmos_target.py",
                "scripts/cfg/configure_cosmos_target.py", "scripts/fsw/launch_sat.sh",
                "cfg/nos3_defs/targets.cmake", "cfg/nos3_defs/cpu1_cfe_es_startup.scr",
                "cfg/sims/nos3-simulator.xml", "components/ComponentSettings.cmake", "sims/CMakeLists.txt"]
    missing = [x for x in required if not (root/x).is_file()]
    if missing and not component_only:
        raise InstallError("Incomplete NOS3 source checkout; missing: " + ", ".join(missing) +
                           ". Use --component-only only to restore source; it does not integrate/build NOS3.")
    planned = {}
    with tempfile.TemporaryDirectory(prefix="sb-eps-plan-") as temporary:
        stage = Path(temporary)
        if not component_only:
            for directory in ("cfg", "scripts", "gsw/cosmos/config"):
                if (root/directory).is_dir():
                    shutil.copytree(root/directory, stage/directory)
            for path in (root/"gsw/yamcs").rglob("*.yaml"):
                target = stage/path.relative_to(root)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, target)
            changed = []
            patch_targets(stage, changed)
            patch_startup(stage, changed)
            patch_simulator_xml(stage, component, changed)
            patch_configure_and_launchers(stage, changed)
            patch_component_tables(stage, changed)
            patch_yamcs(stage, changed)
            patch_cosmos_maps(stage, changed)
            for path in changed:
                planned[path.relative_to(stage)] = path.read_bytes()
        for path in component.rglob("*"):
            if path.is_file() and "__pycache__" not in path.parts and path.suffix not in (".pyc", ".gcno", ".gcda"):
                planned[Path("components")/COMPONENT_NAME/path.relative_to(component)] = path.read_bytes()
    planned = {p: b for p,b in planned.items() if not (root/p).is_file() or (root/p).read_bytes() != b}
    for relative in planned:
        target = root/relative
        if target.is_symlink() or any(x.is_symlink() for x in target.parents if x != root.parent):
            raise InstallError(f"Refusing to replace through a symlink: {target}")
        if target.exists() and not target.is_file():
            raise InstallError(f"Expected a file: {target}")
    if dry_run:
        for path in sorted(planned): print(f"WOULD WRITE {path}")
        return list(planned)
    if not planned: return []
    backup = root/"sentinelbio_eps_backups"/datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    backup.mkdir(parents=True)
    manifest = []
    for relative in planned:
        target = root/relative
        existed = target.is_file()
        manifest.append({"path": str(relative), "existed": existed})
        if existed:
            saved = backup/"files"/relative
            saved.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(target, saved)
    (backup/"manifest.json").write_text(json.dumps(manifest, indent=2))
    applied = []
    try:
        for relative, data in planned.items():
            target = root/relative
            target.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile(dir=target.parent, delete=False) as output:
                output.write(data)
                name = output.name
            os.chmod(name, target.stat().st_mode & 0o777 if target.exists() else 0o644)
            os.replace(name, target)
            applied.append(relative)
    except OSError:
        for item in reversed(manifest):
            relative = Path(item["path"])
            if relative in applied:
                if item["existed"]: shutil.copy2(backup/"files"/relative, root/relative)
                else: (root/relative).unlink()
        raise
    print(f"Backup: {backup}")
    return list(planned)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("nos3_root", type=Path)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--component-only", action="store_true", help="Restore component files without mission integration")
    args = parser.parse_args()
    try:
        changed = install(args.nos3_root, Path(__file__).resolve().parents[1], args.dry_run, args.component_only)
    except (InstallError, OSError) as exc:
        print(f"INSTALL FAILED: {exc}", file=sys.stderr)
        return 2
    print(f"{'Planned' if args.dry_run else 'Wrote'} {len(changed)} changed files.")
    if args.component_only:
        print("Component source only; mission configuration and builds have NOT been integrated.")
    else:
        print("Next: verify source, regenerate with make config, rebuild FSW/sim/GSW, then verify --built.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
