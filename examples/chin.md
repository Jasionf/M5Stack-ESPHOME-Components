# Chain DualKey Home Assistant Integration

<div style="display: flex; gap: 15px; flex-wrap: wrap;">
  <img
   src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/C147_chain-dualkey-mainpicture_01.webp"
    width="300px"
  />
</div>

## Introduction

**Chain DualKey** is a programmable dual-key input development board equipped with the ESP32-S3FN8 main control chip. The front integrates 2 hot-swappable blue switch mechanical keyboard keys and 2 programmable RGB LEDs, providing excellent interactive feedback. It has a built-in 350mAh lithium battery, combining with a low-power design for good battery life. The product comes with pre-installed Chain macro keyboard firmware, supports USB / BLE connections, and can emulate HID input devices. After the device is powered on, you can connect to the device's AP hotspot and configure the HID function mapping for the local device or expansion nodes via the built-in web page to achieve various control functions. This development board adopts the M5Stack Chain series expandable design, featuring two HY2.0-4P expansion ports that support lateral expansion and connection to other sensor devices. With the USB-OTG peripheral function built into ESP32-S3, it is suitable for smart home, keyboard peripherals, macro keyboards, and other scenarios.

## Preparation

- Home Assistant host
- Install and enable [ESPHome Builder](https://esphome.io/guides/getting_started_hassio/) in Home Assistant
- [Chain DualKey](https://docs.m5stack.com/en/chain/Chain_DualKey)
- [Chain Angel](https://docs.m5stack.com/en/chain/Chain_Angle)
- [Chain Encoder](https://docs.m5stack.com/en/chain/Chain_Encoder)
- [Chain Key](https://docs.m5stack.com/en/chain/Chain_Key)
- [Chain Joystick](https://docs.m5stack.com/en/chain/Chain_Joystick)
- [Chian Tof](https://docs.m5stack.com/en/chain/Chain_ToF)

\#> Tip | In this tutorial, the firmware is compiled and uploaded with ESPHome 2025.1.2. If you encounter compile/upload issues, consider switching ESPHome to this version.

**Step 1. Create New Device**

- Click the green button in the lower right corner to create a device.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA28.webp"
width="700px"
/>

**Step 2. Create Device Name**

- Click `CONTINUE`.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA1.webp"
width="300px"
/>

- Click `New Device Setup`.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA3.webp"
width="300px"
/>

- Enter the name of the device and click `NEXT`.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/HA6.webp"
width="400px"
/>

**Step 3. Choose Device Type**

- Click `ESP32-S3`.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA5.webp"
width="300px"
/>

- Click `SKIP`.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA27.webp"
width="300px"
/>

**Step 4. Start Edit YAML File**

- Click `EDIT`. We can customize device functionality through YAML files.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/HA4.webp"
width="700px"
/>

## Device Setup

The Master acts as the main controller of the system. When connecting expansion sensors, it is important to correctly identify both the connection direction and the ID order.

<img src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/chain_bus_device_id_01.jpg" width="70%">

**Direction Selection**

Choose the appropriate uart_id based on which side of the Master the expansion sensor is connected:
- Connected to the left side of the Master → use chain_uart_left
- Connected to the right side of the Master → use chain_uart_right

**ID Numbering Rules**

The chain_id represents the position of the expansion sensor relative to the Master:
- Numbering starts from the module closest to the Master
- IDs increase sequentially (ID:1 → ID:2 → ID:3 …)
- The left and right sides are numbered independently
- Refer to the diagram above to determine the correct order

**Configuration Example**

- `uart_id:` chain_uart_left
- `chain_id:` 1
  
### Chain DualKey

```yaml
uart:
  - id: chain_uart_right
    tx_pin: GPIO6
    rx_pin: GPIO5
    baud_rate: 115200

  - id: chain_uart_left
    tx_pin: GPIO48
    rx_pin: GPIO47
    baud_rate: 115200

sensor:
  - platform: adc
    pin: GPIO10
    name: "ADC_BAT"
    update_interval: 1s

  - platform: adc
    pin: GPIO2
    name: "ADC_VBUS"
    update_interval: 1s

  - platform: adc
    pin: GPIO9
    name: "ADC_CHARGE"
    update_interval: 1s

output:
  - platform: gpio
    id: pwr_en
    pin: GPIO40

light:
  - platform: esp32_rmt_led_strip
    id: key_light_raw
    internal: true
    pin: GPIO21
    num_leds: 2
    chipset: ws2812
    rgb_order: GRB
    restore_mode: ALWAYS_OFF

  - platform: partition
    name: "Key Light 1"
    id: key_light_1
    segments:
      - id: key_light_raw
        from: 0
        to: 0

  - platform: partition
    name: "Key Light 2"
    id: key_light_2
    segments:
      - id: key_light_raw
        from: 1
        to: 1

binary_sensor:

  - platform: gpio
    name: "KEY_2"
    pin:
      number: GPIO17
      inverted: true
      mode: INPUT_PULLUP
    filters:
      - delayed_on: 10ms
      - delayed_off: 10ms

    on_press:
      - light.turn_on:
          id: key_light_1
          transition_length: 0ms

    on_release:
      - light.turn_off: key_light_1

  - platform: gpio
    name: "KEY_1"
    pin:
      number: GPIO0
      inverted: true
      mode: INPUT_PULLUP
    filters:
      - delayed_on: 10ms
      - delayed_off: 10ms

    on_press:
      - light.turn_on:
          id: key_light_2
          transition_length: 0ms

    on_release:
      - light.turn_off: key_light_2

  - platform: gpio
    name: "SWITCH_1"
    pin:xs
      number: GPIO7
      mode: INPUT

  - platform: gpio
    name: "SWITCH_2"
    pin:
      number: GPIO8
      mode: INPUT
```

### Chain Key

```yaml
external_components:
  - source: github://m5stack/esphome-yaml/components
    components: [m5stack_chain_key]
    refresh: 0s

binary_sensor:
  - platform: m5stack_chain_key
    name: "Chain Key Right 1"
    uart_id: xx
    chain_id: xx        
    update_interval: 50ms
```
### Chain Angle

```yaml
external_components:
  - source: github://m5stack/esphome-yaml/components
    components: [m5stack_chain_angle]
    refresh: 0s

sensor:
  - platform: m5stack_chain_angle
    name: "Chain Angle Left"
    uart_id: xx
    chain_id: xx
    update_interval: 50ms
```
### Chain Encoder

```yaml
external_components:
  - source: github://m5stack/esphome-yaml/components
    components: [m5stack_chain_encoder]
    refresh: 0s

sensor:
  - platform: m5stack_chain_encoder
    name: "Chain Encoder"
    uart_id: xx   
    chain_id: xx                
    update_interval: 100ms
```
### Chain Joystick

```yaml
external_components:
  - source: github://m5stack/esphome-yaml/components
    components: [m5stack_chain_joystick]
    refresh: 0s

sensor:
  - platform: m5stack_chain_joystick
    name: "Chain Joystick Left X"
    uart_id: xx
    chain_id: xx
    axis: x
    update_interval: 50ms

  - platform: m5stack_chain_joystick
    name: "Chain Joystick Left Y"
    uart_id: xx
    chain_id: xx
    axis: y
    update_interval: 50ms
```
### Chain Tof

```yaml
external_components:
  - source: github://m5stack/esphome-yaml/components
    components: [m5stack_chain_tof]
    refresh: 0s

sensor:
  - platform: m5stack_chain_tof
    name: "Chain Tof"
    uart_id: xx   
    chain_id: xx                
    update_interval: 100ms
```
## Example

The following code example is configured according to the connection order shown in the diagram above.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/example.png"
width="700px"
/>

### External Components
**Modules used**: Chain Angle, Chain Encoder, Chain ToF, Chain Joystick, Chain Key.

- Add the [External](https://esphome.io/components/external_components/) components
  
This block imports all required external components for the Chain series modules. If you do not use a certain module (for example, Chain Encoder or Chain ToF), you can remove it from the components list.

```yaml
external_components:
  - source: github://m5stack/esphome-yaml/components
    components: [m5stack_chain_angle,m5stack_chain_encoder,m5stack_chain_tof,m5stack_chain_joystick,m5stack_chain_key]
    refresh: 0s
```

### UART Components
**Modules used**: Shared UART bus for all Chain series modules on left/right HY2.0 ports.

- Add the [Uart](https://esphome.io/components/uart/) components

```yaml
captive_portal:
uart:
  - id: chain_uart_right
    tx_pin: GPIO6
    rx_pin: GPIO5
    baud_rate: 115200

  - id: chain_uart_left
    tx_pin: GPIO48
    rx_pin: GPIO47
    baud_rate: 115200
```

### Sensor Components
**Modules used**: Chain Encoder, Chain Angle, Chain ToF, Chain Joystick (X/Y), DualKey battery ADC sensors.

- Add the [Sensor](https://esphome.io/components/external_components/) components
  
In this example, the Master is connected to the following Chain modules:
- Chain Encoder (right side, ID 1)
- Chain Angle (right side, ID 2)
- Chain ToF (right side, ID 3)
- Chain Joystick (left side, ID 1, X/Y axes)

In addition, the built‑in battery related ADC channels (BAT, VBUS, CHARGE) are also enabled as sensors.
```yaml
sensor:
  - platform: m5stack_chain_encoder
    id: chain_encoder_1
    name: "Encoder"
    uart_id: chain_uart_right   
    chain_id: 1                
    update_interval: 100ms

  - platform: m5stack_chain_tof
    id: chain_tof_1
    name: "ToF Distance"
    uart_id: chain_uart_right   
    chain_id: 3                
    update_interval: 100ms

  - platform: m5stack_chain_angle
    id: chain_angle_1
    name: "Angle"
    uart_id: chain_uart_right
    chain_id: 2
    update_interval: 100ms
    
  - platform: m5stack_chain_joystick
    id: chain_joystick_x
    name: "Joystick X"
    uart_id: chain_uart_left
    chain_id: 1
    axis: x
    update_interval: 100ms

  - platform: m5stack_chain_joystick
    name: "Joystick Y"
    uart_id: chain_uart_left
    chain_id: 1
    axis: y
    update_interval: 100ms

  - platform: adc
    pin: GPIO10
    name: "ADC_BAT"
    update_interval: 1s

  - platform: adc
    pin: GPIO2
    name: "ADC_VBUS"
    update_interval: 1s

  - platform: adc
    pin: GPIO9
    name: "ADC_CHARGE"
    update_interval: 1s
```

### Output Components
**Modules used**: RGB LEDs on Chain Encoder, Chain Key, Chain Joystick, Chain Angle, Chain ToF, plus DualKey power control.

- Add the [Ouput](https://esphome.io/components/output/) components
  
```yaml
output:
  - platform: gpio
    id: pwr_en
    pin: GPIO40
  - platform: m5stack_chain_encoder
    id: chain_encoder_rgb_r
    chain_encoder_id: chain_encoder_1
    channel: rgb_red

  - platform: m5stack_chain_encoder
    id: chain_encoder_rgb_g
    chain_encoder_id: chain_encoder_1
    channel: rgb_green

  - platform: m5stack_chain_encoder
    id: chain_encoder_rgb_b
    chain_encoder_id: chain_encoder_1
    channel: rgb_blue

  - platform: m5stack_chain_key
    id: chain_key_rgb_r
    chain_key_id: chain_key_1
    channel: rgb_red

  - platform: m5stack_chain_key
    id: chain_key_rgb_g
    chain_key_id: chain_key_1
    channel: rgb_green

  - platform: m5stack_chain_key
    id: chain_key_rgb_b
    chain_key_id: chain_key_1
    channel: rgb_blue

  - platform: m5stack_chain_joystick
    id: chain_joystick_rgb_r
    chain_joystick_id: chain_joystick_x
    channel: rgb_red

  - platform: m5stack_chain_joystick
    id: chain_joystick_rgb_g
    chain_joystick_id: chain_joystick_x
    channel: rgb_green

  - platform: m5stack_chain_joystick
    id: chain_joystick_rgb_b
    chain_joystick_id: chain_joystick_x
    channel: rgb_blue
  - platform: m5stack_chain_angle
    id: chain_angle_rgb_r
    chain_angle_id: chain_angle_1
    channel: rgb_red

  - platform: m5stack_chain_angle
    id: chain_angle_rgb_g
    chain_angle_id: chain_angle_1
    channel: rgb_green

  - platform: m5stack_chain_angle
    id: chain_angle_rgb_b
    chain_angle_id: chain_angle_1
    channel: rgb_blue
  - platform: m5stack_chain_tof
    id: chain_tof_rgb_r
    m5stack_chain_tof_id: chain_tof_1
    channel: rgb_red

  - platform: m5stack_chain_tof
    id: chain_tof_rgb_g
    m5stack_chain_tof_id: chain_tof_1
    channel: rgb_green

  - platform: m5stack_chain_tof
    id: chain_tof_rgb_b
    m5stack_chain_tof_id: chain_tof_1
    channel: rgb_blue
```

Here the `pwr_en` GPIO output is used to control the power supply for the Chain expansion bus. Usually this output needs to be turned on so that the connected Chain modules can work normally.

### Light Components
**Modules used**: DualKey WS2812 key LEDs and RGB indicator LEDs on each Chain module.

- Add the [Light](https://esphome.io/components/light/) components
  

```yaml
light:
  - platform: esp32_rmt_led_strip
    id: key_light_raw
    internal: true
    pin: GPIO21
    num_leds: 2
    chipset: ws2812
    rgb_order: GRB
    restore_mode: ALWAYS_OFF

  - platform: partition
    name: "Key1 LED"
    id: key_light_1
    segments:
      - id: key_light_raw
        from: 1
        to: 1

  - platform: partition
    name: "Key2 LED"
    id: key_light_2
    segments:
      - id: key_light_raw
        from: 0
        to: 0

  - platform: rgb
    name: "Encoder RGB"
    red: chain_encoder_rgb_r
    green: chain_encoder_rgb_g
    blue: chain_encoder_rgb_b

  - platform: rgb
    name: "Key RGB"
    red: chain_key_rgb_r
    green: chain_key_rgb_g
    blue: chain_key_rgb_b

  - platform: rgb
    name: "Joystick RGB"
    red: chain_joystick_rgb_r
    green: chain_joystick_rgb_g
    blue: chain_joystick_rgb_b

  - platform: rgb
    name: "Angle RGB"
    red: chain_angle_rgb_r
    green: chain_angle_rgb_g
    blue: chain_angle_rgb_b

  - platform: rgb
    name: "ToF RGB"
    red: chain_tof_rgb_r
    green: chain_tof_rgb_g
    blue: chain_tof_rgb_b
```

This section configures the per-key RGB backlight. `key_light_raw` represents the underlying LED strip, while `key_light_1` and `key_light_2` map each LED to the corresponding key so you can control them independently.

### Binary Sensor Components
**Modules used**: DualKey mechanical keys and side switches, plus buttons on Chain Key, Chain Encoder, Chain Joystick.

- Add the [Binary Sensor](https://esphome.io/components/binary_sensor/) components

This part defines all key-related inputs: the two mechanical keys (`KEY 1`, `KEY 2`) with RGB feedback, one Chain Key module on the bus, and the two side switches (`SWITCH 1`, `SWITCH 2`). You can rename these entities in Home Assistant according to your use case.

```yaml
binary_sensor:
  - platform: gpio
    name: "KEY 2"
    pin:
      number: GPIO17
      inverted: true
      mode: INPUT_PULLUP
    filters:
      - delayed_on: 10ms
      - delayed_off: 10ms

    on_press:
      - light.turn_on:
          id: key_light_2
          transition_length: 0ms

    on_release:
      - light.turn_off: key_light_2

  - platform: gpio
    name: "KEY 1"
    pin:
      number: GPIO0
      inverted: true
      mode: INPUT_PULLUP
    filters:
      - delayed_on: 10ms
      - delayed_off: 10ms

    on_press:
      - light.turn_on:
          id: key_light_1
          transition_length: 0ms

    on_release:
      - light.turn_off: key_light_1

  - platform: m5stack_chain_key
    id: chain_key_1
    name: "Key Module Button"
    uart_id: chain_uart_left
    chain_id: 2        
    update_interval: 50ms

  - platform: m5stack_chain_encoder
    name: "Encoder Button"
    chain_encoder_id: chain_encoder_1

  - platform: m5stack_chain_joystick
    name: "Joystick Button"
    chain_joystick_id: chain_joystick_x

  - platform: gpio
    name: "SWITCH 1"
    pin:
      number: GPIO7
      mode: INPUT

  - platform: gpio
    name: "SWITCH 2"
    pin:
      number: GPIO8
      mode: INPUT
```


## Firmware Build

- Click `INSTALL` again to flash and wait for it to complete.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1159/SwitchC6HA5.webp"
width="700px"
/>

- After making changes, click `SAVE` and `INSTALL` in the top-right corner, then choose `Manual Download` in the popup.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1040/timercamera_ha_esp_builder_install_method.webp"
width="400px"
/>

- After the firmware compilation is complete, click Download and select
  `Factory format(Previously Modern)`

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA8.webp"
width="700px"
/>

\#> Tip | Click [m5stack_chain](https://github.com/m5stack/esphome-yaml/blob/main/examples/sensor/m5stack_chain_series.yaml) to view the complete example configuration. The first build may take a while, depending on the performance of the Home Assistant host and network quality.

## Firmware Upload

- Connect the device to your host via a USB Type‑C cable. Open [ESPHome Web](https://web.esphome.io/) and click `CONNECT` to connect to the device.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA12.webp"
width="400px"
/>

- Locate the corresponding serial port number

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/915/HA4.webp"
width="400px"
/>

- Click `INSTALL`

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA9.webp"
width="400px"
/>

- Select the previously compiled firmware to upload.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/HA5.webp"
width="300px"
/>

\#> Tip |
After successfully burning the program, power must be cycled to perform a hardware reset.

## Home Assistant Integration

- Click `Settings` -> `Device & services` to check the device.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/683/HA23.webp"
width="400px"
/>

- We can find the corresponding device in the `Discover` section.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/HA3.webp"
width="400px"
/>


- After adding the device, the data will be displayed correctly.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/HA1.webp"
width="500px"
/>

- Finally, we add these entities to the Dashboard, and the following shows their display results.

<img
src="https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1176/HA7.webp"
width="500px"
/>
