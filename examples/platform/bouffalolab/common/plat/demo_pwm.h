#ifndef __DEMO_PWM__
#define __DEMO_PWM__

#ifdef __cplusplus
extern "C" {
#endif

void demo_pwm_init(void);
void demo_pwm_start(void);
void set_color_red(uint8_t currLevel);
void set_color_green(uint8_t currLevel);
void set_color_yellow(uint8_t currLevel);
void set_color(uint8_t currLevel, uint8_t currHue, uint8_t currSat);

void set_level(uint8_t currLevel);

#ifdef __cplusplus
}
#endif

#endif // __DEMO_PWM__
