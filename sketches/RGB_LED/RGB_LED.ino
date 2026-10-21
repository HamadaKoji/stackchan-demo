/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <Arduino.h>
#include <M5StackChan.h>

struct Color_t {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

static std::vector<Color_t> colors = {
    {0, 0, 0}, {168, 0, 0}, {0, 168, 0}, {0, 0, 168}, {168, 168, 0}, {168, 0, 168}, {0, 168, 168}, {168, 168, 168},
};

void setup()
{
    /* Init StackChan */
    M5StackChan.begin();
}

void loop()
{
    /* There are 12 RGB LEDs, index 0-5 are on the left, 6-11 are on the right */
    for (int color_index = 0; color_index < colors.size(); color_index++) {
        for (int led_index = 0; led_index < 12; led_index++) {
            M5StackChan.setRgbColor(led_index, colors[color_index].r, colors[color_index].g, colors[color_index].b);
            M5StackChan.refreshRgb();
            delay(1000 / 24);
        }
    }
}
