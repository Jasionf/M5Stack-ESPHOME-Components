#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/core/log.h"
#include "../m5stack_chain_angle.h"

namespace esphome {
namespace m5stack_chain_angle {

static const char *const TAG_ANGLE_OUTPUT = "m5stack_chain_angle.output";

class ChainAngleLedOutput : public output::FloatOutput, public Component {
 public:
  void set_parent(ChainAngleSensor *parent) { parent_ = parent; }

  void write_state(float state) override {
    if (parent_ == nullptr) {
      ESP_LOGW(TAG_ANGLE_OUTPUT, "Parent is null!");
      return;
    }

    if (state < 0.0f)
      state = 0.0f;
    if (state > 1.0f)
      state = 1.0f;

    uint8_t brightness = static_cast<uint8_t>(state * 100.0f);

    uint8_t op_status = 0;
    ChainStatus status = parent_->set_led_brightness(brightness, &op_status);

    ESP_LOGD(TAG_ANGLE_OUTPUT, "Set LED brightness= %u, status=0x%02X, op_status=%u", brightness,
             static_cast<uint8_t>(status), op_status);

    if (status != CHAIN_OK) {
      ESP_LOGW(TAG_ANGLE_OUTPUT, "Failed to set LED brightness (status=0x%02X)", static_cast<uint8_t>(status));
    }
  }

 protected:
  ChainAngleSensor *parent_{nullptr};
};

}  // namespace m5stack_chain_angle
}  // namespace esphome
