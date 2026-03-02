import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID

from .. import m5stack_chain_joystick_ns, ChainJoystickSensor

CODEOWNERS = ["@Jasionf"]
DEPENDENCIES = ["m5stack_chain_joystick"]

ChainJoystickLedOutput = m5stack_chain_joystick_ns.class_(
    "ChainJoystickLedOutput",
    output.FloatOutput,
)

CONF_CHAIN_JOYSTICK_ID = "chain_joystick_id"

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(ChainJoystickLedOutput),
        cv.GenerateID(CONF_CHAIN_JOYSTICK_ID): cv.use_id(ChainJoystickSensor),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)

    parent = await cg.get_variable(config[CONF_CHAIN_JOYSTICK_ID])
    cg.add(var.set_parent(parent))
