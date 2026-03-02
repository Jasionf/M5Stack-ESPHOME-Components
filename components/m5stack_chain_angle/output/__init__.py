import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID

from .. import m5stack_chain_angle_ns, ChainAngleSensor

CODEOWNERS = ["@Jasionf"]
DEPENDENCIES = ["m5stack_chain_angle"]

ChainAngleLedOutput = m5stack_chain_angle_ns.class_(
    "ChainAngleLedOutput",
    output.FloatOutput,
)

CONF_CHAIN_ANGLE_ID = "chain_angle_id"

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(ChainAngleLedOutput),
        cv.GenerateID(CONF_CHAIN_ANGLE_ID): cv.use_id(ChainAngleSensor),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)

    parent = await cg.get_variable(config[CONF_CHAIN_ANGLE_ID])
    cg.add(var.set_parent(parent))
