/************************************************************************
** File:
**   $Id: sentinelbio_flatsat_eps_platform_cfg.h  $
**
** Purpose:
**  Define sentinelbio_flatsat_eps Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _SENTINELBIO_FLATSAT_EPS_PLATFORM_CFG_H_
#define _SENTINELBIO_FLATSAT_EPS_PLATFORM_CFG_H_

/*
** Default SENTINELBIO_FLATSAT_EPS Configuration
*/
#ifndef SENTINELBIO_FLATSAT_EPS_CFG
#define SENTINELBIO_FLATSAT_EPS_CFG
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_12V_HANDLE  1
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_5V_HANDLE   2
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_3V3_HANDLE  3
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_SPEED       100
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_12V_ADDRESS 0x40
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_5V_ADDRESS  0x41
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_3V3_ADDRESS 0x44
#define SENTINELBIO_FLATSAT_EPS_CFG_I2C_TIMEOUT     10
#define SENTINELBIO_FLATSAT_EPS_CFG_INA219_CONFIGURATION 0x399F
#define SENTINELBIO_FLATSAT_EPS_CFG_INA219_CALIBRATION   4096
/* Calibration 4096 with a 0.1 ohm shunt gives 100 uA/current bit. */
#define SENTINELBIO_FLATSAT_EPS_CFG_CURRENT_LSB_UA  100
#define SENTINELBIO_FLATSAT_EPS_CFG_POWER_LSB_UW    2000
#define SENTINELBIO_FLATSAT_EPS_CFG_IDLE_HK_MS      1000
/* Note: Debug flag disabled (commented out) by default */
//#define SENTINELBIO_FLATSAT_EPS_CFG_DEBUG
#endif

#endif /* _SENTINELBIO_FLATSAT_EPS_PLATFORM_CFG_H_ */
