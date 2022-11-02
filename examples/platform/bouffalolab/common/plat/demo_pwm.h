#ifndef __DEMO_PWM__
#define __DEMO_PWM__

#include <board.h>
#include <hosal_pwm.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void demo_hosal_pwm_init(void);
void demo_hosal_pwm_start(void);
void demo_hosal_pwm_change_param(hosal_pwm_config_t * para);
void set_color_red(uint8_t currLevel);
void set_color_green(uint8_t currLevel);
void set_color_yellow(uint8_t currLevel);
void set_color(uint8_t currLevel, uint8_t currHue, uint8_t currSat);

void set_level(uint8_t currLevel);

#ifdef __cplusplus
}
#endif

#endif // __DEMO_PWM__
