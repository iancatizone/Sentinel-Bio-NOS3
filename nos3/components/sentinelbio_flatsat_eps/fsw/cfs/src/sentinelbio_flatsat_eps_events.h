/************************************************************************
** File:
**    sentinelbio_flatsat_eps_events.h
**
** Purpose:
**  Define SENTINELBIO_FLATSAT_EPS application event IDs
**
*************************************************************************/

#ifndef _SENTINELBIO_FLATSAT_EPS_EVENTS_H_
#define _SENTINELBIO_FLATSAT_EPS_EVENTS_H_

/* Standard app event IDs */
#define SENTINELBIO_FLATSAT_EPS_RESERVED_EID        0
#define SENTINELBIO_FLATSAT_EPS_STARTUP_INF_EID     1
#define SENTINELBIO_FLATSAT_EPS_LEN_ERR_EID         2
#define SENTINELBIO_FLATSAT_EPS_PIPE_ERR_EID        3
#define SENTINELBIO_FLATSAT_EPS_SUB_CMD_ERR_EID     4
#define SENTINELBIO_FLATSAT_EPS_SUB_REQ_HK_ERR_EID  5
#define SENTINELBIO_FLATSAT_EPS_PROCESS_CMD_ERR_EID 6

/* Standard command event IDs */
#define SENTINELBIO_FLATSAT_EPS_CMD_ERR_EID       10
#define SENTINELBIO_FLATSAT_EPS_CMD_NOOP_INF_EID  11
#define SENTINELBIO_FLATSAT_EPS_CMD_RESET_INF_EID 12

/* Device specific command event IDs */
#define SENTINELBIO_FLATSAT_EPS_CMD_SWITCH_INF_EID 20
#define SENTINELBIO_FLATSAT_EPS_SWITCH_INF_EID     21
#define SENTINELBIO_FLATSAT_EPS_SWITCH_ERR_EID     22

/* Standard telemetry event IDs */
#define SENTINELBIO_FLATSAT_EPS_DEVICE_TLM_ERR_EID 30
#define SENTINELBIO_FLATSAT_EPS_REQ_HK_ERR_EID     31

/* Hardware protocol event IDs */
#define SENTINELBIO_FLATSAT_EPS_I2C_INIT_ERR_EID 40
#define SENTINELBIO_FLATSAT_EPS_I2C_READY_EID    41
#define SENTINELBIO_FLATSAT_EPS_HEALTH_EID       42

#endif /* _SENTINELBIO_FLATSAT_EPS_EVENTS_H_ */
