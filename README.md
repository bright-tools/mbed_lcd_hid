# mbed_lcd_hid

## Overview

Display notifications received via USB HID on an mbed connected LCD

The mbed board acts as a HID meaning that no special USB drivers are required.  The host PC is then able to send messages to the mbed board in order to display text on the LCD.

Currently implemented features:
* Scrolling of messages too long to fit on the display
* Pause before scrolling to allow start of message to be read
* Pulsing of LCD for new messages, cancelled via a swipe of the position sensor
* Configurable LCD contrast (via the "up" and "down" buttons on the LCD shield)

## Kit

I'm using:
* [NXP FRDM-KL46Z](https://developer.mbed.org/platforms/FRDM-KL46Z/)
* [Robot LCD Shielf](https://www.dfrobot.com/product-51.html)
