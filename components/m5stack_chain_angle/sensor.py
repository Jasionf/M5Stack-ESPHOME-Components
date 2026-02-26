import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import CONF_ID

from . import m5stack_chain_angle_ns, ChainAngleSensor

DEPENDENCIES = ["uart"]

CONF_DEVICE_ID = "device_id"


CONFIG_SCHEMA = (
    sensor.sensor_schema(
        icon="mdi:rotate-3d-variant",
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(ChainAngleSensor),
            cv.Optional(CONF_DEVICE_ID, default=1): cv.int_range(min=1, max=255),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("200ms"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_device_id(config[CONF_DEVICE_ID]))
