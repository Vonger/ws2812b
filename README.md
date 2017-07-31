# ws2812b
VoCore2 WS2812B driver, support upto 1024 LEDs, CPU usage less than 1%

More information please check http://vonger.cn/?p=14302

driver/misc/ws2812b.c: this file is Linux kernel driver, based on VoCore2 GDMA driver.

ws2812/ws2812.c: this file is provide two simple API to control WS2812 LED, depends on ws2812b.c, the kernel driver.

This is a quick code, so it is not the optimized to the best. Currently it supports upto 1024 WS2812 at 24 fps, but CPU usage is almost zero, that is the magic dma :)

Later I will update the code to make it based on GDMA driver.
