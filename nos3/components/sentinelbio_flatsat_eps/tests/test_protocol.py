"""Dependency-free checks for the Sentinel Bio FlatSat EPS model."""

from pathlib import Path
import re
import xml.etree.ElementTree as ET

PACKAGE_ROOT = Path(__file__).resolve().parents[1]


def ina219_values(voltage, current, shunt_ohms=0.1, calibration=4096):
    """Return the simulated INA219 raw values and cFS engineering values."""
    shunt_code = round(current * shunt_ohms / 0.00001)
    bus_code = round(voltage / 0.004)
    current_code = round(shunt_code * calibration / 4096) if calibration else 0
    power_code = int(current_code * bus_code / 5000) if calibration else 0
    return {
        "shunt_uv": shunt_code * 10,
        "bus_mv": (bus_code * 4),
        "current_ma": round(current_code * 0.1),
        "power_mw": power_code * 2,
    }


def test_simulator_xml_is_well_formed_and_has_three_ina219s():
    path = PACKAGE_ROOT / "sim/cfg/nos3-eps-simulator.xml"
    root = ET.parse(path).getroot()
    assert root.findtext("name") == "sentinelbio_flatsat_eps_sim"
    connections = root.find("hardware-model/connections")
    i2c = [node for node in connections if node.findtext("type") == "i2c"]
    assert len(i2c) == 3
    assert [(node.findtext("bus-name"), node.findtext("bus-address")) for node in i2c] == [
        ("i2c_1", "0x40"),
        ("i2c_2", "0x41"),
        ("i2c_3", "0x44"),
    ]
    assert [node.findtext("rail") for node in i2c] == ["12v", "5v", "3v3"]


def test_ina219_register_scaling_matches_defaults():
    assert ina219_values(12.0, 0.25) == {
        "shunt_uv": 25000,
        "bus_mv": 12000,
        "current_ma": 250,
        "power_mw": 3000,
    }
    assert ina219_values(5.0, 0.50) == {
        "shunt_uv": 50000,
        "bus_mv": 5000,
        "current_ma": 500,
        "power_mw": 2500,
    }
    assert ina219_values(3.3, 0.75) == {
        "shunt_uv": 75000,
        "bus_mv": 3300,
        "current_ma": 750,
        "power_mw": 2474,
    }


def test_telemetry_ids_and_signed_fields_are_aligned():
    text = (PACKAGE_ROOT / "gsw/SENTINELBIO_FLATSAT_EPS/cmd_tlm/SENTINELBIO_FLATSAT_EPS_TLM.txt").read_text()
    assert "0x09E0" in text
    assert len(re.findall(r"APPEND_ITEM\s+RAIL_[^ ]+_SHUNT_UV\s+32 INT", text)) == 3
    assert len(re.findall(r"APPEND_ITEM\s+RAIL_[^ ]+_CURRENT_MA\s+16 INT", text)) == 3


def test_xtce_is_well_formed_and_uses_the_custom_stream_id():
    xtce = PACKAGE_ROOT / "gsw/sentinelbio_flatsat_eps.xtce"
    root = ET.parse(xtce).getroot()
    assert root.attrib["name"] == "SENTINELBIO_FLATSAT_EPS"
    contents = xtce.read_text()
    assert "2528" in contents
    for command in (
        "SENTINELBIO_FLATSAT_EPS_NOOP_CC",
        "SENTINELBIO_FLATSAT_EPS_RESET_COUNTERS_CC",
        "SENTINELBIO_FLATSAT_EPS_REQ_HK",
    ):
        assert command in contents


def test_cfs_module_filename_fits_nos3_osal_limit():
    cmake = (PACKAGE_ROOT / "fsw/cfs/CMakeLists.txt").read_text()
    assert "add_cfe_app(sentinelbio_eps" in cmake
    assert "add_cfe_app(sentinelbio_flatsat_eps" not in cmake
    assert len("sentinelbio_eps.so") < 20  # OS_MAX_FILE_NAME includes the NUL byte.


if __name__ == "__main__":
    tests = sorted(name for name in globals() if name.startswith("test_"))
    for name in tests:
        globals()[name]()
        print(f"PASS {name}")
