#include "ns4150b.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ns4150b {

static const char *const TAG = "ns4150b";

void NS4150B::dump_config() {
  ESP_LOGCONFIG(TAG, "NS4150B Class D Power Amplifier:");
  LOG_PIN("  Enable Pin: ", this->enable_pin_);
}

}  // namespace ns4150b
}  // namespace esphome
