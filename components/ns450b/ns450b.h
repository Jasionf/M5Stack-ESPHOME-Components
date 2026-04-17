#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace ns450b {

class NS450BComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void set_mute_pin(GPIOPin *pin) { this->mute_pin_ = pin; }
  void set_mute_active_low(bool active_low) { this->mute_active_low_ = active_low; }
  void set_startup_muted(bool startup_muted) { this->startup_muted_ = startup_muted; }

  bool set_mute(bool mute);
  bool is_muted() const { return this->muted_; }

 protected:
  bool apply_mute_state_(bool mute);

  GPIOPin *mute_pin_{nullptr};
  bool mute_active_low_{true};
  bool startup_muted_{false};
  bool muted_{false};
};

class NS450BMuteSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(NS450BComponent *parent) { this->parent_ = parent; }
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
  NS450BComponent *parent_{nullptr};
};

}  // namespace ns450b
}  // namespace esphome
