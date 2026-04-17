import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID

CODEOWNERS = ["@m5stack"]

ns450b_ns = cg.esphome_ns.namespace("ns450b")
NS450BComponent = ns450b_ns.class_("NS450BComponent", cg.Component)

CONF_MUTE_PIN = "mute_pin"
CONF_MUTE_ACTIVE_LOW = "mute_active_low"
CONF_STARTUP_MUTED = "startup_muted"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(NS450BComponent),
        cv.Required(CONF_MUTE_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_MUTE_ACTIVE_LOW, default=True): cv.boolean,
        cv.Optional(CONF_STARTUP_MUTED, default=False): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    mute_pin = await cg.gpio_pin_expression(config[CONF_MUTE_PIN])
    cg.add(var.set_mute_pin(mute_pin))
    cg.add(var.set_mute_active_low(config[CONF_MUTE_ACTIVE_LOW]))
    cg.add(var.set_startup_muted(config[CONF_STARTUP_MUTED]))
