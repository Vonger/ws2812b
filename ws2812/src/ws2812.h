#ifndef WS2812_H
#define WS2812_H

// reset all leds, turn off all of them.
extern void ws2812_reset();

// set leds color, in 0x0RGB(32bit) order.
extern void ws2812_set(int *color, int size);

#endif // WS2812_H
