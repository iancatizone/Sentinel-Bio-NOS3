/************************************************************************
** File:
**   $Id: sentinelbio_flatsat_eps_msgids.h  $
**
** Purpose:
**  Define SENTINELBIO_FLATSAT_EPS Message IDs
**
*************************************************************************/
#ifndef _SENTINELBIO_FLATSAT_EPS_MSGIDS_H_
#define _SENTINELBIO_FLATSAT_EPS_MSGIDS_H_

/*
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define SENTINELBIO_FLATSAT_EPS_CMD_MID 0x19E0

/*
** This MID is for commands telling the app to publish its telemetry message
*/
#define SENTINELBIO_FLATSAT_EPS_REQ_HK_MID 0x19E1

/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define SENTINELBIO_FLATSAT_EPS_HK_TLM_MID 0x09E0

#endif /* _SENTINELBIO_FLATSAT_EPS_MSGIDS_H_ */
