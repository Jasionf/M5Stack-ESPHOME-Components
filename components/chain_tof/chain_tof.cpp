#include "chain_tof.h"
#include "esphome/core/log.h"

namespace esphome {
namespace chain_tof {

static const char *const TAG = "chain_tof";

void ChainToFComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Chain ToF...");
  
  // Initialize buffers
  this->receive_buffer_.reserve(RECEIVE_BUFFER_SIZE);
  this->send_buffer_.reserve(SEND_BUFFER_SIZE);
  this->cmd_buffer_.reserve(CMD_BUFFER_SIZE);
  this->return_packet_.reserve(CMD_BUFFER_SIZE);
  
  // Clear buffers
  this->receive_buffer_.clear();
  this->send_buffer_.clear();
  this->cmd_buffer_.clear();
  this->return_packet_.clear();
  
  ESP_LOGCONFIG(TAG, "Chain ToF setup complete");
}

void ChainToFComponent::loop() {
  // Read incoming data
  this->read_buffer_();
  
  // Process any incoming packets
  this->process_buffer_data_();
  
  // Update sensors periodically
  uint32_t now = millis();
  if (now - this->last_read_time_ > 500) { // Update every 500ms
    this->last_read_time_ = now;
    
    for (auto *sensor : this->sensors_) {
      // Trigger sensor updates if needed
      // This will be handled by the sensor component itself
    }
  }
}

void ChainToFComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Chain ToF:");
  ESP_LOGCONFIG(TAG, "  Device ID: %d", this->device_id_);
  this->check_uart_settings(115200, 1, uart::UART_CONFIG_PARITY_NONE, 8);
}

bool ChainToFComponent::acquire_mutex_() {
  uint32_t start = millis();
  while (true) {
    if (!this->mutex_locked_) {
      this->mutex_locked_ = true;
      return true;
    }
    if (millis() - start >= 1000) {
      return false;
    }
    delay(1);
  }
}

void ChainToFComponent::release_mutex_() {
  this->mutex_locked_ = false;
}

uint8_t ChainToFComponent::calculate_crc_(const uint8_t *buffer, uint16_t size) {
  uint8_t crc8 = 0;
  for (uint16_t i = 4; i < (size - 3); i++) {
    crc8 += buffer[i];
  }
  return crc8;
}

bool ChainToFComponent::check_packet_(const uint8_t *buffer, uint16_t size) {
  if (size < PACK_SIZE_MIN) {
    return false;
  }

  if (buffer[0] != PACK_HEAD_HIGH || buffer[1] != PACK_HEAD_LOW || 
      buffer[size - 1] != PACK_END_LOW || buffer[size - 2] != PACK_END_HIGH) {
    return false;
  }

  uint16_t length = ((uint16_t)buffer[3] << 8) | buffer[2];
  length += 6;
  if (length != size) {
    return false;
  }

  uint8_t crc8 = this->calculate_crc_(buffer, size);
  if (crc8 != buffer[size - 3]) {
    return false;
  }

  return true;
}

void ChainToFComponent::send_packet_(uint16_t id, uint8_t cmd, const uint8_t *buffer, uint16_t size) {
  uint16_t cmd_size = 3 + size;  // id + cmd + data + crc8
  uint16_t send_buffer_size = size + 9;

  this->send_buffer_.clear();
  this->send_buffer_.resize(send_buffer_size);

  this->send_buffer_[0] = PACK_HEAD_HIGH;
  this->send_buffer_[1] = PACK_HEAD_LOW;
  this->send_buffer_[2] = cmd_size & 0xFF;
  this->send_buffer_[3] = (cmd_size >> 8) & 0xFF;
  this->send_buffer_[4] = id;
  this->send_buffer_[5] = cmd;

  if (size > 0) {
    memcpy(&this->send_buffer_[6], buffer, size);
  }

  uint8_t crc8 = this->calculate_crc_(this->send_buffer_.data(), send_buffer_size);
  this->send_buffer_[send_buffer_size - 3] = crc8;
  this->send_buffer_[send_buffer_size - 2] = PACK_END_HIGH;
  this->send_buffer_[send_buffer_size - 1] = PACK_END_LOW;

  this->write_array(this->send_buffer_.data(), send_buffer_size);
}

void ChainToFComponent::read_buffer_() {
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    
    if (this->receive_buffer_.size() < RECEIVE_BUFFER_SIZE) {
      this->receive_buffer_.push_back(byte);
    } else {
      // Buffer full, remove oldest byte
      this->receive_buffer_.erase(this->receive_buffer_.begin());
      this->receive_buffer_.push_back(byte);
    }
  }
}

bool ChainToFComponent::process_buffer_data_(uint16_t id, uint8_t cmd) {
  if (this->receive_buffer_.size() < PACK_SIZE_MIN) {
    return false;
  }

  uint16_t start_index = 0;
  bool packet_found = false;

  // Continue scanning while at least minimum packet size bytes remain
  while ((this->receive_buffer_.size() - start_index) >= PACK_SIZE_MIN) {
    // Check for packet header 0xAA 0x55
    if (this->receive_buffer_[start_index] == 0xAA && 
        this->receive_buffer_[start_index + 1] == 0x55) {
      
      // Read 2-byte packet length (little endian)
      uint16_t length = (uint16_t)this->receive_buffer_[start_index + 2] | 
                       ((uint16_t)this->receive_buffer_[start_index + 3] << 8);

      // Calculate total packet size
      uint16_t packet_size = 4 + length + 2;

      // Check if full packet data is available in buffer
      if ((this->receive_buffer_.size() - start_index) < packet_size) {
        break; // Incomplete packet
      }

      // Verify packet footer bytes
      if (this->receive_buffer_[start_index + packet_size - 2] != 0x55 ||
          this->receive_buffer_[start_index + packet_size - 1] != 0xAA) {
        start_index++; // Invalid footer, skip this byte
        continue;
      }

      uint8_t *packet_data = &this->receive_buffer_[start_index];
      uint8_t packet_id = packet_data[4];
      uint8_t packet_cmd = packet_data[5];

      // Verify packet CRC checksum
      if (!this->check_packet_(packet_data, packet_size)) {
        start_index++; // CRC error, skip byte
        continue;
      }

      // Check for matching device ID and command
      if ((id == 0 || packet_id == id) && (cmd == 0 || packet_cmd == cmd)) {
        this->return_packet_.clear();
        this->return_packet_.resize(packet_size);
        memcpy(this->return_packet_.data(), packet_data, packet_size);
        this->return_packet_size_ = packet_size;
        packet_found = true;
      }

      // Remove processed packet from buffer
      this->receive_buffer_.erase(this->receive_buffer_.begin() + start_index,
                                  this->receive_buffer_.begin() + start_index + packet_size);
      
      if (packet_found) {
        break;
      }
    } else {
      start_index++;
    }
  }

  return packet_found;
}

bool ChainToFComponent::wait_for_data_(uint16_t id, uint8_t cmd, uint32_t timeout) {
  uint32_t start_time = millis();
  
  while (millis() - start_time < timeout) {
    this->read_buffer_();
    if (this->process_buffer_data_(id, cmd)) {
      return true;
    }
    delay(1);
  }
  
  return false;
}

ChainStatus ChainToFComponent::get_tof_distance(uint8_t id, uint16_t *distance, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->send_packet_(id, CHAIN_TOF_GET_DISTANCE, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_GET_DISTANCE, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *distance = (this->return_packet_[7] << 8) | this->return_packet_[6];
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::set_tof_measure_time(uint8_t id, uint8_t time, uint8_t *operation_status, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (time < TOF_MEASUREMENT_TIME_MIN || time > TOF_MEASUREMENT_TIME_MAX) {
    return CHAIN_PARAMETER_ERROR;
  }

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->cmd_buffer_.push_back(time);
    this->send_packet_(id, CHAIN_TOF_SET_MEASURE_TIME, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_SET_MEASURE_TIME, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *operation_status = this->return_packet_[6];
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::get_tof_measure_time(uint8_t id, uint8_t *time, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->send_packet_(id, CHAIN_TOF_GET_MEASURE_TIME, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_GET_MEASURE_TIME, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *time = this->return_packet_[6];
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::set_tof_measure_mode(uint8_t id, ChainToFMode mode, uint8_t *operation_status, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (mode < CHAIN_TOF_MODE_STOP || mode > CHAIN_TOF_MODE_CONTINUOUS) {
    return CHAIN_PARAMETER_ERROR;
  }

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->cmd_buffer_.push_back(static_cast<uint8_t>(mode));
    this->send_packet_(id, CHAIN_TOF_SET_MEASURE_MODE, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_SET_MEASURE_MODE, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *operation_status = this->return_packet_[6];
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::get_tof_measure_mode(uint8_t id, ChainToFMode *mode, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->send_packet_(id, CHAIN_TOF_GET_MEASURE_MODE, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_GET_MEASURE_MODE, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *mode = static_cast<ChainToFMode>(this->return_packet_[6]);
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::set_tof_measure_status(uint8_t id, ChainToFMeasureStatus measure_status, uint8_t *operation_status, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (measure_status < CHAIN_TOF_STATUS_STOP || measure_status > CHAIN_TOF_STATUS_START) {
    return CHAIN_PARAMETER_ERROR;
  }

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->cmd_buffer_.push_back(static_cast<uint8_t>(measure_status));
    this->send_packet_(id, CHAIN_TOF_SET_MEASURE_STATUS, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_SET_MEASURE_STATUS, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *operation_status = this->return_packet_[6];
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::get_tof_measure_status(uint8_t id, ChainToFMeasureStatus *measure_status, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->send_packet_(id, CHAIN_TOF_GET_MEASURE_STATUS, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_GET_MEASURE_STATUS, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *measure_status = static_cast<ChainToFMeasureStatus>(this->return_packet_[6]);
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

ChainStatus ChainToFComponent::get_tof_measure_complete_flag(uint8_t id, uint8_t *complete_flag, uint32_t timeout) {
  ChainStatus status = CHAIN_OK;

  if (this->acquire_mutex_()) {
    this->cmd_buffer_.clear();
    this->send_packet_(id, CHAIN_TOF_GET_MEASURE_COMPLETE_FLAG, this->cmd_buffer_.data(), this->cmd_buffer_.size());
    
    if (this->wait_for_data_(id, CHAIN_TOF_GET_MEASURE_COMPLETE_FLAG, timeout)) {
      if (this->check_packet_(this->return_packet_.data(), this->return_packet_size_)) {
        *complete_flag = this->return_packet_[6];
      } else {
        status = CHAIN_RETURN_PACKET_ERROR;
      }
    } else {
      status = CHAIN_TIMEOUT;
    }
    this->release_mutex_();
  } else {
    status = CHAIN_BUSY;
  }

  return status;
}

// ChainToFSensor implementation
void ChainToFSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Chain ToF Sensor %d...", this->sensor_id_);
  
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "Parent component not set for sensor %d", this->sensor_id_);
    this->mark_failed();
    return;
  }
  
  // Initialize sensor measurement settings
  uint8_t operation_status;
  ChainStatus status = this->parent_->set_tof_measure_time(this->sensor_id_, this->measure_time_, &operation_status);
  if (status == CHAIN_OK && operation_status == 1) {
    ESP_LOGD(TAG, "Measurement time set to %d ms for sensor %d", this->measure_time_, this->sensor_id_);
  } else {
    ESP_LOGW(TAG, "Failed to set measurement time for sensor %d", this->sensor_id_);
  }
  
  // Set to continuous measurement mode
  status = this->parent_->set_tof_measure_mode(this->sensor_id_, CHAIN_TOF_MODE_CONTINUOUS, &operation_status);
  if (status == CHAIN_OK && operation_status == 1) {
    ESP_LOGD(TAG, "Set sensor %d to continuous mode", this->sensor_id_);
  } else {
    ESP_LOGW(TAG, "Failed to set continuous mode for sensor %d", this->sensor_id_);
  }
  
  // Start measurement
  status = this->parent_->set_tof_measure_status(this->sensor_id_, CHAIN_TOF_STATUS_START, &operation_status);
  if (status == CHAIN_OK && operation_status == 1) {
    ESP_LOGD(TAG, "Started measurement for sensor %d", this->sensor_id_);
    this->initialized_ = true;
  } else {
    ESP_LOGW(TAG, "Failed to start measurement for sensor %d", this->sensor_id_);
  }
}

void ChainToFSensor::update() {
  if (!this->initialized_) {
    ESP_LOGW(TAG, "Sensor %d not initialized, skipping update", this->sensor_id_);
    return;
  }
  
  uint16_t distance;
  ChainStatus status = this->parent_->get_tof_distance(this->sensor_id_, &distance);
  
  if (status == CHAIN_OK) {
    ESP_LOGV(TAG, "Sensor %d distance: %d mm", this->sensor_id_, distance);
    this->publish_state(distance);
  } else {
    ESP_LOGW(TAG, "Failed to read distance from sensor %d (status: %d)", this->sensor_id_, status);
    if (status == CHAIN_TIMEOUT) {
      // Try to restart measurement on timeout
      uint8_t operation_status;
      this->parent_->set_tof_measure_status(this->sensor_id_, CHAIN_TOF_STATUS_START, &operation_status);
    }
  }
}

void ChainToFSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Chain ToF Sensor:");
  ESP_LOGCONFIG(TAG, "  Sensor ID: %d", this->sensor_id_);
  ESP_LOGCONFIG(TAG, "  Measure Time: %d ms", this->measure_time_);
  ESP_LOGCONFIG(TAG, "  Update Interval: %.1fs", this->get_update_interval() / 1000.0f);
  LOG_SENSOR("  ", "Distance", this);
}

} // namespace chain_tof
} // namespace esphome
