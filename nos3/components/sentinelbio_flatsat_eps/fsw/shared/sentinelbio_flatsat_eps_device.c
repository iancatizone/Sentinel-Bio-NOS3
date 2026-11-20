#include <stdint.h>
#include <string.h>

#include "sentinelbio_flatsat_eps_device.h"

#define INA219_REG_CONFIGURATION 0x00U
#define INA219_REG_SHUNT_VOLTAGE 0x01U
#define INA219_REG_BUS_VOLTAGE   0x02U
#define INA219_REG_POWER         0x03U
#define INA219_REG_CURRENT       0x04U
#define INA219_REG_CALIBRATION   0x05U
#define INA219_BUS_CNVR          0x0002U
#define INA219_BUS_OVF           0x0001U

static int32_t SENTINELBIO_FLATSAT_EPS_ReadRegister(i2c_bus_info_t *bus, uint8_t address,
                                                   uint8_t reg, uint16_t *value)
{
    uint8_t response[2] = {0, 0};
    int32_t status = i2c_master_transaction(bus, address, &reg, 1, response, 2,
                                            SENTINELBIO_FLATSAT_EPS_CFG_I2C_TIMEOUT);
    if (status == OS_SUCCESS)
        *value = (uint16_t)(((uint16_t)response[0] << 8) | response[1]);
    return status;
}

static int32_t SENTINELBIO_FLATSAT_EPS_WriteRegister(i2c_bus_info_t *bus, uint8_t address,
                                                    uint8_t reg, uint16_t value)
{
    uint8_t command[3] = {reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFFU)};
    return i2c_master_transaction(bus, address, command, sizeof(command), NULL, 0,
                                   SENTINELBIO_FLATSAT_EPS_CFG_I2C_TIMEOUT);
}

static int32_t SENTINELBIO_FLATSAT_EPS_ReadAll(i2c_bus_info_t *bus, uint8_t address,
                                              SENTINELBIO_FLATSAT_EPS_Rail_tlm_t *data)
{
    uint16_t configuration = 0;
    uint16_t calibration = 0;
    uint16_t raw = 0;
    int32_t status;
    bool changed = false;

    status = SENTINELBIO_FLATSAT_EPS_ReadRegister(bus, address, INA219_REG_CONFIGURATION,
                                                  &configuration);
    if (status != OS_SUCCESS) return status;
    status = SENTINELBIO_FLATSAT_EPS_ReadRegister(bus, address, INA219_REG_CALIBRATION,
                                                  &calibration);
    if (status != OS_SUCCESS) return status;

    /* INA219 resets with calibration=0, making current/power unusable.  Check
     * both settings on every sample so late arrivals and sensor resets recover.
     * Do not rewrite unchanged configuration: that restarts ADC conversion. */
    if (configuration != SENTINELBIO_FLATSAT_EPS_CFG_INA219_CONFIGURATION)
    {
        status = SENTINELBIO_FLATSAT_EPS_WriteRegister(bus, address, INA219_REG_CONFIGURATION,
                                   SENTINELBIO_FLATSAT_EPS_CFG_INA219_CONFIGURATION);
        if (status != OS_SUCCESS) return status;
        changed = true;
    }
    if (calibration != SENTINELBIO_FLATSAT_EPS_CFG_INA219_CALIBRATION)
    {
        status = SENTINELBIO_FLATSAT_EPS_WriteRegister(bus, address, INA219_REG_CALIBRATION,
                                   SENTINELBIO_FLATSAT_EPS_CFG_INA219_CALIBRATION);
        if (status != OS_SUCCESS) return status;
        changed = true;
    }
    /* Wait until a later HK attempt for a conversion made with these settings.
     * This path deliberately produces invalid/zero data, not stale readings. */
    if (changed) return OS_ERROR;
    data->Configuration = configuration;
    data->Calibration = calibration;

    status = SENTINELBIO_FLATSAT_EPS_ReadRegister(bus, address, INA219_REG_SHUNT_VOLTAGE, &raw);
    if (status != OS_SUCCESS) return status;
    /* INA219 shunt voltage is signed two's-complement, 10 uV per bit. */
    data->ShuntVoltageUv = ((raw & 0x8000U) ? (int32_t)raw - 65536L : (int32_t)raw) * 10L;

    status = SENTINELBIO_FLATSAT_EPS_ReadRegister(bus, address, INA219_REG_BUS_VOLTAGE, &raw);
    if (status != OS_SUCCESS) return status;
    if ((raw & INA219_BUS_OVF) != 0U || (raw & INA219_BUS_CNVR) == 0U) return OS_ERROR;
    data->BusVoltageMv = (uint16_t)((raw >> 3) * 4U);

    status = SENTINELBIO_FLATSAT_EPS_ReadRegister(bus, address, INA219_REG_POWER, &raw);
    if (status != OS_SUCCESS) return status;
    {
        uint32_t mw = ((uint32_t)raw * SENTINELBIO_FLATSAT_EPS_CFG_POWER_LSB_UW) / 1000U;
        /* Overflow must invalidate the sample rather than silently saturate it. */
        if (mw > UINT16_MAX) return OS_ERROR;
        data->PowerMw = (uint16_t)mw;
    }

    status = SENTINELBIO_FLATSAT_EPS_ReadRegister(bus, address, INA219_REG_CURRENT, &raw);
    if (status != OS_SUCCESS) return status;
    {
        const uint32_t raw_code = (uint32_t)raw;
        const uint32_t negative = raw_code & 0x8000U;
        const uint32_t magnitude = negative ? ((~raw_code + 1U) & 0xFFFFU) : raw_code;
        const uint32_t current_ma =
            (magnitude * (uint32_t)SENTINELBIO_FLATSAT_EPS_CFG_CURRENT_LSB_UA) / 1000U;
        if (current_ma > (negative ? 32768U : 32767U)) return OS_ERROR;
        data->CurrentMa = negative ? (int16_t)(-(int32_t)current_ma) : (int16_t)current_ma;
    }
    return OS_SUCCESS;
}

int32_t SENTINELBIO_FLATSAT_EPS_INA219_Read(i2c_bus_info_t *bus, uint8_t address,
                                          SENTINELBIO_FLATSAT_EPS_Rail_tlm_t *data)
{
    SENTINELBIO_FLATSAT_EPS_Rail_tlm_t sample;
    int32_t status;
    if (data == NULL) return OS_ERROR;
    memset(data, 0, sizeof(*data));
    if (bus == NULL || bus->isOpen != I2C_OPEN) return OS_ERROR;
    memset(&sample, 0, sizeof(sample));
    status = SENTINELBIO_FLATSAT_EPS_ReadAll(bus, address, &sample);
    if (status == OS_SUCCESS) *data = sample;
    return status;
}

int32_t SENTINELBIO_FLATSAT_EPS_RequestHK(i2c_bus_info_t buses[3],
                                         SENTINELBIO_FLATSAT_EPS_Device_HK_tlm_t *data)
{
    int32_t status = OS_SUCCESS;
    int32_t rail_status;
    if (data == NULL) return OS_ERROR;
    memset(data, 0, sizeof(*data));
    if (buses == NULL) return OS_ERROR;
    rail_status = SENTINELBIO_FLATSAT_EPS_INA219_Read(
        &buses[0], SENTINELBIO_FLATSAT_EPS_CFG_I2C_12V_ADDRESS, &data->Rail12V);
    if (rail_status != OS_SUCCESS) status = rail_status;
    rail_status = SENTINELBIO_FLATSAT_EPS_INA219_Read(
        &buses[1], SENTINELBIO_FLATSAT_EPS_CFG_I2C_5V_ADDRESS, &data->Rail5V);
    if (rail_status != OS_SUCCESS && status == OS_SUCCESS) status = rail_status;
    rail_status = SENTINELBIO_FLATSAT_EPS_INA219_Read(
        &buses[2], SENTINELBIO_FLATSAT_EPS_CFG_I2C_3V3_ADDRESS, &data->Rail3V3);
    if (rail_status != OS_SUCCESS && status == OS_SUCCESS) status = rail_status;
    return status;
}
