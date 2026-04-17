#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/helpers.h"
#include "../ns4150b.h"

namespace esphome {
namespace ns4150b {

/// Switch entity that exposes the NS4150B enable (E1502AM5G EN) state.
/// ON  = amplifier enabled  (EN pin HIGH)
/// OFF = amplifier disabled (EN pin LOW / shutdown)
class NS4150BEnableSwitch : public switch_::Switch, public Parented<NS4150B> {
 protected:
  void write_state(bool state) override;
};

}  // namespace ns4150b
}  // namespace esphome
