#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <set>
#include <utility>
#include <vector>

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
          _input_max_voltage(config.get("simulator.hardware-model.physical.input-max-voltage", 16.0)),
          _input_current_limit_amps(config.get(
              "simulator.hardware-model.physical.input-current-limit-amps", 2.0)),
          _converter_efficiency(config.get(
              "simulator.hardware-model.physical.converter-efficiency", 0.90)),
          _loads(),
          _start_time(std::chrono::steady_clock::now())
    {
        const std::string connection = config.get("common.nos-connection-string",
                                                  "tcp://127.0.0.1:12001");

        if (!std::isfinite(_input_voltage) || !std::isfinite(_input_min_voltage) ||
            !std::isfinite(_input_max_voltage) || !std::isfinite(_input_current_limit_amps) ||
            !std::isfinite(_converter_efficiency) || _input_voltage < 0.0 ||
            _input_min_voltage < 0.0 || _input_min_voltage > _input_max_voltage)
        {
            throw std::invalid_argument("Sentinel Bio EPS: invalid bench input voltage/range");
        }
        if (_input_current_limit_amps <= 0.0 || _converter_efficiency <= 0.0 ||
            _converter_efficiency > 1.0)
        {
            throw std::invalid_argument("Sentinel Bio EPS: input current limit must be positive and efficiency must be in (0, 1]");
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
            _rails[i].max_current_amps = config.get(prefix + ".max-current-amps", 5.0);
            _rails[i].shunt_ohms = config.get(prefix + ".shunt-ohms", 0.1);
            _rails[i].dropout_voltage = config.get(prefix + ".dropout-voltage", 0.0);
            _rails[i].configuration = config.get(prefix + ".configuration", INA219_DEFAULT_CONFIGURATION);
            _rails[i].calibration = static_cast<std::uint16_t>(
                config.get(prefix + ".calibration", INA219_DEFAULT_CALIBRATION) & 0xFFFEU);
            const Rail& rail = _rails[i];
            if (!std::isfinite(rail.nominal_voltage) || !std::isfinite(rail.current_amps) ||
                !std::isfinite(rail.max_current_amps) ||
                !std::isfinite(rail.shunt_ohms) || !std::isfinite(rail.dropout_voltage) ||
                rail.nominal_voltage < 0.0 || rail.nominal_voltage > 26.0 ||
                rail.current_amps < 0.0 || rail.max_current_amps <= 0.0 ||
                rail.shunt_ohms <= 0.0 || rail.dropout_voltage < 0.0 ||
                rail.nominal_voltage + rail.dropout_voltage > 35.0)
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

        const std::string loads_path = "simulator.hardware-model.physical.loads";
        if (config.get_child_optional(loads_path))
        {
            BOOST_FOREACH(const boost::property_tree::ptree::value_type& item,
                          config.get_child(loads_path))
            {
                if (item.first != "load")
                {
                    continue;
                }

                LoadPulse load;
                load.name = item.second.get<std::string>("name", "unnamed-load");
                const std::string rail_name = item.second.get<std::string>("rail", "");
                load.rail = RAIL_COUNT;
                for (std::size_t i = 0; i < RAIL_COUNT; ++i)
                {
                    if (rail_name == RAIL_NAMES[i])
                    {
                        load.rail = i;
                        break;
                    }
                }
                load.current_amps = item.second.get<double>("current-amps", 0.0);
                load.start_seconds = item.second.get<double>("start-seconds", 0.0);
                load.duration_seconds = item.second.get<double>("duration-seconds", 0.0);
                load.period_seconds = item.second.get<double>("period-seconds", 0.0);

                if (load.rail >= RAIL_COUNT || !std::isfinite(load.current_amps) ||
                    !std::isfinite(load.start_seconds) || !std::isfinite(load.duration_seconds) ||
                    !std::isfinite(load.period_seconds) || load.current_amps < 0.0 ||
                    load.start_seconds < 0.0 || load.duration_seconds <= 0.0 ||
                    load.period_seconds < 0.0 ||
                    (load.period_seconds > 0.0 && load.duration_seconds > load.period_seconds))
                {
                    throw std::invalid_argument("Sentinel Bio EPS: invalid timed load profile " + load.name);
                }
                _loads.push_back(load);
                sim_logger->info("Sentinel Bio FlatSat EPS: load %s adds %.3f A to %s",
                                 load.name.c_str(), load.current_amps,
                                 RAIL_NAMES[load.rail]);
            }
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

    double Sentinelbio_flatsat_epsHardwareModel::requested_current(
        std::size_t rail_index, double elapsed_seconds) const
    {
        double current = _rails[rail_index].current_amps;
        for (const LoadPulse& load : _loads)
        {
            if (load.rail != rail_index || elapsed_seconds < load.start_seconds)
            {
                continue;
            }

            const double load_elapsed = elapsed_seconds - load.start_seconds;
            bool active = false;
            if (load.period_seconds == 0.0)
            {
                active = load_elapsed < load.duration_seconds;
            }
            else
            {
                active = std::fmod(load_elapsed, load.period_seconds) < load.duration_seconds;
            }
            if (active)
            {
                current += load.current_amps;
            }
        }
        return current;
    }

    void Sentinelbio_flatsat_epsHardwareModel::calculate_operating_point(
        double elapsed_seconds, std::array<double, 3>& voltages,
        std::array<double, 3>& currents) const
    {
        double total_output_power = 0.0;
        for (std::size_t i = 0; i < RAIL_COUNT; ++i)
        {
            const Rail& rail = _rails[i];
            const double requested = requested_current(i, elapsed_seconds);
            currents[i] = std::min(requested, rail.max_current_amps);

            if (_input_voltage + 1.0e-9 >= rail.nominal_voltage + rail.dropout_voltage)
            {
                voltages[i] = rail.nominal_voltage;
            }
            else
            {
                voltages[i] = std::max(0.0, _input_voltage - rail.dropout_voltage);
            }

            /* Approximate converter current limiting: once a rail's demanded
             * current exceeds its configured limit, its voltage droops in
             * proportion to the delivered-current fraction. */
            if (requested > rail.max_current_amps && requested > 0.0)
            {
                voltages[i] *= rail.max_current_amps / requested;
            }
            total_output_power += voltages[i] * currents[i];
        }

        /* The three buck converters share the PSU's input power budget. This
         * first-order model treats the programmed loads as constant-current
         * sinks and lowers all output voltages proportionally if that budget
         * is exceeded. */
        const double available_output_power =
            _input_voltage * _input_current_limit_amps * _converter_efficiency;
        if (total_output_power > available_output_power && total_output_power > 0.0)
        {
            const double voltage_scale = available_output_power / total_output_power;
            for (std::size_t i = 0; i < RAIL_COUNT; ++i)
            {
                voltages[i] *= voltage_scale;
            }
        }
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
        const double elapsed_seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - _start_time).count();
        std::array<double, RAIL_COUNT> bus_voltages;
        std::array<double, RAIL_COUNT> rail_currents;
        calculate_operating_point(elapsed_seconds, bus_voltages, rail_currents);

        const double bus_voltage = bus_voltages[rail_index];
        const double current_amps = rail_currents[rail_index];
        const long raw_shunt_code = std::lround(
            (current_amps * rail.shunt_ohms) / 0.00001);
        const long pga = static_cast<long>((rail.configuration >> 11) & 0x0003U);
        const long shunt_limit = 4000L << pga; // 40, 80, 160, or 320 mV at 10 uV/LSB
        const long shunt_code = std::max(-shunt_limit,
                                         std::min(shunt_limit, raw_shunt_code));
        const long bus_code = std::lround(bus_voltage / 0.004);
        const long current_code = (rail.calibration == 0)
                                      ? 0
                                      : (shunt_code * rail.calibration) / 4096L;
        const long power_code = (current_code * bus_code) / 5000L;
        const bool overflow = raw_shunt_code > shunt_limit ||
                              raw_shunt_code < -shunt_limit ||
                              current_code < -32768L || current_code > 32767L ||
                              power_code < 0 || power_code > 65535L;
        std::uint16_t value = 0;

        switch (reg)
        {
            case 0x00:
                value = rail.configuration;
                break;

            case 0x01:
                value = static_cast<std::uint16_t>(clamp_signed(shunt_code));
                break;

            case 0x02:
            {
                const std::uint16_t clamped = static_cast<std::uint16_t>(
                    std::max(0L, std::min(8191L, bus_code)));
                /* Steady-state approximation: conversions complete immediately.
                 * No ADC timing or CNVR clearing on Power reads is modeled. */
                value = static_cast<std::uint16_t>((clamped << 3) | 0x0002 |
                                                   (overflow ? 0x0001 : 0));
                break;
            }

            case 0x03:
                value = clamp_unsigned(power_code);
                break;

            case 0x04:
                value = static_cast<std::uint16_t>(clamp_signed(current_code));
                break;

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
