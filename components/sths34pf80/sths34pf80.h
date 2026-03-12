#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace sths34pf80 {

class STHS34PF80Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void update() override;

  void set_presence_sensor(sensor::Sensor *sensor) { presence_sensor_ = sensor; }
  void set_motion_sensor(sensor::Sensor *sensor) { motion_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { temperature_sensor_ = sensor; }

 protected:
  bool read_bytes_(uint8_t reg, uint8_t *data, uint8_t len);
  bool write_byte_(uint8_t reg, uint8_t value);

  sensor::Sensor *presence_sensor_{nullptr};
  sensor::Sensor *motion_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
};

}  // namespace sths34pf80
}  // namespace esphome
