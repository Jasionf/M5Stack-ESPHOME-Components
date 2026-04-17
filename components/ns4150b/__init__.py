import esphome.codegen as cg
from esphome.components import audio_dac

CODEOWNERS = ["@your-github-username"]

ns4150b_ns = cg.esphome_ns.namespace("ns4150b")
NS4150B = ns4150b_ns.class_("NS4150B", audio_dac.AudioDac, cg.Component)
