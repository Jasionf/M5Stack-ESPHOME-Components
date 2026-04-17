#include "enable_switch.h"

namespace esphome {
namespace ns4150b {

void NS4150BEnableSwitch::write_state(bool state) {
  if (state) {
    this->parent_->set_mute_off();
  } else {
    this->parent_->set_mute_on();
  }
  this->publish_state(state);
}

}  // namespace ns4150b
}  // namespace esphome
