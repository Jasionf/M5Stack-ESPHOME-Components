#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/core/log.h"
#include "../m5stack_chain_key.h"

namespace esphome {
namespace m5stack_chain_key {

static const char *const TAG_KEY_OUTPUT = "m5stack_chain_key.output";

// 通道定义参照 unit_step16 OutputChannel：亮度 + RGB 颜色
enum LedChannel : uint8_t {
  LED_BRIGHTNESS = 0,
  RGB_BRIGHTNESS = 1,
  RGB_RED = 2,
  RGB_GREEN = 3,
  RGB_BLUE = 4,
};

class ChainKeyLedOutput : public output::FloatOutput, public Component {
 public:
  void set_parent(ChainKeyBinarySensor *parent) { parent_ = parent; }
  void set_channel(LedChannel channel) { channel_ = channel; }

  void write_state(float state) override {
    if (parent_ == nullptr) {
      ESP_LOGW(TAG_KEY_OUTPUT, "Parent is null!");
      return;
    }

    if (state < 0.0f)
      state = 0.0f;
    if (state > 1.0f)
      state = 1.0f;

    uint8_t op_status = 0;

    ESP_LOGD(TAG_KEY_OUTPUT, "Channel %d: write_state(%.2f)", static_cast<int>(channel_), state);

    switch (channel_) {
      case LED_BRIGHTNESS: {
        // 单色灯亮度：0.0-1.0 -> 0-100，对应板载 RGB 灯的整体亮度
        uint8_t brightness = static_cast<uint8_t>(state * 100.0f);
        ChainStatus status = parent_->set_led_brightness(brightness, &op_status);

        ESP_LOGD(TAG_KEY_OUTPUT, "Set LED brightness= %u, status=0x%02X, op_status=%u", brightness,
                 static_cast<uint8_t>(status), op_status);

        if (status != CHAIN_OK) {
          ESP_LOGW(TAG_KEY_OUTPUT, "Failed to set LED brightness (status=0x%02X)", static_cast<uint8_t>(status));
        }

        // 第一次设置亮度时，如果还没设置颜色，默认给一个白色，保证肉眼可见
        if (!color_initialized_) {
          r_ = g_ = b_ = 255;
          parent_->set_rgb_color(r_, g_, b_, &op_status);
          color_initialized_ = true;
        }
        break;
      }
      case RGB_BRIGHTNESS: {
        // RGB 硬件亮度通道：和 LED_BRIGHTNESS 一样，方便和 unit_step16 对齐
        uint8_t brightness = static_cast<uint8_t>(state * 100.0f);
        ChainStatus status = parent_->set_led_brightness(brightness, &op_status);

        ESP_LOGD(TAG_KEY_OUTPUT, "Set RGB brightness= %u, status=0x%02X, op_status=%u", brightness,
                 static_cast<uint8_t>(status), op_status);

        if (status != CHAIN_OK) {
          ESP_LOGW(TAG_KEY_OUTPUT, "Failed to set RGB brightness (status=0x%02X)", static_cast<uint8_t>(status));
        }
        break;
      }
      case RGB_RED:
      case RGB_GREEN:
      case RGB_BLUE: {
        // 颜色通道：0.0-1.0 -> 0-255
        uint8_t value = static_cast<uint8_t>(state * 255.0f);

        if (!color_initialized_) {
          uint8_t r = 0, g = 0, b = 0;
          if (parent_->get_rgb_color(&r, &g, &b, &op_status) == CHAIN_OK) {
            r_ = r;
            g_ = g;
            b_ = b;
          }
          color_initialized_ = true;
        }

        switch (channel_) {
          case RGB_RED:
            r_ = value;
            break;
          case RGB_GREEN:
            g_ = value;
            break;
          case RGB_BLUE:
            b_ = value;
            break;
          default:
            break;
        }

        ChainStatus status = parent_->set_rgb_color(r_, g_, b_, &op_status);

        ESP_LOGD(TAG_KEY_OUTPUT, "Set RGB color: R=%u G=%u B=%u, status=0x%02X, op_status=%u", r_, g_, b_,
                 static_cast<uint8_t>(status), op_status);

        if (status != CHAIN_OK) {
          ESP_LOGW(TAG_KEY_OUTPUT, "Failed to set RGB color (status=0x%02X)", static_cast<uint8_t>(status));
        }
        break;
      }
      default:
        break;
    }
  }

 protected:
  ChainKeyBinarySensor *parent_{nullptr};
  LedChannel channel_{LED_BRIGHTNESS};
  uint8_t r_{0}, g_{0}, b_{0};
  bool color_initialized_{false};
};

}  // namespace m5stack_chain_key
}  // namespace esphome
