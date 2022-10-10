# f401 blackpill modelm 

![IBM Model M Controller](https://dynamic.reauktion.de/flugphase/wp-content/uploads/2021/11/IMG_20211030_204552-scaled.jpg)

Supported Hardware: *STM32F401CCU6 WeAct v1.3*.

Supported boards / matrices:

* IBM 1392595 (102 key terminal board)
* IBM 1390587 (122 key terminal board)
* IBM 1391403 (102 key ISO board)

Other boards will need additional keymaps and matrix-to-keycode mappings in modelmstm.h.

To compile the M122 version, do

``` make handwired/modelmstm/terminal122:marfrit_terminal122 ```

And upload the bin to your controller.

Do not use pins
* B2 (pull down for reliably getting to dfu-mode via keycode), 
* A9, 
* A11, and 
* A12 (USB port is connected to these pins). 

Use 
* C13, 
* C14, and 
* C15 as input only.

As for parts, you will need the appropriate Triomate connectors to connect to the matrix sheets, a few wires, a 4.7kOhm pull down resistor (B2), a 22kOhm pull up resistor (A10) and a perma proto board/ bread board.

For an example with LEDs, check the configuration of the iso102 board.
