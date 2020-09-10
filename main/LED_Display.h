#ifndef _LED_DISPLAY_H_
#define _LED_DISPLAY_H_

#include "FastLED.h"
#include "Task.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define NUM_LEDS 25
#define NOT_TRANSIENT 255

#define RED 0x00F000
#define GREEN 0xF00000
#define BLUE 0x0000F0
#define WHITE 0xF0F0F0


class LED_Display : public Task
{
private:
    CRGB _ledbuff[NUM_LEDS];
    uint8_t _ledtransient[NUM_LEDS];
    uint8_t _numberled;

    uint8_t _mode = 0;
    uint16_t _am_speed;
    uint8_t _am_mode;
    int32_t _count_x, _count_y;
    int32_t _am_count = -1;
    uint8_t *_am_buffptr;

    SemaphoreHandle_t _xSemaphore = NULL;

public:
    enum
    {
        kStatic = 0,
        kAnmiation_run,
        kAnmiation_stop,
    } Dismode;
    enum
    {
        kMoveRight = 0x01,
        kMoveLeft = 0x02,
        kMoveTop = 0x04,
        kMoveButtom = 0x08,
    } Am_mode;

    /* data */
public:
    LED_Display(uint8_t LEDNumbre = 25);
    ~LED_Display();

    void run();

    void animation(uint8_t *buffptr, uint8_t amspeed, uint8_t ammode, int64_t amcount = -1);
    void displaybuff(uint8_t *buffptr, int8_t offsetx = 0, int8_t offsety = 0);
    void MoveDisPlayBuff(int8_t offsetx = 0, int8_t offsety = 0);

    void setBrightness(uint8_t brightness);
    void drawpixA(uint8_t xpos, uint8_t ypos, CRGB Color, uint8_t transient = NOT_TRANSIENT);
    void drawpixB(uint8_t Number, CRGB Color, uint8_t transient = NOT_TRANSIENT);
    void fillpix(CRGB Color);
    void clear();

private:
    void _displaybuff(uint8_t *buffptr, int8_t offsetx = 0, int8_t offsety = 0);
};

#endif
