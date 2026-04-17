import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import audio_dac
from esphome import pins
from esphome.const import CONF_ID, CONF_PIN

CODEOWNERS = ["@your-github-username"]
AUTO_LOAD = ["audio_dac"]

ns4150b_ns = cg.esphome_ns.namespace("ns4150b")
NS4150B = ns4150b_ns.class_("NS4150B", audio_dac.AudioDac, cg.Component)

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
