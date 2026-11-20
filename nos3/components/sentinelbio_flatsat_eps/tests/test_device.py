"""Execute the real C driver with an injected I2C transport; no NOS Engine required."""
from pathlib import Path
import tempfile, subprocess
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory)
 (p/'hwlib.h').write_text('''#include <stdint.h>
#include <stdbool.h>
#define OS_SUCCESS 0
#define OS_ERROR -1
#define I2C_OPEN 0
#define I2C_CLOSED 1
typedef struct { int32_t handle,addr; uint8_t isOpen; uint32_t speed; } i2c_bus_info_t;
int32_t i2c_master_transaction(i2c_bus_info_t*,uint8_t,void*,uint8_t,void*,uint8_t,uint16_t);
''')
 (p/'main.c').write_text('''#include <assert.h>
#include <string.h>
#include "sentinelbio_flatsat_eps_device.h"
static uint16_t registers[6]={0x399f,2500,(3000<<3)|2,1500,2500,0};
static int fail_reg=-1;
int32_t i2c_master_transaction(i2c_bus_info_t*b,uint8_t a,void*t,uint8_t n,void*r,uint8_t m,uint16_t timeout){
 (void)b;(void)a;(void)timeout; uint8_t *tx=t,*rx=r;unsigned reg=tx[0];
 if((int)reg==fail_reg)return -1;
 if(n==3 && m==0){registers[reg]=(tx[1]<<8)|tx[2];return 0;}
 assert(n==1 && m==2 && reg<6);rx[0]=registers[reg]>>8;rx[1]=registers[reg];return 0;
}
int main(void){
 i2c_bus_info_t b={1,0x40,I2C_OPEN,100}; SENTINELBIO_FLATSAT_EPS_Rail_tlm_t d,zero;
 memset(&zero,0,sizeof zero);
 assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)!=0);assert(registers[5]==4096);assert(!memcmp(&d,&zero,sizeof d));
 assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)==0);assert(d.BusVoltageMv==12000 && d.CurrentMa==250 && d.PowerMw==3000 && d.ShuntVoltageUv==25000);
 fail_reg=4;assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)!=0);assert(!memcmp(&d,&zero,sizeof d));fail_reg=-1;
 registers[4]=(uint16_t)-2500;registers[1]=(uint16_t)-2500;
 assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)==0);assert(d.CurrentMa==-250 && d.ShuntVoltageUv==-25000);
 registers[2]|=1;assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)!=0);registers[2]&=~1;
 registers[5]=0;assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)!=0);assert(registers[5]==4096);
 b.isOpen=I2C_CLOSED;assert(SENTINELBIO_FLATSAT_EPS_INA219_Read(&b,0x40,&d)!=0);assert(!memcmp(&d,&zero,sizeof d));
 return 0;
}
''')
 subprocess.run(['gcc','-std=c99','-Wall','-Wextra','-Werror','-I'+str(p),'-I'+str(root/'fsw/shared'),'-I'+str(root/'fsw/cfs/platform_inc'),str(root/'fsw/shared/sentinelbio_flatsat_eps_device.c'),str(p/'main.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: calibration/reset recovery, signed units, read failure clearing, overflow, closed bus')
