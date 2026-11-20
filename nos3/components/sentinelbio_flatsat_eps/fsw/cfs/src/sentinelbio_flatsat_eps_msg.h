/*******************************************************************************
** File:
**   sentinelbio_flatsat_eps_msg.h
**
** Purpose:
**  Define SENTINELBIO_FLATSAT_EPS application commands and telemetry messages
**
*******************************************************************************/
#ifndef _SENTINELBIO_FLATSAT_EPS_MSG_H_
#define _SENTINELBIO_FLATSAT_EPS_MSG_H_

#include "cfe.h"
#include "sentinelbio_flatsat_eps_device.h"

/*
** Ground Command Codes
*/
#define SENTINELBIO_FLATSAT_EPS_NOOP_CC           0
#define SENTINELBIO_FLATSAT_EPS_RESET_COUNTERS_CC 1
#define SENTINELBIO_FLATSAT_EPS_SWITCH_CC         2

/*
** Telemetry Request Command Codes
*/
#define SENTINELBIO_FLATSAT_EPS_REQ_HK_TLM 0

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} SENTINELBIO_FLATSAT_EPS_NoArgs_cmd_t;

/*
** Compatibility command retained for standard NOS3 component tooling.  The
** physical FlatSat described by this component has no controllable switch;
** the cFS app accepts the packet format and reports it as unsupported.
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint8                   SwitchNumber;
    uint8                   State;

} SENTINELBIO_FLATSAT_EPS_Switch_cmd_t;

/*
** SENTINELBIO_FLATSAT_EPS housekeeping type definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t   TlmHeader;
    uint8                       CommandErrorCount;
    uint8                       CommandCount;
    uint8                       DeviceErrorCount;
    uint8                       DeviceCount;
    SENTINELBIO_FLATSAT_EPS_Device_HK_tlm_t DeviceHK;
    uint8 RailValidMask; /* bit 0=12V, bit 1=5V, bit 2=3V3; full read succeeded this HK */
    uint8 I2CReadyMask;  /* opened masters, does not imply a slave responded */

} __attribute__((packed)) SENTINELBIO_FLATSAT_EPS_Hk_tlm_t;
#define SENTINELBIO_FLATSAT_EPS_HK_TLM_LNGTH sizeof(SENTINELBIO_FLATSAT_EPS_Hk_tlm_t)

#endif /* _SENTINELBIO_FLATSAT_EPS_MSG_H_ */
