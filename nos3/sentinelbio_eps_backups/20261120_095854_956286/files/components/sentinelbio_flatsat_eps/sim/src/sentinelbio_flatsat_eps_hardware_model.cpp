#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <set>
#include <utility>

#include <boost/foreach.hpp>

#include <sentinelbio_flatsat_eps_hardware_model.hpp>

namespace Nos3
{
    namespace
    {
        const std::uint16_t INA219_DEFAULT_CONFIGURATION = 0x399F;
        const std::uint16_t INA219_DEFAULT_CALIBRATION = 0;
        const std::size_t RAIL_COUNT = 3;
        const char* const RAIL_NAMES[RAIL_COUNT] = {"12v", "5v", "3v3"};
        const char* const DEFAULT_BUSES[RAIL_COUNT] = {"i2c_1", "i2c_2", "i2c_3"};
        const int DEFAULT_ADDRESSES[RAIL_COUNT] = {0x40, 0x41, 0x44};
        const double DEFAULT_VOLTAGES[RAIL_COUNT] = {12.0, 5.0, 3.3};
    }

    REGISTER_HARDWARE_MODEL(Sentinelbio_flatsat_epsHardwareModel, "SENTINELBIO_FLATSAT_EPS");

    extern ItcLogger::Logger* sim_logger;

    SentinelbioFlatsatEpsI2CSlave::SentinelbioFlatsatEpsI2CSlave(
        Sentinelbio_flatsat_epsHardwareModel* model, std::size_t rail, int address,
        const std::string& connection, const std::string& bus)
        : NosEngine::I2C::I2CSlave(address, connection, bus),
          _model(model),
          _rail(rail),
          _register(0)
    {
    }

    std::size_t SentinelbioFlatsatEpsI2CSlave::i2c_write(const std::uint8_t* buffer,
                                                        std::size_t length)
    {
        /* A valid transaction is a one-byte register pointer or a complete
         * pointer + 16-bit register write. Never acknowledge truncated writes. */
        if (buffer == nullptr || (length != 1 && length != 3) || buffer[0] > 0x05)
        {
            return 0;
        }
        if (length == 3 && _model->write_register(_rail, buffer, length) != length)
        {
            return 0;
        }
        _register.store(buffer[0]);
        return length;
    }

    std::size_t SentinelbioFlatsatEpsI2CSlave::i2c_read(std::uint8_t* buffer,
                                                       std::size_t length)
    {
        return _model->read_register(_rail, _register.load(), buffer, length);
    }

    Sentinelbio_flatsat_epsHardwareModel::Sentinelbio_flatsat_epsHardwareModel(
        const boost::property_tree::ptree& config)
        : SimIHardwareModel(config),
          _slaves(),
          _input_voltage(config.get("simulator.hardware-model.physical.input-voltage", 14.0)),
          _input_min_voltage(config.get("simulator.hardware-model.physical.input-min-voltage", 12.0)),
          _input_max_voltage(config.get("simulator.hardware-model.physical.input-max-voltage", 16.0))
    {
        const std::string connection = config.get("common.nos-connection-string",
                                                  "tcp://127.0.0.1:12001");

        if (!std::isfinite(_input_voltage) || !std::isfinite(_input_min_voltage) ||
            !std::isfinite(_input_max_voltage) || _input_voltage < 0.0 ||
            _input_min_voltage < 0.0 || _input_min_voltage > _input_max_voltage)
        {
            throw std::invalid_argument("Sentinel Bio EPS: invalid bench input voltage/range");
        }
        if (connection.empty())
        {
            throw std::invalid_argument("Sentinel Bio EPS: empty NOS connection string");
        }
        std::array<std::string, RAIL_COUNT> buses;
        std::array<int, RAIL_COUNT> addresses;
        std::set<std::pair<std::string, int> > endpoints;

        if (_input_voltage < _input_min_voltage || _input_voltage > _input_max_voltage)
        {
            sim_logger->warning("Sentinel Bio FlatSat EPS input voltage %.3f V is outside the configured %.3f-%.3f V range",
                                _input_voltage, _input_min_voltage, _input_max_voltage);
        }

        for (std::size_t i = 0; i < RAIL_COUNT; ++i)
        {
            const std::string prefix = std::string("simulator.hardware-model.physical.rail-") + RAIL_NAMES[i];
            _rails[i].nominal_voltage = config.get(prefix + ".nominal-voltage", DEFAULT_VOLTAGES[i]);
            _rails[i].current_amps = config.get(prefix + ".current-amps", 0.0);
            _rails[i].shunt_ohms = config.get(prefix + ".shunt-ohms", 0.1);
            _rails[i].dropout_voltage = config.get(prefix + ".dropout-voltage", 0.0);
            _rails[i].configuration = config.get(prefix + ".configuration", INA219_DEFAULT_CONFIGURATION);
            _rails[i].calibration = static_cast<std::uint16_t>(
                config.get(prefix + ".calibration", INA219_DEFAULT_CALIBRATION) & 0xFFFEU);
            const Rail& rail = _rails[i];
            if (!std::isfinite(rail.nominal_voltage) || !std::isfinite(rail.current_amps) ||
                !std::isfinite(rail.shunt_ohms) || !std::isfinite(rail.dropout_voltage) ||
                rail.nominal_voltage < 0.0 || rail.nominal_voltage > 26.0 ||
                rail.shunt_ohms <= 0.0 || rail.dropout_voltage < 0.0 ||
                std::abs(rail.current_amps * rail.shunt_ohms) > 0.320)
            {
                throw std::invalid_argument("Sentinel Bio EPS: invalid physical settings for rail " +
                                            std::string(RAIL_NAMES[i]));
            }

            std::string bus = DEFAULT_BUSES[i];
            int address = DEFAULT_ADDRESSES[i];
            if (config.get_child_optional("simulator.hardware-model.connections"))
            {
                BOOST_FOREACH(const boost::property_tree::ptree::value_type& item,
                              config.get_child("simulator.hardware-model.connections"))
                {
                    if (item.second.get("type", "") == "i2c" &&
                        item.second.get("rail", "") == RAIL_NAMES[i])
                    {
                        bus = item.second.get("bus-name", bus);
                        const std::string address_text = item.second.get<std::string>(
                            "bus-address", std::to_string(address));
                        char* end = nullptr;
                        errno = 0;
                        const long parsed = std::strtol(address_text.c_str(), &end, 0);
                        if (errno != 0 || end == address_text.c_str() || *end != '\0' ||
                            parsed < 0x40 || parsed > 0x4F)
                        {
                            throw std::invalid_argument("Sentinel Bio EPS: invalid INA219 I2C address " +
                                                        address_text);
                        }
                        address = static_cast<int>(parsed);
                    }
                }
            }

            if (bus.empty() || !endpoints.insert(std::make_pair(bus, address)).second)
            {
                throw std::invalid_argument("Sentinel Bio EPS: empty bus or duplicate I2C endpoint");
            }
            buses[i] = bus;
            addresses[i] = address;
        }

        /* Validate every rail before starting callbacks; unique_ptr cleans up
         * earlier slaves if the transport throws while connecting a later one. */
        for (std::size_t i = 0; i < RAIL_COUNT; ++i)
        {
            _slaves[i].reset(new SentinelbioFlatsatEpsI2CSlave(
                this, i, addresses[i], connection, buses[i]));
            sim_logger->info("Sentinel Bio FlatSat EPS: INA219 %s on %s at 0x%02X",
                             RAIL_NAMES[i], buses[i].c_str(), addresses[i]);
        }
    }

    Sentinelbio_flatsat_epsHardwareModel::~Sentinelbio_flatsat_epsHardwareModel()
    {
        for (auto& slave : _slaves)
        {
            slave.reset();
        }
    }

    double Sentinelbio_flatsat_epsHardwareModel::effective_voltage(const Rail& rail) const
    {
        if (_input_voltage + 1.0e-9 >= rail.nominal_voltage + rail.dropout_voltage)
        {
            return rail.nominal_voltage;
        }
        return std::max(0.0, _input_voltage - rail.dropout_voltage);
    }

    std::uint16_t Sentinelbio_flatsat_epsHardwareModel::clamp_unsigned(long value)
    {
        if (value < 0)
        {
            return 0;
        }
        if (value > 0xFFFFL)
        {
            return 0xFFFF;
        }
        return static_cast<std::uint16_t>(value);
    }

    std::int16_t Sentinelbio_flatsat_epsHardwareModel::clamp_signed(long value)
    {
        if (value < -32768L)
        {
            return static_cast<std::int16_t>(-32768);
        }
        if (value > 32767L)
        {
            return static_cast<std::int16_t>(32767);
        }
        return static_cast<std::int16_t>(value);
    }

    std::size_t Sentinelbio_flatsat_epsHardwareModel::read_register(
        std::size_t rail_index, std::uint8_t reg, std::uint8_t* buffer, std::size_t length) const
    {
        if (buffer == nullptr || rail_index >= RAIL_COUNT || reg > 0x05 ||
            (length != 1 && length != 2))
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(_mutex);
        const Rail& rail = _rails[rail_index];
        std::uint16_t value = 0;

        switch (reg)
        {
            case 0x00:
                value = rail.configuration;
                break;

            case 0x01:
            {
                const long shunt_code = std::lround(
                    (rail.current_amps * rail.shunt_ohms) / 0.00001);
                value = static_cast<std::uint16_t>(clamp_signed(shunt_code));
                break;
            }

            case 0x02:
            {
                const long bus_code = std::lround(effective_voltage(rail) / 0.004);
                const std::uint16_t clamped = static_cast<std::uint16_t>(
                    std::max(0L, std::min(8191L, bus_code)));
                const long shunt_code = std::lround(
                    (rail.current_amps * rail.shunt_ohms) / 0.00001);
                const long current_code = (shunt_code * rail.calibration) / 4096L;
                const long power_code = (current_code * bus_code) / 5000L;
                const bool overflow = current_code < -32768L || current_code > 32767L ||
                                      power_code > 65535L;
                /* Steady-state approximation: conversions complete immediately.
                 * No ADC timing or CNVR clearing on Power reads is modeled. */
                value = static_cast<std::uint16_t>((clamped << 3) | 0x0002 |
                                                   (overflow ? 0x0001 : 0));
                break;
            }

            case 0x03:
            {
                const long shunt_code = std::lround(
                    (rail.current_amps * rail.shunt_ohms) / 0.00001);
                const long current_code = (rail.calibration == 0)
                                              ? 0
                                              : (shunt_code * rail.calibration) / 4096L;
                const long bus_code = std::lround(effective_voltage(rail) / 0.004);
                value = clamp_unsigned((current_code * bus_code) / 5000L);
                break;
            }

            case 0x04:
            {
                const long shunt_code = std::lround(
                    (rail.current_amps * rail.shunt_ohms) / 0.00001);
                const long current_code = (rail.calibration == 0)
                                              ? 0
                                              : (shunt_code * rail.calibration) / 4096L;
                value = static_cast<std::uint16_t>(clamp_signed(current_code));
                break;
            }

            case 0x05:
                value = rail.calibration;
                break;

            default:
                value = 0;
                break;
        }

        if (length > 0)
        {
            buffer[0] = static_cast<std::uint8_t>(value >> 8);
        }
        if (length > 1)
        {
            buffer[1] = static_cast<std::uint8_t>(value & 0xFF);
        }
        for (std::size_t i = 2; i < length; ++i)
        {
            buffer[i] = 0;
        }
        return length;
    }

    std::size_t Sentinelbio_flatsat_epsHardwareModel::write_register(
        std::size_t rail_index, const std::uint8_t* buffer, std::size_t length)
    {
        if (buffer == nullptr || rail_index >= RAIL_COUNT || length != 3 ||
            (buffer[0] != 0x00 && buffer[0] != 0x05))
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(_mutex);
        const std::uint16_t value = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(buffer[1]) << 8) | buffer[2]);

        if (buffer[0] == 0x00)
        {
            if ((value & 0x8000U) != 0U)
            {
                _rails[rail_index].configuration = INA219_DEFAULT_CONFIGURATION;
                _rails[rail_index].calibration = INA219_DEFAULT_CALIBRATION;
            }
            else
            {
                _rails[rail_index].configuration = value;
            }
        }
        else if (buffer[0] == 0x05)
        {
            _rails[rail_index].calibration = static_cast<std::uint16_t>(value & 0xFFFEU);
        }
        return length;
    }
}
