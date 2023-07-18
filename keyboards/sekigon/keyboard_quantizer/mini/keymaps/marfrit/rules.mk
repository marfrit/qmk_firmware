SRC += quantizer_mouse.c
SRC += marfrit.c

include keyboards/sekigon/keyboard_quantizer/mini/keymaps/full/dynamic_config/rules.mk
include keyboards/sekigon/keyboard_quantizer/mini/keymaps/full/cli/rules.mk

VPATH += keyboards/sekigon/keyboard_quantizer/mini/keymaps/full/dynamic_config/
VPATH += keyboards/sekigon/keyboard_quantizer/mini/keymaps/full/cli/

KEY_OVERRIDE_ENABLE = yes
COMBO_ENABLE = yes
TAP_DANCE_ENABLE = yes
LEADER_ENABLE = yes
OS_DETECTION_ENABLE = yes

GIT_DESCRIBE := $(shell git describe --tags --long --dirty="\\*" 2>/dev/null)
CFLAGS += -DGIT_DESCRIBE=$(GIT_DESCRIBE)
