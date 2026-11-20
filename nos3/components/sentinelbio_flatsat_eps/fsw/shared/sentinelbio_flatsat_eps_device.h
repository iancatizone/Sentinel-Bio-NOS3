#ifndef _SENTINELBIO_FLATSAT_EPS_DEVICE_H_
#define _SENTINELBIO_FLATSAT_EPS_DEVICE_H_

#include <stdbool.h>
#include "hwlib.h"
#include "sentinelbio_flatsat_eps_platform_cfg.h"

typedef struct
{
    uint16_t BusVoltageMv;
    int32_t  ShuntVoltageUv;
    int16_t  CurrentMa;
    uint16_t PowerMw;
    uint16_t Configuration;
    uint16_t Calibration;
} __attribute__((packed)) SENTINELBIO_FLATSAT_EPS_Rail_tlm_t;

typedef struct
{
    SENTINELBIO_FLATSAT_EPS_Rail_tlm_t Rail12V;
    SENTINELBIO_FLATSAT_EPS_Rail_tlm_t Rail5V;
    SENTINELBIO_FLATSAT_EPS_Rail_tlm_t Rail3V3;
} __attribute__((packed)) SENTINELBIO_FLATSAT_EPS_Device_HK_tlm_t;

#define SENTINELBIO_FLATSAT_EPS_DEVICE_HK_LEN sizeof(SENTINELBIO_FLATSAT_EPS_Device_HK_tlm_t)

int32_t SENTINELBIO_FLATSAT_EPS_INA219_Read(i2c_bus_info_t* bus, uint8_t address,
                                           SENTINELBIO_FLATSAT_EPS_Rail_tlm_t* data);
int32_t SENTINELBIO_FLATSAT_EPS_RequestHK(i2c_bus_info_t buses[3],
                                          SENTINELBIO_FLATSAT_EPS_Device_HK_tlm_t* data);

#endif
