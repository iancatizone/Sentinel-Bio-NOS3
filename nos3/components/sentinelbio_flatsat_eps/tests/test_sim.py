"""Compile and execute actual model with real Boost headers and transport/base shims.
Requires g++ and Boost headers. --boost-include accepts an alternate include dir.
This does not test the real NOS Engine ABI, network, or plugin loader.
"""
from pathlib import Path
import argparse, subprocess, tempfile
p=argparse.ArgumentParser();p.add_argument('--boost-include',default='/usr/include');a=p.parse_args()
r=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as d:
 t=Path(d);(t/'I2C/Client').mkdir(parents=True)
 (t/'I2C/Client/I2CSlave.hpp').write_text('''#pragma once
#include <string>
#include <cstdint>
#include <cstddef>
namespace NosEngine { namespace I2C { class I2CSlave { public:
I2CSlave(int,const std::string&,const std::string&){} virtual ~I2CSlave(){}
virtual std::size_t i2c_read(std::uint8_t*,std::size_t)=0;
virtual std::size_t i2c_write(const std::uint8_t*,std::size_t)=0;
};}}
''')
 (t/'sim_i_hardware_model.hpp').write_text('''#pragma once
#include <boost/property_tree/ptree.hpp>
namespace ItcLogger { struct Logger { template<class... T> void info(const char*,T...){} template<class... T> void warning(const char*,T...){} }; }
namespace Nos3 { class SimIHardwareModel { public: explicit SimIHardwareModel(const boost::property_tree::ptree&){} virtual ~SimIHardwareModel(){} }; }
#define REGISTER_HARDWARE_MODEL(T,K) static_assert(true,"test registration shim")
''')
 (t/'main.cpp').write_text('''#include <cassert>
#include <boost/property_tree/xml_parser.hpp>
#include "sentinelbio_flatsat_eps_hardware_model.hpp"
namespace Nos3 { ItcLogger::Logger logger; ItcLogger::Logger* sim_logger=&logger; }
int main(int argc,char** argv){(void)argc;boost::property_tree::ptree cfg;read_xml(argv[1],cfg);
Nos3::Sentinelbio_flatsat_epsHardwareModel m(cfg);unsigned char b[2];
auto read=[&](int rail,int reg){assert(m.read_register(rail,reg,b,2)==2);return (b[0]<<8)|b[1];};
assert(read(0,5)==0 && read(0,4)==0);
unsigned char cal[3]={5,0x10,0}; for(int i=0;i<3;++i)assert(m.write_register(i,cal,3)==3);
assert(read(0,4)==2500 && read(0,3)==1500 && read(0,2)==24002);
assert(read(1,4)==5000 && read(1,3)==1250);
assert(read(2,4)==7500 && read(2,3)==1237);
unsigned char reset[3]={0,0x80,0}; assert(m.write_register(0,reset,3)==3);assert(read(0,5)==0 && read(0,0)==0x399f);
assert(m.read_register(0,6,b,2)==0);assert(m.write_register(0,cal,2)==0);
Nos3::SentinelbioFlatsatEpsI2CSlave slave(&m,0,0x40,"test","i2c_1");
assert(slave.i2c_write(cal,2)==0);assert(slave.i2c_write(cal,3)==3);assert(slave.i2c_read(b,2)==2);assert(b[0]==0x10 && b[1]==0);
}
''')
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-pedantic','-pthread','-I'+str(t),'-I'+a.boost_include,'-I'+str(r/'sim/inc'),str(r/'sim/src/sentinelbio_flatsat_eps_hardware_model.cpp'),str(t/'main.cpp'),'-o',str(t/'test')],check=True)
 subprocess.run([str(t/'test'),str(r/'sim/cfg/nos3-eps-simulator.xml')],check=True)
print('PASS: actual C++ model defaults, calibration, all rails, reset, malformed transactions')
