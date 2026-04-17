import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import NS450BComponent, ns450b_ns

DEPENDENCIES = ["ns450b"]
CODEOWNERS = ["@m5stack"]

CONF_NS450B_ID = "ns450b_id"

NS450BMuteSwitch = ns450b_ns.class_("NS450BMuteSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = (
    switch.switch_schema(NS450BMuteSwitch)
    .extend(
        {
            cv.GenerateID(CONF_NS450B_ID): cv.use_id(NS450BComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)

    parent = await cg.get_variable(config[CONF_NS450B_ID])
    cg.add(var.set_parent(parent))
