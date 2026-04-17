#include "ns450b.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ns450b {

static const char *const TAG = "ns450b";

void NS450BComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up NS450B controller...");

  if (this->mute_pin_ == nullptr) {
    ESP_LOGE(TAG, "mute_pin is required");
    this->mark_failed();
    return;
  }

  this->mute_pin_->setup();

  if (!this->apply_mute_state_(this->startup_muted_)) {
    ESP_LOGE(TAG, "Failed to apply startup mute state");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "NS450B control ready, active_low=%s", this->mute_active_low_ ? "true" : "false");
}

void NS450BComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "NS450B Controller:");
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Setup failed");
    return;
  }
  LOG_PIN("  Mute pin: ", this->mute_pin_);
  ESP_LOGCONFIG(TAG, "  Mute active low: %s", this->mute_active_low_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Current muted: %s", this->muted_ ? "YES" : "NO");
}

bool NS450BComponent::apply_mute_state_(bool mute) {
  if (this->mute_pin_ == nullptr) {
    ESP_LOGW(TAG, "mute_pin is not configured");
    return false;
  }

  const bool mute_level = this->mute_active_low_ ? false : true;
  const bool unmute_level = !mute_level;
  const bool pin_level = mute ? mute_level : unmute_level;

  this->mute_pin_->digital_write(pin_level);

  this->muted_ = mute;
  return true;
}

bool NS450BComponent::set_mute(bool mute) {
  return this->apply_mute_state_(mute);
}

void NS450BMuteSwitch::setup() {
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "NS450B parent is required for switch");
    this->mark_failed();
    return;
  }
  this->publish_state(this->parent_->is_muted());
}

void NS450BMuteSwitch::dump_config() {
  ESP_LOGCONFIG(TAG, "NS450B Mute Switch:");
  LOG_SWITCH("  ", "Mute", this);
}

void NS450BMuteSwitch::write_state(bool state) {
  if (this->parent_ != nullptr && this->parent_->set_mute(state)) {
    this->publish_state(state);
    return;
  }
  ESP_LOGW(TAG, "Failed to set mute switch state: %s", state ? "ON" : "OFF");
}

}  // namespace ns450b
}  // namespace esphome
