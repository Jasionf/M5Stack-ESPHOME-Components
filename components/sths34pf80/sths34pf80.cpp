#include "sths34pf80.h"

#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace sths34pf80 {

static const char *const TAG = "sths34pf80";

// Main-page register addresses
static const uint8_t REG_LPF1         = 0x0C;
static const uint8_t REG_LPF2         = 0x0D;
static const uint8_t REG_WHO_AM_I     = 0x0F;
static const uint8_t REG_AVG_TRIM     = 0x10;
// 0x11 = PAGE_RW (accessible only when embedded bank is active via CTRL2 bit4)
static const uint8_t REG_CTRL1        = 0x20;
static const uint8_t REG_CTRL2        = 0x21;
static const uint8_t REG_STATUS       = 0x23;  // bit2 = DRDY
static const uint8_t REG_FUNC_STATUS  = 0x25;  // bit2=pres_flag, bit1=mot_flag, bit0=tamb_shock_flag
static const uint8_t REG_TOBJECT_L    = 0x26;
static const uint8_t REG_TPRESENCE_L  = 0x3A;
static const uint8_t REG_TMOTION_L    = 0x3C;

// Embedded-bank register addresses (used via func_cfg_write_)
static const uint8_t EMB_PRESENCE_THS  = 0x20;
static const uint8_t EMB_MOTION_THS    = 0x22;
static const uint8_t EMB_HYST_MOTION   = 0x26;
static const uint8_t EMB_HYST_PRESENCE = 0x27;
static const uint8_t EMB_RESET_ALGO    = 0x2A;

// ── helpers ──────────────────────────────────────────────────────────────────

bool STHS34PF80Component::read_bytes_(uint8_t reg, uint8_t *data, uint8_t len) {
  if (this->write(&reg, 1) != i2c::ERROR_OK) return false;
  return this->read(data, len) == i2c::ERROR_OK;
}

bool STHS34PF80Component::write_byte_(uint8_t reg, uint8_t value) {
  uint8_t buf[2] = {reg, value};
  return this->write(buf, 2) == i2c::ERROR_OK;
}

/**
 * Write @len bytes to the embedded-function configuration register space.
 * Mirrors sths34pf80_func_cfg_write() in the ST driver.
 * Assumes the sensor is already in power-down (ODR=0).
 */
bool STHS34PF80Component::func_cfg_write_(uint8_t embedded_addr, const uint8_t *data, uint8_t len) {
  // 1. Enable embedded-bank access: CTRL2 bit4 = func_cfg_access
  uint8_t ctrl2 = 0;
  if (!read_bytes_(REG_CTRL2, &ctrl2, 1)) return false;
  if (!write_byte_(REG_CTRL2, ctrl2 | 0x10)) return false;

  // 2. Enable write mode: PAGE_RW (addr 0x11 in embedded mode) bit6 = func_cfg_write
  if (!write_byte_(0x11, 0x40)) return false;

  // 3. Set starting address
  if (!write_byte_(0x08, embedded_addr)) return false;  // FUNC_CFG_ADDR

  // 4. Write each byte (address auto-increments)
  for (uint8_t i = 0; i < len; i++) {
    if (!write_byte_(0x09, data[i])) return false;  // FUNC_CFG_DATA
  }

  // 5. Disable write mode
  if (!write_byte_(0x11, 0x00)) return false;

  // 6. Return to main bank
  if (!write_byte_(REG_CTRL2, ctrl2 & ~0x10)) return false;

  return true;
}

bool STHS34PF80Component::algo_reset_() {
  uint8_t v = 0x01;
  return func_cfg_write_(EMB_RESET_ALGO, &v, 1);
}

// ── setup ────────────────────────────────────────────────────────────────────

void STHS34PF80Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up STHS34PF80…");

  // 1. WHO_AM_I
  uint8_t id = 0;
  if (!read_bytes_(REG_WHO_AM_I, &id, 1)) {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I");
    this->mark_failed();
    return;
  }
  if (id != 0xD3) {
    ESP_LOGE(TAG, "Unexpected WHO_AM_I=0x%02X (expected 0xD3)", id);
    this->mark_failed();
    return;
  }

  // 2. Boot OTP reset (CTRL2.boot = bit7)
  if (!write_byte_(REG_CTRL2, 0x80)) {
    ESP_LOGW(TAG, "Failed to set boot bit");
  }
  delay(3);  // wait ≥3 ms for OTP reload

  // 3. AVG_TRIM: avg_tmos=2 → 32 object averages (bits[2:0])
  //              avg_t=0   → 8 ambient averages   (bits[5:4])
  //              result = 0x02
  if (!write_byte_(REG_AVG_TRIM, 0x02)) {
    ESP_LOGW(TAG, "Failed to set AVG_TRIM");
  }

  // 4. Enter power-down: CTRL1 = BDU=1 (bit4), ODR=0000
  if (!write_byte_(REG_CTRL1, 0x10)) {
    ESP_LOGW(TAG, "Failed to enter power-down");
  }

  // 5. Presence threshold (15-bit, little-endian in embedded space)
  {
    uint8_t buf[2] = {static_cast<uint8_t>(presence_threshold_ & 0xFF),
                      static_cast<uint8_t>(presence_threshold_ >> 8)};
    if (!func_cfg_write_(EMB_PRESENCE_THS, buf, 2)) {
      ESP_LOGW(TAG, "Failed to set presence threshold");
    }
  }

  // 6. Motion threshold
  {
    uint8_t buf[2] = {static_cast<uint8_t>(motion_threshold_ & 0xFF),
                      static_cast<uint8_t>(motion_threshold_ >> 8)};
    if (!func_cfg_write_(EMB_MOTION_THS, buf, 2)) {
      ESP_LOGW(TAG, "Failed to set motion threshold");
    }
  }

  // 7. Motion hysteresis
  if (!func_cfg_write_(EMB_HYST_MOTION, &motion_hysteresis_, 1)) {
    ESP_LOGW(TAG, "Failed to set motion hysteresis");
  }

  // 8. Presence hysteresis
  if (!func_cfg_write_(EMB_HYST_PRESENCE, &presence_hysteresis_, 1)) {
    ESP_LOGW(TAG, "Failed to set presence hysteresis");
  }

  // 9. Reset algorithm state (clears stale presence/motion accumulators)
  if (!algo_reset_()) {
    ESP_LOGW(TAG, "algo_reset failed");
  }

  // 10. Start ODR=1 Hz, BDU=1: CTRL1 = 0b0001_0011
  //     bits[3:0]=ODR=3 (1 Hz), bit4=BDU=1
  if (!write_byte_(REG_CTRL1, 0x13)) {
    ESP_LOGW(TAG, "Failed to start ODR");
  }
}

// ── dump_config ──────────────────────────────────────────────────────────────

void STHS34PF80Component::dump_config() {
  ESP_LOGCONFIG(TAG, "STHS34PF80:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Communication with STHS34PF80 failed!");
    return;
  }
  ESP_LOGCONFIG(TAG, "  Presence threshold: %u", presence_threshold_);
  ESP_LOGCONFIG(TAG, "  Motion threshold:   %u", motion_threshold_);
  ESP_LOGCONFIG(TAG, "  Presence hysteresis: %u", presence_hysteresis_);
  ESP_LOGCONFIG(TAG, "  Motion hysteresis:   %u", motion_hysteresis_);
}

// ── update ───────────────────────────────────────────────────────────────────

void STHS34PF80Component::update() {
  if (this->is_failed()) return;

  // Check data-ready flag (STATUS reg bit2)
  uint8_t status = 0;
  if (!read_bytes_(REG_STATUS, &status, 1)) {
    ESP_LOGW(TAG, "Failed to read STATUS register");
    return;
  }
  if (!(status & 0x04)) {
    // No new data yet – skip this cycle
    return;
  }

  // Read FUNC_STATUS for hardware-computed presence/motion flags
  uint8_t func_status = 0;
  if (read_bytes_(REG_FUNC_STATUS, &func_status, 1)) {
    bool pres = (func_status >> 2) & 0x01;
    bool mot  = (func_status >> 1) & 0x01;
    if (pres_flag_sensor_ != nullptr) pres_flag_sensor_->publish_state(pres);
    if (mot_flag_sensor_  != nullptr) mot_flag_sensor_->publish_state(mot);
  }

  uint8_t buf[2];

  // Object temperature: raw / 2000 °C (relative to ambient)
  if (temperature_sensor_ != nullptr) {
    if (read_bytes_(REG_TOBJECT_L, buf, 2)) {
      int16_t raw = static_cast<int16_t>(buf[0] | (static_cast<uint16_t>(buf[1]) << 8));
      temperature_sensor_->publish_state(static_cast<float>(raw) / 2000.0f);
    } else {
      ESP_LOGW(TAG, "Failed to read object temperature");
    }
  }

  // Presence value (algorithm output, raw counts)
  if (presence_sensor_ != nullptr) {
    if (read_bytes_(REG_TPRESENCE_L, buf, 2)) {
      int16_t raw = static_cast<int16_t>(buf[0] | (static_cast<uint16_t>(buf[1]) << 8));
      presence_sensor_->publish_state(static_cast<float>(raw));
    } else {
      ESP_LOGW(TAG, "Failed to read presence value");
    }
  }

  // Motion value (algorithm output, raw counts)
  if (motion_sensor_ != nullptr) {
    if (read_bytes_(REG_TMOTION_L, buf, 2)) {
      int16_t raw = static_cast<int16_t>(buf[0] | (static_cast<uint16_t>(buf[1]) << 8));
      motion_sensor_->publish_state(static_cast<float>(raw));
    } else {
      ESP_LOGW(TAG, "Failed to read motion value");
    }
  }
}

}  // namespace sths34pf80
}  // namespace esphome
