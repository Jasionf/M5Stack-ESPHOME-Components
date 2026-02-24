#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"
#include <vector>

namespace esphome {
namespace chain_tof {

// Protocol constants
static const uint8_t PACK_HEAD_HIGH = 0xAA;
static const uint8_t PACK_HEAD_LOW = 0x55;
static const uint8_t PACK_END_HIGH = 0x55;
static const uint8_t PACK_END_LOW = 0xAA;
static const uint16_t PACK_SIZE_MIN = 9;
static const uint16_t RECEIVE_BUFFER_SIZE = 1024;
static const uint16_t SEND_BUFFER_SIZE = 256;
static const uint16_t CMD_BUFFER_SIZE = 256;

// Status codes
enum ChainStatus : uint8_t {
  CHAIN_OK = 0x00,
  CHAIN_PARAMETER_ERROR = 0x01,
  CHAIN_RETURN_PACKET_ERROR = 0x02,
  CHAIN_BUSY = 0x04,
  CHAIN_TIMEOUT = 0x05
};

// ToF specific commands
enum ChainToFCmd : uint8_t {
  CHAIN_TOF_GET_DISTANCE = 0x50,
  CHAIN_TOF_SET_MEASURE_TIME = 0x51,
  CHAIN_TOF_GET_MEASURE_TIME = 0x52,
  CHAIN_TOF_SET_MEASURE_MODE = 0x53,
  CHAIN_TOF_GET_MEASURE_MODE = 0x54,
  CHAIN_TOF_SET_MEASURE_STATUS = 0x55,
  CHAIN_TOF_GET_MEASURE_STATUS = 0x56,
  CHAIN_TOF_GET_MEASURE_COMPLETE_FLAG = 0x57
};

// Measurement modes
enum ChainToFMode : uint8_t {
  CHAIN_TOF_MODE_STOP = 0,
  CHAIN_TOF_MODE_SINGLE = 1,
  CHAIN_TOF_MODE_CONTINUOUS = 2
};

// Measurement status
enum ChainToFMeasureStatus : uint8_t {
  CHAIN_TOF_STATUS_STOP = 0,
  CHAIN_TOF_STATUS_START = 1
};

// Time constants
static const uint8_t TOF_MEASUREMENT_TIME_MIN = 20;
static const uint8_t TOF_MEASUREMENT_TIME_MAX = 200;

class ChainToFSensor; // Forward declaration

class ChainToFComponent : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  
  void set_device_id(uint8_t device_id) { this->device_id_ = device_id; }
  
  // ToF operations
  ChainStatus get_tof_distance(uint8_t id, uint16_t *distance, uint32_t timeout = 100);
  ChainStatus set_tof_measure_time(uint8_t id, uint8_t time, uint8_t *operation_status, uint32_t timeout = 100);
  ChainStatus get_tof_measure_time(uint8_t id, uint8_t *time, uint32_t timeout = 100);
  ChainStatus set_tof_measure_mode(uint8_t id, ChainToFMode mode, uint8_t *operation_status, uint32_t timeout = 100);
  ChainStatus get_tof_measure_mode(uint8_t id, ChainToFMode *mode, uint32_t timeout = 100);
  ChainStatus set_tof_measure_status(uint8_t id, ChainToFMeasureStatus status, uint8_t *operation_status, uint32_t timeout = 100);
  ChainStatus get_tof_measure_status(uint8_t id, ChainToFMeasureStatus *status, uint32_t timeout = 100);
  ChainStatus get_tof_measure_complete_flag(uint8_t id, uint8_t *complete_flag, uint32_t timeout = 100);

  // Register sensor
  void register_sensor(ChainToFSensor *sensor) { this->sensors_.push_back(sensor); }

 protected:
  bool acquire_mutex_();
  void release_mutex_();
  void send_packet_(uint16_t id, uint8_t cmd, const uint8_t *buffer, uint16_t size);
  bool check_packet_(const uint8_t *buffer, uint16_t size);
  uint8_t calculate_crc_(const uint8_t *buffer, uint16_t size);
  bool wait_for_data_(uint16_t id, uint8_t cmd, uint32_t timeout);
  void read_buffer_();
  bool process_buffer_data_(uint16_t id = 0, uint8_t cmd = 0);

 private:
  uint8_t device_id_{1};
  bool mutex_locked_{false};
  
  std::vector<uint8_t> receive_buffer_;
  std::vector<uint8_t> send_buffer_;
  std::vector<uint8_t> cmd_buffer_;
  std::vector<uint8_t> return_packet_;
  uint16_t return_packet_size_{0};
  
  std::vector<ChainToFSensor *> sensors_;
  uint32_t last_read_time_{0};
};

class ChainToFSensor : public sensor::Sensor, public PollingComponent {
 public:
  ChainToFSensor() = default;
  
  void setup() override;
  void update() override;
  void dump_config() override;
  
  void set_parent(ChainToFComponent *parent) { this->parent_ = parent; }
  void set_sensor_id(uint8_t sensor_id) { this->sensor_id_ = sensor_id; }
  void set_measure_time(uint8_t measure_time) { this->measure_time_ = measure_time; }

 protected:
  ChainToFComponent *parent_;
  uint8_t sensor_id_{1};
  uint8_t measure_time_{50};
  bool initialized_{false};
};

} // namespace chain_tof
} // namespace esphome
