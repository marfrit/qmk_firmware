CC = x86_64-w64-mingw32-gcc
OBJCOPY =
OBJDUMP =
SIZE =
AR =
NM =
HEX =
EEP =
BIN =

FIRMWARE_FORMAT = exe

COMPILEFLAGS += -funsigned-char
COMPILEFLAGS += -fshort-enums
COMPILEFLAGS += -DPLATFORM_WINDOWS
COMPILEFLAGS += -D_WIN32_WINNT=0x0600

CFLAGS += $(COMPILEFLAGS)
CFLAGS += -fno-strict-aliasing
CFLAGS += -Wno-int-to-pointer-cast
CFLAGS += -Wno-pointer-to-int-cast

CXXFLAGS += $(COMPILEFLAGS)
CXXFLAGS += -fno-exceptions

# mingw PE format doesn't resolve __attribute__((weak)) across TUs.
# We provide strong stubs in weak_stubs.c and allow multiple definitions
# so the strong versions win over the mangled .weak.* sections.
LDFLAGS += -Wl,--allow-multiple-definition
LDFLAGS += -mconsole
LDFLAGS += -luser32 -lgdi32 -lkernel32
