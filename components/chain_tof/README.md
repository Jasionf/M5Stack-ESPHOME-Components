# Chain ToF Component

This ESPHome component provides support for M5Stack Chain ToF (Time-of-Flight) distance sensors. It is based on the M5Chain library and enables communication with multiple ToF sensors connected in a chain configuration via UART.

## Features

- **Chain Communication**: Support for multiple ToF sensors connected in a daisy-chain configuration
- **Configurable Measurement**: Adjustable measurement time (20-200 ms) for accuracy vs speed trade-offs
- **Multiple Modes**: Support for continuous, single, and stopped measurement modes
- **ESPHome Integration**: Full integration with ESPHome's sensor framework
- **Auto-recovery**: Automatic recovery from communication timeouts

## Hardware Requirements

- ESP32-based device
- M5Stack Chain ToF sensors
- UART connection (TX/RX pins)

## Configuration

### Basic Setup

```yaml
# UART configuration
uart:
  id: chain_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

# Chain ToF component
chain_tof:
  id: tof_bus
  uart_id: chain_uart
  device_id: 1  # Optional, default is 1

# Individual sensors
sensor:
  - platform: chain_tof
    name: "Distance Sensor 1"
    chain_tof_id: tof_bus
    sensor_id: 1
    measure_time: 50  # Optional, 20-200ms, default 50
    update_interval: 1s
```

### Parameters

#### Chain ToF Component (`chain_tof`)

- **`id`** (Required): Component ID for referencing in sensors
- **`uart_id`** (Required): UART component ID to use for communication
- **`device_id`** (Optional): Device identifier for the chain bus (default: 1)

#### Sensor (`sensor` with `platform: chain_tof`)

- **`chain_tof_id`** (Required): Reference to the Chain ToF component
- **`sensor_id`** (Required): Sensor position in the chain (1-255)
- **`measure_time`** (Optional): Measurement time in milliseconds (20-200, default: 50)
- **`update_interval`** (Optional): How often to read the sensor (default: 1s)

## Protocol Details

The component implements the M5Chain communication protocol:

- **Packet Format**: Header (0xAA55) + Length + ID + Command + Data + CRC + Footer (0x55AA)
- **Commands Supported**:
  - Get Distance (0x50)
  - Set/Get Measurement Time (0x51/0x52)
  - Set/Get Measurement Mode (0x53/0x54)
  - Set/Get Measurement Status (0x55/0x56)
  - Get Measurement Complete Flag (0x57)

## Measurement Modes

- **Continuous**: Sensor continuously measures distance
- **Single**: Take one measurement then stop
- **Stop**: Disable measurements

The component automatically configures sensors for continuous mode and handles measurement initialization.

## Error Handling

The component includes robust error handling:

- **Timeout Recovery**: Automatically restarts measurements on communication timeouts
- **CRC Validation**: Verifies packet integrity
- **Mutex Protection**: Thread-safe access to UART communication
- **Status Reporting**: Detailed logging for troubleshooting

## Example Configurations

### Single Sensor
```yaml
sensor:
  - platform: chain_tof
    name: "Front Distance"
    chain_tof_id: tof_bus
    sensor_id: 1
    update_interval: 500ms
```

### Multiple Sensors with Different Settings
```yaml
sensor:
  - platform: chain_tof
    name: "Fast Sensor"
    chain_tof_id: tof_bus
    sensor_id: 1
    measure_time: 20  # Fast but less accurate
    update_interval: 100ms
    
  - platform: chain_tof
    name: "Accurate Sensor"
    chain_tof_id: tof_bus
    sensor_id: 2
    measure_time: 200  # Slower but more accurate
    update_interval: 2s
```

## Troubleshooting

### Common Issues

1. **No sensor readings**: Check UART wiring and baud rate
2. **Timeout errors**: Verify sensor power and chain connections
3. **Inconsistent readings**: Try increasing `measure_time` or `update_interval`

### Debug Logging

Enable debug logging to troubleshoot communication issues:

```yaml
logger:
  level: DEBUG
  logs:
    chain_tof: DEBUG
```

## Technical Notes

- Based on M5Chain ToF sensor library
- Supports up to 255 sensors per chain
- Measurement range depends on specific ToF sensor hardware
- All distances are reported in millimeters
- Component automatically handles sensor initialization and mode configuration