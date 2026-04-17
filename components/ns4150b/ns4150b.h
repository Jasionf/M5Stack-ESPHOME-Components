#pragma once

#include "esphome/components/audio_dac/audio_dac.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ns4150b {

/// NS4150B Class D audio power amplifier component.
/// Controlled via a single GPIO enable/shutdown pin (active high = enabled).
class NS4150B : public audio_dac::AudioDac, public Component {
 public:
  void set_enable_pin(GPIOPin *pin) { this->enable_pin_ = pin; }
  void set_enable_switch(switch_::Switch *s) { this->enable_switch_ = s; }

  void setup() override {
    this->enable_pin_->setup();
    // Default: enable the amplifier (not muted)
    this->enable_pin_->digital_write(true);
    this->is_muted_ = false;
    // Publish initial state to the switch entity if registered
    if (this->enable_switch_ != nullptr)
      this->enable_switch_->publish_state(true);
  }

  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

  /// Unmute: drive enable pin HIGH to activate amplifier
  bool set_mute_off() override {
    this->enable_pin_->digital_write(true);
    this->is_muted_ = false;
    if (this->enable_switch_ != nullptr)
      this->enable_switch_->publish_state(true);
    return true;
  }

  /// Mute: drive enable pin LOW to shut down amplifier
  bool set_mute_on() override {
    this->enable_pin_->digital_write(false);
    this->is_muted_ = true;
    if (this->enable_switch_ != nullptr)
      this->enable_switch_->publish_state(false);
    return true;
  }

  /// NS4150B has no volume register; volume is controlled by the DAC upstream.
  bool set_volume(float volume) override {
    this->volume_ = volume;
    return true;
  }

  bool is_muted() override { return this->is_muted_; }
  float volume() override { return this->volume_; }

 protected:
  GPIOPin *enable_pin_{nullptr};
  switch_::Switch *enable_switch_{nullptr};
  float volume_{1.0f};
};

}  // namespace ns4150b
}  // namespace esphome
