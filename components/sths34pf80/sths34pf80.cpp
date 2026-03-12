#include "sths34pf80.h"

#include "esphome/core/log.h"

namespace esphome {
namespace sths34pf80 {

static const char *const TAG = "sths34pf80";

// Register addresses (7-bit I2C address is 0x5A)
static const uint8_t STHS34PF80_REG_WHO_AM_I = 0x0F;
static const uint8_t STHS34PF80_REG_CTRL1    = 0x20;
static const uint8_t STHS34PF80_REG_TOBJECT_L = 0x26;
static const uint8_t STHS34PF80_REG_TPRESENCE_L = 0x3A;
static const uint8_t STHS34PF80_REG_TMOTION_L   = 0x3C;

void STHS34PF80Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up STHS34PF80...");

  uint8_t id = 0;
  if (!this->read_bytes_(STHS34PF80_REG_WHO_AM_I, &id, 1)) {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I register");
    this->mark_failed();
    return;
  }

  if (id != 0xD3) {
    ESP_LOGE(TAG, "Unexpected WHO_AM_I=0x%02X (expected 0xD3)", id);
    this->mark_failed();
    return;
  }

  // Configure output data rate to ~1Hz and enable block data update.
  // CTRL1 layout: lower 4 bits are ODR, bit 4 is BDU.
  // Value 0x13 -> ODR=3 (1Hz), BDU=1.
  if (!this->write_byte_(STHS34PF80_REG_CTRL1, 0x13)) {
    ESP_LOGW(TAG, "Failed to configure CTRL1 register");
  }
}

void STHS34PF80Component::dump_config() {
  ESP_LOGCONFIG(TAG, "STHS34PF80:");
  LOG_I2C_DEVICE(this);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Communication with STHS34PF80 failed!");
    return;
  }
}

void STHS34PF80Component::update() {
  if (this->is_failed()) {
    ESP_LOGW(TAG, "STHS34PF80 component is failed, skipping update");
    return;
  }

  uint8_t buf[2];

  // Object temperature: raw value / 2000.0
  if (this->temperature_sensor_ != nullptr) {
    if (this->read_bytes_(STHS34PF80_REG_TOBJECT_L, buf, 2)) {
      int16_t raw = static_cast<int16_t>(buf[0] | (static_cast<int16_t>(buf[1]) << 8));
      float temp_c = static_cast<float>(raw) / 2000.0f;
      this->temperature_sensor_->publish_state(temp_c);
    } else {
      ESP_LOGW(TAG, "Failed to read object temperature");
    }
  }

  // Presence value (raw units from sensor)
  if (this->presence_sensor_ != nullptr) {
    if (this->read_bytes_(STHS34PF80_REG_TPRESENCE_L, buf, 2)) {
      int16_t raw = static_cast<int16_t>(buf[0] | (static_cast<int16_t>(buf[1]) << 8));
      this->presence_sensor_->publish_state(static_cast<float>(raw));
    } else {
      ESP_LOGW(TAG, "Failed to read presence value");
    }
  }

  // Motion value (raw units from sensor)
  if (this->motion_sensor_ != nullptr) {
    if (this->read_bytes_(STHS34PF80_REG_TMOTION_L, buf, 2)) {
      int16_t raw = static_cast<int16_t>(buf[0] | (static_cast<int16_t>(buf[1]) << 8));
      this->motion_sensor_->publish_state(static_cast<float>(raw));
    } else {
      ESP_LOGW(TAG, "Failed to read motion value");
    }
  }
}

bool STHS34PF80Component::read_bytes_(uint8_t reg, uint8_t *data, uint8_t len) {
  if (len == 0 || data == nullptr) {
    return false;
  }

  if (this->write(&reg, 1) != i2c::ERROR_OK) {
    return false;
  }

  if (this->read(data, len) != i2c::ERROR_OK) {
    return false;
  }

  return true;
}

bool STHS34PF80Component::write_byte_(uint8_t reg, uint8_t value) {
  uint8_t buf[2] = {reg, value};
  return this->write(buf, 2) == i2c::ERROR_OK;
}

}  // namespace sths34pf80
}  // namespace esphome
