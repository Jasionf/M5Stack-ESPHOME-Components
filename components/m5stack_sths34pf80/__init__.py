import esphome.codegen as cg
from esphome.components import i2c

DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@m5stack"]

m5stack_sths34pf80_ns = cg.esphome_ns.namespace("m5stack_sths34pf80")
STHS34PF80Component = m5stack_sths34pf80_ns.class_(
    "STHS34PF80Component", cg.PollingComponent, i2c.I2CDevice
)
