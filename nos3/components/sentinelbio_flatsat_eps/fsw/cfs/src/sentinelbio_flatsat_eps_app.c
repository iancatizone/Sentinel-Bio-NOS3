#include <string.h>
#include "sentinelbio_flatsat_eps_app.h"

SENTINELBIO_FLATSAT_EPS_AppData_t SENTINELBIO_FLATSAT_EPS_AppData;

/* Connecting to a simulator is operational, never an application-start gate. */
static void SENTINELBIO_FLATSAT_EPS_Connect(void)
{
    uint8 i;
    for (i = 0; i < 3; ++i)
    {
        i2c_bus_info_t *bus = &SENTINELBIO_FLATSAT_EPS_AppData.Sentinelbio_flatsat_epsI2C[i];
        if (bus->isOpen != I2C_OPEN)
        {
            int32 status = i2c_master_init(bus);
            if (status == OS_SUCCESS)
            {
                /* HWLIB can return success for a master already opened by another app. */
                bus->isOpen = I2C_OPEN;
                SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.I2CReadyMask |= (uint8)(1U << i);
                CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_I2C_READY_EID, CFE_EVS_EventType_INFORMATION,
                                 "SB_EPS: I2C master ready: rail=%u handle=%d address=0x%02X",
                                 (unsigned int)i, (int)bus->handle, (unsigned int)bus->addr);
            }
            else if (!SENTINELBIO_FLATSAT_EPS_AppData.InitFailureReported[i])
            {
                SENTINELBIO_FLATSAT_EPS_AppData.InitFailureReported[i] = true;
                CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                                 "SB_EPS: I2C unavailable: rail=%u handle=%d RC=0x%08X; retrying",
                                 (unsigned int)i, (int)bus->handle, (unsigned int)status);
            }
        }
    }
}

void EPS_AppMain(void)
{
    int32 status = SENTINELBIO_FLATSAT_EPS_AppInit();
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SB_EPS: application initialization failed RC=0x%08X\n", (unsigned int)status);
        SENTINELBIO_FLATSAT_EPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }
    while (CFE_ES_RunLoop(&SENTINELBIO_FLATSAT_EPS_AppData.RunStatus))
    {
        CFE_SB_Buffer_t *buffer = NULL;
        status = CFE_SB_ReceiveBuffer(&buffer, SENTINELBIO_FLATSAT_EPS_AppData.CmdPipe,
                                     SENTINELBIO_FLATSAT_EPS_CFG_IDLE_HK_MS);
        if (status == CFE_SUCCESS)
        {
            SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr = &buffer->Msg;
            SENTINELBIO_FLATSAT_EPS_ProcessCommandPacket();
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            SENTINELBIO_FLATSAT_EPS_ReportHousekeeping();
        }
        else
        {
            CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                             "SB_EPS: receive failed RC=0x%08X", (unsigned int)status);
            SENTINELBIO_FLATSAT_EPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }
    CFE_ES_ExitApp(SENTINELBIO_FLATSAT_EPS_AppData.RunStatus);
}

int32 SENTINELBIO_FLATSAT_EPS_AppInit(void)
{
    int32 status;
    uint8 i;
    const int handles[3] = {SENTINELBIO_FLATSAT_EPS_CFG_I2C_12V_HANDLE,
                            SENTINELBIO_FLATSAT_EPS_CFG_I2C_5V_HANDLE,
                            SENTINELBIO_FLATSAT_EPS_CFG_I2C_3V3_HANDLE};
    const uint8 addresses[3] = {SENTINELBIO_FLATSAT_EPS_CFG_I2C_12V_ADDRESS,
                                SENTINELBIO_FLATSAT_EPS_CFG_I2C_5V_ADDRESS,
                                SENTINELBIO_FLATSAT_EPS_CFG_I2C_3V3_ADDRESS};
    memset(&SENTINELBIO_FLATSAT_EPS_AppData, 0, sizeof(SENTINELBIO_FLATSAT_EPS_AppData));
    SENTINELBIO_FLATSAT_EPS_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_ES_WriteToSysLog("SB_EPS: EPS_AppMain entered; package 2.0.0\n");
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS) return status;
    status = CFE_SB_CreatePipe(&SENTINELBIO_FLATSAT_EPS_AppData.CmdPipe,
                               SENTINELBIO_FLATSAT_EPS_PIPE_DEPTH, "SB_EPS_CMD_PIPE");
    if (status != CFE_SUCCESS) return status;
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SENTINELBIO_FLATSAT_EPS_CMD_MID),
                              SENTINELBIO_FLATSAT_EPS_AppData.CmdPipe);
    if (status != CFE_SUCCESS) return status;
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SENTINELBIO_FLATSAT_EPS_REQ_HK_MID),
                              SENTINELBIO_FLATSAT_EPS_AppData.CmdPipe);
    if (status != CFE_SUCCESS) return status;
    status = CFE_MSG_Init(CFE_MSG_PTR(SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.TlmHeader),
                          CFE_SB_ValueToMsgId(SENTINELBIO_FLATSAT_EPS_HK_TLM_MID),
                          sizeof(SENTINELBIO_FLATSAT_EPS_Hk_tlm_t));
    if (status != CFE_SUCCESS) return status;
    for (i = 0; i < 3; ++i)
    {
        i2c_bus_info_t *bus = &SENTINELBIO_FLATSAT_EPS_AppData.Sentinelbio_flatsat_epsI2C[i];
        bus->handle = handles[i];
        bus->addr = addresses[i];
        bus->speed = SENTINELBIO_FLATSAT_EPS_CFG_I2C_SPEED;
        bus->isOpen = I2C_CLOSED;
    }
    CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                     "SENTINELBIO_FLATSAT_EPS App Initialized. Version 2.0.0; I2C connects on HK");
    CFE_ES_WriteToSysLog("SB_EPS: initialized; CMD=0x19E0 REQ_HK=0x19E1 TLM=0x09E0\n");
    return CFE_SUCCESS;
}

int32 SENTINELBIO_FLATSAT_EPS_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length)
{
    CFE_MSG_Size_t size = 0;
    if (CFE_MSG_GetSize(msg, &size) != CFE_SUCCESS || size != expected_length)
    {
        SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandErrorCount++;
        CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                         "SB_EPS: bad command length actual=%lu expected=%u",
                         (unsigned long)size, (unsigned int)expected_length);
        return OS_ERROR;
    }
    return OS_SUCCESS;
}

void SENTINELBIO_FLATSAT_EPS_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t mid = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr, &mid);
    switch (CFE_SB_MsgIdToValue(mid))
    {
        case SENTINELBIO_FLATSAT_EPS_CMD_MID:
            SENTINELBIO_FLATSAT_EPS_ProcessGroundCommand();
            break;
        case SENTINELBIO_FLATSAT_EPS_REQ_HK_MID:
            SENTINELBIO_FLATSAT_EPS_ProcessTelemetryRequest();
            break;
        default:
            SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_PROCESS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                             "SB_EPS: unexpected MID=0x%04X", (unsigned int)CFE_SB_MsgIdToValue(mid));
            break;
    }
}

void SENTINELBIO_FLATSAT_EPS_ProcessGroundCommand(void)
{
    CFE_MSG_FcnCode_t code = 0;
    CFE_MSG_GetFcnCode(SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr, &code);
    if (code == SENTINELBIO_FLATSAT_EPS_NOOP_CC || code == SENTINELBIO_FLATSAT_EPS_RESET_COUNTERS_CC)
    {
        if (SENTINELBIO_FLATSAT_EPS_VerifyCmdLength(SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr,
                                                  sizeof(SENTINELBIO_FLATSAT_EPS_NoArgs_cmd_t)) != OS_SUCCESS) return;
        if (code == SENTINELBIO_FLATSAT_EPS_RESET_COUNTERS_CC)
        {
            SENTINELBIO_FLATSAT_EPS_ResetCounters();
            CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_CMD_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                             "SENTINELBIO_FLATSAT_EPS: RESET counters command received");
        }
        else
        {
            SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandCount++;
            CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_CMD_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                             "SENTINELBIO_FLATSAT_EPS: NOOP command received");
        }
    }
    else
    {
        if (code == SENTINELBIO_FLATSAT_EPS_SWITCH_CC &&
            SENTINELBIO_FLATSAT_EPS_VerifyCmdLength(SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr,
                                                  sizeof(SENTINELBIO_FLATSAT_EPS_Switch_cmd_t)) != OS_SUCCESS) return;
        SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandErrorCount++;
        CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                         "SB_EPS: unsupported command code=%u (fixed rails; no switch hardware)", (unsigned int)code);
    }
}

void SENTINELBIO_FLATSAT_EPS_ProcessTelemetryRequest(void)
{
    CFE_MSG_FcnCode_t code = 0;
    if (SENTINELBIO_FLATSAT_EPS_VerifyCmdLength(SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr,
                                              sizeof(SENTINELBIO_FLATSAT_EPS_NoArgs_cmd_t)) != OS_SUCCESS) return;
    CFE_MSG_GetFcnCode(SENTINELBIO_FLATSAT_EPS_AppData.MsgPtr, &code);
    if (code == SENTINELBIO_FLATSAT_EPS_REQ_HK_TLM)
        SENTINELBIO_FLATSAT_EPS_ReportHousekeeping();
    else
        SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandErrorCount++;
}

void SENTINELBIO_FLATSAT_EPS_ReportHousekeeping(void)
{
    uint8 i;
    SENTINELBIO_FLATSAT_EPS_Device_HK_tlm_t data;
    SENTINELBIO_FLATSAT_EPS_Rail_tlm_t *rails[3] = {&data.Rail12V, &data.Rail5V, &data.Rail3V3};
    uint8 previous = SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.RailValidMask;
    memset(&data, 0, sizeof(data));
    SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.RailValidMask = 0;
    SENTINELBIO_FLATSAT_EPS_Connect();
    for (i = 0; i < 3; ++i)
    {
        i2c_bus_info_t *bus = &SENTINELBIO_FLATSAT_EPS_AppData.Sentinelbio_flatsat_epsI2C[i];
        if (bus->isOpen == I2C_OPEN &&
            SENTINELBIO_FLATSAT_EPS_INA219_Read(bus, bus->addr, rails[i]) == OS_SUCCESS)
            SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.RailValidMask |= (uint8)(1U << i);
    }
    SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.DeviceHK = data;
    if (SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.RailValidMask == 7U)
        SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.DeviceCount++;
    else
        SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.DeviceErrorCount++;
    if (!SENTINELBIO_FLATSAT_EPS_AppData.HkAttempted ||
        previous != SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.RailValidMask)
    {
        CFE_EVS_SendEvent(SENTINELBIO_FLATSAT_EPS_HEALTH_EID, CFE_EVS_EventType_INFORMATION,
                         "SB_EPS: rail-valid mask=0x%02X (bit0=12V bit1=5V bit2=3V3)",
                         (unsigned int)SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.RailValidMask);
        SENTINELBIO_FLATSAT_EPS_AppData.HkAttempted = true;
    }
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt);
    if (CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt, true) != CFE_SUCCESS)
        CFE_ES_WriteToSysLog("SB_EPS: housekeeping transmit failed\n");
}

void SENTINELBIO_FLATSAT_EPS_ResetCounters(void)
{
    SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandCount = 0;
    SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.DeviceCount = 0;
    SENTINELBIO_FLATSAT_EPS_AppData.HkTelemetryPkt.DeviceErrorCount = 0;
}
