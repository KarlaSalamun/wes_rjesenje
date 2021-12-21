#ifndef __AUDIO_H_
#define __AUDIO_H_

#include "stm32f4xx_hal.h"
#include "i2c.h"

#define AUDIO_RESET_PIN		GPIO_PIN_4
#define AUDIO_I2C_ADDRESS	0x94

extern const int16_t sinus_lut[4800];

void init_AudioReset();
void configAudio();

#endif
