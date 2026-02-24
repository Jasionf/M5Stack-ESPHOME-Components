import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_DISTANCE,
    STATE_CLASS_MEASUREMENT,
    UNIT_MILLIMETER,
    ICON_RULER,
)
from . import ChainToFComponent, chain_tof_ns

CODEOWNERS = ["@jason"]
DEPENDENCIES = ["chain_tof"]

# Configuration constants
CONF_CHAIN_TOF_ID = "chain_tof_id"
CONF_SENSOR_ID = "sensor_id"
CONF_MEASURE_TIME = "measure_time"

ChainToFSensor = chain_tof_ns.class_("ChainToFSensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        ChainToFSensor,
        unit_of_measurement=UNIT_MILLIMETER,
        icon=ICON_RULER,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_DISTANCE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_CHAIN_TOF_ID): cv.use_id(ChainToFComponent),
            cv.Optional(CONF_SENSOR_ID, default=1): cv.int_range(min=1, max=255),
            cv.Optional(CONF_MEASURE_TIME, default=50): cv.int_range(min=20, max=200),
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_CHAIN_TOF_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_sensor_id(config[CONF_SENSOR_ID]))

    if CONF_MEASURE_TIME in config:
        cg.add(var.set_measure_time(config[CONF_MEASURE_TIME]))

    cg.add(parent.register_sensor(var))