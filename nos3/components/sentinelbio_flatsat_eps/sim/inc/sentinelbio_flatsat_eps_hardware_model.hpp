#ifndef NOS3_SENTINELBIO_FLATSAT_EPS_HARDWARE_MODEL_HPP
#define NOS3_SENTINELBIO_FLATSAT_EPS_HARDWARE_MODEL_HPP

#include <array>
#include <atomic>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <I2C/Client/I2CSlave.hpp>
#include <sim_i_hardware_model.hpp>

namespace Nos3
{
    class Sentinelbio_flatsat_epsHardwareModel;

    class SentinelbioFlatsatEpsI2CSlave : public NosEngine::I2C::I2CSlave
    {
    public:
        SentinelbioFlatsatEpsI2CSlave(Sentinelbio_flatsat_epsHardwareModel* model,
                                      std::size_t rail, int address,
                                      const std::string& connection, const std::string& bus);

        std::size_t i2c_read(std::uint8_t* buffer, std::size_t length) override;
        std::size_t i2c_write(const std::uint8_t* buffer, std::size_t length) override;

    private:
        Sentinelbio_flatsat_epsHardwareModel* _model;
        std::size_t _rail;
        std::atomic<std::uint8_t> _register;
    };

    class Sentinelbio_flatsat_epsHardwareModel : public SimIHardwareModel
    {
    public:
        explicit Sentinelbio_flatsat_epsHardwareModel(const boost::property_tree::ptree& config);
        ~Sentinelbio_flatsat_epsHardwareModel() override;

        std::size_t read_register(std::size_t rail, std::uint8_t reg,
                                  std::uint8_t* buffer, std::size_t length) const;
        std::size_t write_register(std::size_t rail, const std::uint8_t* buffer,
                                   std::size_t length);

    private:
        struct Rail
        {
            double nominal_voltage;
            double current_amps;
            double shunt_ohms;
            double dropout_voltage;
            std::uint16_t configuration;
            std::uint16_t calibration;
        };

        double effective_voltage(const Rail& rail) const;
        static std::uint16_t clamp_unsigned(long value);
        static std::int16_t clamp_signed(long value);

        std::array<Rail, 3> _rails;
        std::array<std::unique_ptr<SentinelbioFlatsatEpsI2CSlave>, 3> _slaves;
        double _input_voltage;
        double _input_min_voltage;
        double _input_max_voltage;
        mutable std::mutex _mutex;
    };
}

#endif
