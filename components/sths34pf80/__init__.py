import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    UNIT_CELSIUS,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
)

DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@m5stack"]

sths34pf80_ns = cg.esphome_ns.namespace("sths34pf80")
STHS34PF80Component = sths34pf80_ns.class_(
    "STHS34PF80Component", cg.PollingComponent, i2c.I2CDevice
)

CONF_PRESENCE = "presence"
CONF_MOTION = "motion"
CONF_TEMPERATURE = "temperature"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(STHS34PF80Component),
            cv.Optional(CONF_PRESENCE): sensor.sensor_schema(
                icon="mdi:account",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MOTION): sensor.sensor_schema(
                icon="mdi:run",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x5A))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if presence_conf := config.get(CONF_PRESENCE):
        presence = await sensor.new_sensor(presence_conf)
        cg.add(var.set_presence_sensor(presence))

    if motion_conf := config.get(CONF_MOTION):
        motion = await sensor.new_sensor(motion_conf)
        cg.add(var.set_motion_sensor(motion))

    if temp_conf := config.get(CONF_TEMPERATURE):
        temp = await sensor.new_sensor(temp_conf)
        cg.add(var.set_temperature_sensor(temp))
