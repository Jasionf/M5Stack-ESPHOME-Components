import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@jason"]
DEPENDENCIES = ["uart"]

chain_tof_ns = cg.esphome_ns.namespace("chain_tof")
ChainToFComponent = chain_tof_ns.class_("ChainToFComponent", cg.Component, uart.UARTDevice)

# Configuration constants
CONF_DEVICE_ID = "device_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ChainToFComponent),
            cv.Optional(CONF_DEVICE_ID, default=1): cv.int_range(min=1, max=255),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    
    cg.add(var.set_device_id(config[CONF_DEVICE_ID]))
