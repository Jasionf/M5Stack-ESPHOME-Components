import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID, DEVICE_CLASS_SWITCH

from .. import ns4150b_ns, NS4150B

CONF_NS4150B_ID = "ns4150b_id"
DEPENDENCIES = ["ns4150b"]

NS4150BEnableSwitch = ns4150b_ns.class_("NS4150BEnableSwitch", switch.Switch)

CONFIG_SCHEMA = switch.switch_schema(
    NS4150BEnableSwitch,
    device_class=DEVICE_CLASS_SWITCH,
).extend(
    cv.Schema(
        {
            cv.GenerateID(CONF_NS4150B_ID): cv.use_id(NS4150B),
        }
    )
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_NS4150B_ID])
    s = await switch.new_switch(config)
    await cg.register_parented(s, config[CONF_NS4150B_ID])
    cg.add(parent.set_enable_switch(s))
