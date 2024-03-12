# converter/cfw

*A short description of the keyboard/project*

* Keyboard Maintainer: [Markus Fritsche](https://github.com/marfrit)
* Hardware Supported: *Raspberry Pi Pico*

Make example for this keyboard (after setting up your build environment):

    make converter/cfw:default

Flashing example for this keyboard:

    make converter/cfw:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 2 ways:

* **Physical reset button**: Briefly press the button on the front of the PCB.
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
