# pio_ps2_converter

RP2040 based PS/2 Converter.

Make example for this keyboard (after setting up your build environment):

    make converter/pio_ps2_converter:default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Hardware

There is a dedicated PCB designed to be integrated into a plug sized like an USB stick, and an SDL cable on the other side. You can build your own converter though using a Raspberry Pi Pico development board and an I²C capable level shifter, since the GPIOs of the RP2040 are not 5V tolerant and the PS/2 protocol specifies 5V as signalling voltage.

The converter is setup to deal with PS/2 standard-compliant devices having one start bit, one parity bit and one stop bit. Communication with non-compliant devices will likely fail but can be achieved.

## Software

This converter supports combined keyboard/ mouse devices like the [IBM M4](https://sharktastica.co.uk/wiki?id=modelm4) and the [IBM M13](https://sharktastica.co.uk/wiki?id=modelm13).
