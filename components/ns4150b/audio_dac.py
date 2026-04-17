import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID, CONF_PIN

AUTO_LOAD = ["audio_dac"]

from . import ns4150b_ns, NS4150B  # noqa: E402

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(NS4150B),
            cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    enable_pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_enable_pin(enable_pin))
