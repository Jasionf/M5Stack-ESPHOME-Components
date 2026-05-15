#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace m5unit_encoder {

static const uint8_t ENCODER_ADDR = 0x40;
static const uint8_t MODE_REG = 0x00;
static const uint8_t ENCODER_REG = 0x10;
static const uint8_t BUTTON_REG = 0x20;
static const uint8_t RGB_LED_REG = 0x30;

class M5UnitEncoder : public sensor::Sensor,
                       public PollingComponent,
                       public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_button_sensor(binary_sensor::BinarySensor *sensor) { this->button_sensor_ = sensor; }

  void set_led_color(uint8_t index, uint32_t color);
  void set_work_mode(uint8_t mode);

 protected:
  int16_t read_encoder_value_();
  bool read_button_status_();

  binary_sensor::BinarySensor *button_sensor_{nullptr};
};

}  // namespace m5unit_encoder
}  // namespace esphome
