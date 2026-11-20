#!/usr/bin/env python3
"""Cross-check the shipped ground definitions against the C packet declarations.

Runs with Python 3 + a C compiler. The compiler test uses small cFE/hwlib ABI
shims for the supported 16-byte TM / 8-byte TC header profile; it is not a cFS
runtime test. The actual application and device packet headers are included.
Optional: --nos3-root PATH validates XTCE with that checkout's XTCE 1.2 XSD
(requires lxml) and checks its common CCSDS telemetry header.
"""
from pathlib import Path
import argparse
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
TARGET = 'SENTINELBIO_FLATSAT_EPS'
NS = {'x': 'http://www.omg.org/spec/XTCE/20180204'}
NOS3_ROOT = None


def cosmos_items():
    text = (ROOT / f'gsw/{TARGET}/cmd_tlm/{TARGET}_TLM.txt').read_text()
    endian = None
    fields = []
    for line in text.splitlines():
        parts = shlex.split(line, comments=True)
        if not parts:
            continue
        if parts[0] == 'TELEMETRY':
            endian = parts[3]
        elif parts[0] in ('APPEND_ITEM', 'APPEND_ID_ITEM'):
            fields.append((parts[1], int(parts[2]), parts[3] == 'INT',
                           parts[-1] if parts[-1].endswith('_ENDIAN') else endian))
    return fields


def xtce_items():
    root = ET.parse(ROOT / 'gsw/sentinelbio_flatsat_eps.xtce').getroot()
    meta = root.find('x:SpaceSystem/x:TelemetryMetaData', NS)
    types = {x.attrib['name']: x for x in meta.find('x:ParameterTypeSet', NS)}
    params = {x.attrib['name']: x.attrib['parameterTypeRef']
              for x in meta.find('x:ParameterSet', NS)}
    fields = []
    for ref in meta.find('x:ContainerSet/x:SequenceContainer/x:EntryList', NS):
        name = ref.attrib['parameterRef']
        enc = types[params[name]].find('x:IntegerDataEncoding', NS)
        fields.append((name, int(enc.attrib['sizeInBits']),
                       enc.attrib.get('encoding') == 'twosComplement',
                       'LITTLE_ENDIAN' if enc.attrib.get('byteOrder') ==
                       'leastSignificantByteFirst' else 'BIG_ENDIAN'))
    return fields


def decode(packet, fields, offset=0):
    values = {}
    for name, bits, signed, endian in fields:
        assert bits % 8 == 0
        size = bits // 8
        values[name] = int.from_bytes(packet[offset:offset + size],
                                     'little' if endian == 'LITTLE_ENDIAN' else 'big',
                                     signed=signed)
        offset += size
    return values, offset


class GroundTests(unittest.TestCase):
    def test_telemetry_definitions_agree(self):
        cosmos = cosmos_items()
        xtce = xtce_items()
        # Endianness is immaterial for one-byte fields.
        normalize = lambda f: [(n, b, s, e if b > 8 else '') for n, b, s, e in f]
        self.assertEqual(normalize(cosmos[6:]), normalize(xtce))
        self.assertEqual(sum(f[1] for f in cosmos), 64 * 8)
        self.assertEqual(sum(f[1] for f in xtce), 48 * 8)
        self.assertEqual([f[1] for f in cosmos[:6]], [16, 16, 16, 32, 16, 32])
        self.assertTrue(all(f[3] == 'BIG_ENDIAN' for f in cosmos[:5]))
        tree = ET.parse(ROOT / 'gsw/sentinelbio_flatsat_eps.xtce')
        restriction = tree.find('.//x:Comparison', NS)
        self.assertEqual(int(restriction.attrib['value']), 0x09E0)

    def test_command_ids_lengths_function_codes_and_defaults(self):
        source = (ROOT / 'fsw/cfs/platform_inc/sentinelbio_flatsat_eps_msgids.h').read_text()
        mids = {name: int(value, 0) for name, value in re.findall(
            r'^#define\s+(\w+_MID)\s+(0x[0-9A-Fa-f]+)', source, re.M)}
        msg = (ROOT / 'fsw/cfs/src/sentinelbio_flatsat_eps_msg.h').read_text()
        codes = {name: int(value) for name, value in re.findall(
            r'^#define\s+(\w+_CC)\s+(\d+)', msg, re.M)}
        cosmos = {}
        current = None
        for line in (ROOT / f'gsw/{TARGET}/cmd_tlm/{TARGET}_CMD.txt').read_text().splitlines():
            parts = shlex.split(line, comments=True)
            if not parts:
                continue
            if parts[0] == 'COMMAND':
                current = cosmos.setdefault(parts[2], {})
            elif parts[0] == 'APPEND_PARAMETER':
                current[parts[1]] = (int(parts[2]), int(parts[6], 0))
                if not parts[4].startswith('MIN_'):
                    self.assertLessEqual(int(parts[4], 0), int(parts[6], 0))
                    self.assertLessEqual(int(parts[6], 0), int(parts[5], 0))
        tree = ET.parse(ROOT / 'gsw/sentinelbio_flatsat_eps.xtce')
        commands = tree.findall('.//x:MetaCommand', NS)
        self.assertEqual(set(cosmos), {c.attrib['name'] for c in commands})
        for command in commands:
            name = command.attrib['name']
            assignment = {a.attrib['argumentName']: int(a.attrib['argumentValue'])
                          for a in command.findall('.//x:ArgumentAssignment', NS)}
            fields = cosmos[name]
            request = name.endswith('_REQ_HK')
            self.assertEqual(fields['CCSDS_STREAMID'][1],
                             mids[TARGET + ('_REQ_HK_MID' if request else '_CMD_MID')])
            self.assertEqual(fields['CCSDS_FC'][1], 0 if request else codes[name])
            for key in ('CCSDS_STREAMID', 'CCSDS_FC'):
                self.assertEqual(assignment[key], fields[key][1])
            size = sum(bits for bits, _ in fields.values()) // 8
            self.assertEqual(fields['CCSDS_LENGTH'][1], size - 7)
            self.assertEqual(size, 10 if name.endswith('_SWITCH_CC') else 8)
            args = command.find('x:ArgumentList', NS)
            self.assertTrue(args is None or len(args) > 0, 'Empty ArgumentList is invalid XTCE')
        for kind in tree.findall('.//x:EnumeratedArgumentType', NS):
            labels = {e.attrib['label'] for e in kind.findall('x:EnumerationList/x:Enumeration', NS)}
            self.assertIn(kind.attrib['initialValue'], labels)

    @unittest.skipUnless(shutil.which('cc'), 'C compiler unavailable')
    def test_actual_c_packet_decodes_in_both_ground_formats(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            (tmp / 'cfe.h').write_text('''#include <stdint.h>
typedef uint8_t uint8;
typedef struct { uint8_t bytes[8]; } CFE_MSG_CommandHeader_t;
typedef struct { uint8_t bytes[16]; } CFE_MSG_TelemetryHeader_t;
''')
            (tmp / 'hwlib.h').write_text('''#include <stdint.h>
typedef struct { int unused; } i2c_bus_info_t;
''')
            (tmp / 'packet.c').write_text('''#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "sentinelbio_flatsat_eps_msg.h"
#define ASSERT(c) _Static_assert(c, #c)
ASSERT(sizeof(SENTINELBIO_FLATSAT_EPS_NoArgs_cmd_t) == 8);
ASSERT(sizeof(SENTINELBIO_FLATSAT_EPS_Switch_cmd_t) == 10);
ASSERT(sizeof(SENTINELBIO_FLATSAT_EPS_Rail_tlm_t) == 14);
ASSERT(sizeof(SENTINELBIO_FLATSAT_EPS_Hk_tlm_t) == 64);
ASSERT(offsetof(SENTINELBIO_FLATSAT_EPS_Hk_tlm_t, DeviceHK) == 20);
ASSERT(offsetof(SENTINELBIO_FLATSAT_EPS_Hk_tlm_t, RailValidMask) == 62);
ASSERT(offsetof(SENTINELBIO_FLATSAT_EPS_Hk_tlm_t, I2CReadyMask) == 63);
int main(void) {
    SENTINELBIO_FLATSAT_EPS_Hk_tlm_t packet = {0};
    const unsigned char header[16] = {0x09,0xe0,0xc1,0x23,0,57,0,0,0,2,0,3,0,0,0,0};
    const SENTINELBIO_FLATSAT_EPS_Rail_tlm_t r0 = {12000,-25000,-250,3000,0x399f,4096};
    const SENTINELBIO_FLATSAT_EPS_Rail_tlm_t r1 = {5000,50000,500,2500,0x399f,4096};
    const SENTINELBIO_FLATSAT_EPS_Rail_tlm_t r2 = {3300,75000,750,2476,0x399f,4096};
    memcpy(&packet, header, sizeof(header));
    packet.CommandErrorCount=11; packet.CommandCount=22;
    packet.DeviceErrorCount=33; packet.DeviceCount=44;
    packet.DeviceHK.Rail12V=r0; packet.DeviceHK.Rail5V=r1; packet.DeviceHK.Rail3V3=r2;
    packet.RailValidMask=5; packet.I2CReadyMask=7;
    return fwrite(&packet, sizeof(packet), 1, stdout) == 1 ? 0 : 1;
}
''')
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-I' + str(tmp), '-I' + str(ROOT / 'fsw/cfs/src'),
                            '-I' + str(ROOT / 'fsw/shared'),
                            '-I' + str(ROOT / 'fsw/cfs/platform_inc'),
                            str(tmp / 'packet.c'), '-o', str(tmp / 'packet')], check=True)
            packet = subprocess.check_output([str(tmp / 'packet')])
        self.assertEqual(sys.byteorder, 'little', 'This package ground profile targets little-endian NOS3')
        cosmos, end = decode(packet, cosmos_items())
        xtce, end_xtce = decode(packet, xtce_items(), 16)
        self.assertEqual((len(packet), end, end_xtce), (64, 64, 64))
        expected = {'CMD_ERR_COUNT': 11, 'CMD_COUNT': 22, 'DEVICE_ERR_COUNT': 33,
                    'DEVICE_COUNT': 44, 'RAIL_VALID_MASK': 5, 'I2C_READY_MASK': 7}
        for rail, vals in [('12V', (12000, -25000, -250, 3000, 0x399f, 4096)),
                           ('5V', (5000, 50000, 500, 2500, 0x399f, 4096)),
                           ('3V3', (3300, 75000, 750, 2476, 0x399f, 4096))]:
            expected.update({f'RAIL_{rail}_{field}': value for field, value in zip(
                ('BUS_MV', 'SHUNT_UV', 'CURRENT_MA', 'POWER_MW', 'CONFIGURATION', 'CALIBRATION'), vals)})
        self.assertEqual(xtce, expected)
        self.assertEqual({k: cosmos[k] for k in expected}, expected)
        self.assertEqual(cosmos['CCSDS_STREAMID'], 0x09E0)
        self.assertEqual(cosmos['CCSDS_LENGTH'], len(packet) - 7)
        self.assertEqual(cosmos['CCSDS_SEQUENCE'], 0xC123)

    def test_nos3_schema_and_header(self):
        if not NOS3_ROOT: self.skipTest("--nos3-root not supplied")
        # The full XTCE schema imports xml.xsd solely for xml:base. Resolve that
        # standard anyURI attribute locally so validation needs no network.
        from lxml import etree
        class XmlNamespaceResolver(etree.Resolver):
            def resolve(self, url, pubid, context):
                if url == 'http://www.w3.org/2001/03/xml.xsd':
                    return self.resolve_string(
                        '<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" '
                        'targetNamespace="http://www.w3.org/XML/1998/namespace">'
                        '<xs:attribute name="base" type="xs:anyURI"/></xs:schema>', context)
        parser = etree.XMLParser(no_network=True)
        parser.resolvers.add(XmlNamespaceResolver())
        schema = etree.XMLSchema(etree.parse(str(NOS3_ROOT / 'gsw/yamcs/yamcs-xtce/SpaceSystemV1.2.xsd'), parser))
        schema.assertValid(etree.parse(str(ROOT / 'gsw/sentinelbio_flatsat_eps.xtce')))
        common = ET.parse(NOS3_ROOT / 'gsw/yamcs/nos3/src/main/yamcs/mdb/ccsds.xtce').getroot()
        # NOS3's common file has the XTCE 1.0 namespace, although component files
        # use 1.2. Yamcs accepts both. Resolve by local tag name for this check.
        local = lambda e: e.tag.split('}')[-1]
        types = {e.attrib['name']: e for e in common.iter() if local(e) == 'IntegerParameterType'}
        params = {e.attrib['name']: e.attrib['parameterTypeRef'] for e in common.iter() if local(e) == 'Parameter'}
        tm = next(e for e in common.iter() if local(e) == 'SequenceContainer' and e.attrib['name'] == 'CCSDS_TM')
        entries = [e.attrib['parameterRef'] for e in tm.iter() if local(e) == 'ParameterRefEntry']
        lengths = [int(next(e for e in types[params[name]] if local(e) == 'IntegerDataEncoding').attrib['sizeInBits']) for name in entries]
        self.assertEqual(entries, [f[0] for f in cosmos_items()[:6]])
        self.assertEqual(lengths, [16,16,16,32,16,32])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--nos3-root', type=Path)
    args = parser.parse_args()
    if args.nos3_root:
        NOS3_ROOT = args.nos3_root.resolve()
        # skipUnless is evaluated at class definition time.
        GroundTests.test_nos3_schema_and_header.__unittest_skip__ = False
    unittest.main(argv=[sys.argv[0]], verbosity=2)
