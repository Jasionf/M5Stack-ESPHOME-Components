import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor

from . import ChainAngleSensor

DEPENDENCIES = ["uart"]

# 使用 chain_id 而不是 device_id，避免与 ESPHome 内部 device_id 字段冲突
CONF_CHAIN_ID = "chain_id"


CONFIG_SCHEMA = (
    sensor.sensor_schema(
        ChainAngleSensor,
        icon="mdi:rotate-3d-variant",
    )
    .extend(
        {
            cv.Optional(CONF_CHAIN_ID, default=1): cv.int_range(min=1, max=255),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("200ms"))
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_device_id(config[CONF_CHAIN_ID]))
