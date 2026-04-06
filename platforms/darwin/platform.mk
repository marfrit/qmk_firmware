# macOS: compile with clang (system default on Mac)
# When cross-compiling, use osxcross or compile natively on Mac
CC = cc
OBJCOPY =
OBJDUMP =
SIZE =
AR =
NM =
HEX =
EEP =
BIN =

COMPILEFLAGS += -funsigned-char
COMPILEFLAGS += -ffunction-sections
COMPILEFLAGS += -fdata-sections
COMPILEFLAGS += -fshort-enums
COMPILEFLAGS += -DPLATFORM_DARWIN

CFLAGS += $(COMPILEFLAGS)
CFLAGS += -fno-strict-aliasing

CXXFLAGS += $(COMPILEFLAGS)
CXXFLAGS += -fno-exceptions

# macOS frameworks for CGEventTap and IOKit
LDFLAGS += -framework CoreGraphics
LDFLAGS += -framework Carbon
LDFLAGS += -framework CoreFoundation
LDFLAGS += -framework IOKit
