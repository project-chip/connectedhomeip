/***************************************************************************//**
 * @file
 * @brief RGB Led controller.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "led_widget_rgb.h"

// cmath is needed only for HueToRGB.
#include <cmath>
#include <cstring>
#include <stdio.h>

#include "AppConfig.h"
#include "em_gpio.h"
#include "sl_led.h"
#include "sl_pwm_led.h"
#include "sl_simple_rgb_pwm_led.h"


/**
 * @brief Red led instance.
 */
sl_led_pwm_t led_pwm_red = {
  .set_color = sl_pwm_led_set_color,
  .get_color = sl_pwm_led_get_color,
  .channel = 0,
  .port = gpioPortD,
  .pin = 11,
  .location = 19,
  .level = INITIAL_RGB,
  .polarity = LED_RGB_POLARITY,
  .state = LED_RGB_INITIAL_STATE,
  .timer = TIMER1,
  .frequency = PWM_FREQUENCY,
  .resolution = PWM_RESOLUTION_LEVELS,
};

/**
 * @brief Green led instance.
 */
sl_led_pwm_t led_pwm_green = {
  .set_color = sl_pwm_led_set_color,
  .get_color = sl_pwm_led_get_color,
  .channel = 1,
  .port = gpioPortD,
  .pin = 12,
  .location = 19,
  .level = INITIAL_RGB,
  .polarity = LED_RGB_POLARITY,
  .state = LED_RGB_INITIAL_STATE,
  .timer = TIMER1,
  .frequency = PWM_FREQUENCY,
  .resolution = PWM_RESOLUTION_LEVELS,
};

/**
 * @brief Blue led instance.
 */
sl_led_pwm_t led_pwm_blue = {
  .set_color = sl_pwm_led_set_color,
  .get_color = sl_pwm_led_get_color,
  .channel = 2,
  .port = gpioPortD,
  .pin = 13,
  .location = 19,
  .level = INITIAL_RGB,
  .polarity = LED_RGB_POLARITY,
  .state = LED_RGB_INITIAL_STATE,
  .timer = TIMER1,
  .frequency = PWM_FREQUENCY,
  .resolution = PWM_RESOLUTION_LEVELS,
};

/**
 * @brief Context of the envelopping RGB led instance.
 */
sl_simple_rgb_pwm_led_context_t led_rgb_context = {
  .red = &led_pwm_red,
  .green = &led_pwm_green,   
  .blue = &led_pwm_blue,
  .timer = TIMER1,
  .frequency = PWM_FREQUENCY, 
  .resolution = PWM_RESOLUTION_LEVELS, 
  .state = LED_RGB_INITIAL_STATE,
};

/**
 * @brief Structure holding the driver functions for the LED.
 */
sl_led_t sl_led_rgb = {
    .context = &led_rgb_context,
    .init = sl_simple_rgb_pwm_led_init,
    .turn_on = sl_simple_rgb_pwm_led_turn_on,
    .turn_off = sl_simple_rgb_pwm_led_turn_off,
    .toggle = sl_simple_rgb_pwm_led_toggle,
    .get_state = sl_simple_rgb_pwm_led_get_state,
};

/**
 * @brief The envelopping RGB led instance.
 */
const sl_led_rgb_pwm_t sl_led_rgb_pwm = {
  .led_common = sl_led_rgb,
  .set_rgb_color = sl_simple_rgb_pwm_led_set_color,
  .get_rgb_color = sl_simple_rgb_pwm_led_get_color,
};


void LEDWidgetRGB::InitGpioRGB()
{
    sl_pwm_led_init(&led_pwm_red);
    sl_pwm_led_init(&led_pwm_green);
    sl_pwm_led_init(&led_pwm_blue);
    sl_led_init(&sl_led_rgb);
}


void LEDWidgetRGB::Init(const sl_led_rgb_pwm_t* led)
{
    /* 1. Initialize the value of class variables. */
    sl_simple_rgb_pwm_led_context_t* led_context;
    led_context = reinterpret_cast<sl_simple_rgb_pwm_led_context_t*>(led->led_common.context);
    this->level_resolution_ = led_context->resolution;
    this->current_hue_ = INITIAL_HUE;
    this->current_saturation_ = INITIAL_SATURATION;
    this->led_rgb_ = led;  

    /* 2. Set the RGB values to INITIAL_RGB. This does not affect the driver. */
    for (uint8_t i = 0; i < 3; i++)
    {
        this->current_rgb_[i] = INITIAL_RGB;
    } 

    /* 3. Initialize the value of the base class member variables. */
    LEDWidget::Init(&(led->led_common));

    /* 4. Turn on the RGB LED pins. */
    GPIO_PinOutSet(gpioPortJ, 14);
    GPIO_PinOutSet(gpioPortI, 0);
    GPIO_PinOutSet(gpioPortI, 1);
    GPIO_PinOutSet(gpioPortI, 2);
    GPIO_PinOutSet(gpioPortI, 3);
}


void LEDWidgetRGB::Set(bool state)
{
    this->Set(state, LED_ENDPOINT_DEFAULT);
}


void LEDWidgetRGB::Set(bool state, uint16_t led_endpoint)
{
    uint8_t rgb[3] = {0, 0, 0};
    bool save_values = false;

    if (state == true)
    {
        save_values = true;
        bool turned_on = false;

        for (int i = 0; i < 3; i++)
        {
            if (this->current_rgb_[i] > 1)
            {
                turned_on = true;
            }
        }

        if (turned_on == true) 
        {
            for (int i = 0; i < 3; i++)
            {
                rgb[i] = current_rgb_[i];
            }
        }
        else 
        {
            for (int i = 0; i < 3; i++)
            {
                rgb[i] = PWM_MAX_VALUE;
            }
        }
    }

    this->SetRGB(rgb, led_endpoint, save_values);
}


void LEDWidgetRGB::SetLevel(uint8_t level, uint16_t led_endpoint)
{
    if(led_endpoint == 1)
    {
        /* 1. Adjust the color level. */
        // The Levelcontrol cluster takes values from 0 to 254.
        // However, the PWM driver accepts values from 0 to 100.
        // See appclusters, section 1.6.6.1.
        uint8_t adjusted_level = (level * PWM_MAX_VALUE) / ATTRIBUTE_LEVEL_MAX;

        /* 2. Set the colors. */
        uint8_t rgb[3] = {adjusted_level, adjusted_level, adjusted_level};
        this->SetRGB(rgb, led_endpoint, true);
    }
}


void LEDWidgetRGB::GetLevel(uint8_t* rgb, uint32_t size)
{
    /* 1. Create a buffer and assign it to the pointer. */
    if (rgb == nullptr || rgb == NULL || size < 3)
    {
        EFR32_LOG(" Error in led_widget_rgb.cpp. Array refused.");
        return; 
    } 

    /* 2. Get the current RGB value from the driver. */
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
    void* led_rgb_context = this->led_rgb_->led_common.context;
    this->led_rgb_->get_rgb_color(led_rgb_context, &red, &green, &blue);

    /* 3. Assign the colors in the array. */
    rgb[0] = static_cast<uint8_t>(red);
    rgb[1] = static_cast<uint8_t>(green);
    rgb[2] = static_cast<uint8_t>(blue);
}


void LEDWidgetRGB::SetHue(uint8_t hue, uint16_t led_endpoint)
{
    // Hue takes a value [0, 360] and is expressed in degrees.
    // See appclusters, section 3.2.7.1.
    this->current_hue_ = static_cast<uint16_t>((hue * 360) / ATTRIBUTE_LEVEL_MAX);
    this->SetColor(this->current_hue_, this->current_saturation_, led_endpoint); 
}


void LEDWidgetRGB::SetSaturation(uint8_t sat, uint16_t led_endpoint)
{
    // Saturation takes a value [0, 1] representing a percentage.
    // The Color Control cluster accepts saturation values 0 to 254.
    // See appclusters, section 3.2.7.2.
    this->current_saturation_ = sat/254.0;
    this->SetColor(this->current_hue_, this->current_saturation_, led_endpoint);
}


void LEDWidgetRGB::SetColor(uint8_t* rgb, uint16_t led_endpoint)
{
   /* 1. Set the color values. */
    this->SetRGB(rgb, led_endpoint, true);
}


void LEDWidgetRGB::SetColor(uint8_t hue, uint8_t saturation, uint16_t led_endpoint)
{
    /* 1. Convert the hue and saturation input to RGB values. (HSV to RGB conversion) */
    uint8_t rgb[3] = {0, 0, 0};
    HueToRGB(this->current_hue_, this->current_saturation_, rgb, PWM_MAX_VALUE);

    /* 2. Update the LEDs RGB values. */
    this->SetRGB(rgb, led_endpoint, true);
}


void LEDWidgetRGB::SetRGB(uint8_t* rgb, uint16_t led_endpoint, bool memorize)
{
    /* 1. Call the PWM driver to set the new values. */
    void* led_rgb_context = this->led_rgb_->led_common.context;
    this->led_rgb_->set_rgb_color(led_rgb_context, rgb[0], rgb[1], rgb[2]);

    /* 2. Save the current RGB values. */
    if (memorize == true)
    {
        memcpy(this->current_rgb_, rgb, sizeof(this->current_rgb_));
    }
}


/**
 * @note This code is based on the HSV to RGB mathematical formula
 *       which was retrieved from: https://en.wikipedia.org/wiki/HSL_and_HSV 
 */
void LEDWidgetRGB::HueToRGB(uint16_t hue, float saturation, uint8_t* rgb, uint8_t max_value)
{
    /* 1. Normalize the values. */
    uint16_t hue_degrees = hue;
    float saturation_decimal = saturation;
    float hsv_value = 1.0;

    if (hue_degrees == 360)
    {
        hue_degrees = 0;
    } 

    /* 2. Calculate the formula parameters. */
    float chroma = hsv_value * saturation_decimal;
    float hue_prime = hue_degrees / 60.0;
    float hue_modulo = fmod(hue_prime, 2.0);
    float hue_diff = hue_modulo - 1.0;
    float hue_abs = fabs(hue_diff);
    float median_value = chroma * (1.0 - hue_abs);
    float m = hsv_value - chroma;

    /* 3. Determine the points R', G' and B'. */
    float r_prime, g_prime, b_prime = 0;

    if ( hue_degrees < 60 ) 
    {
        r_prime = chroma;
        g_prime = median_value;
        b_prime = 0;
    }
    else if ( (hue_degrees >= 60) && (hue_degrees < 120) )
    {
        r_prime = median_value;
        g_prime = chroma;
        b_prime = 0;
    }
    else if ( (hue_degrees >= 120) && (hue_degrees < 180) )
    {
        r_prime = 0;
        g_prime = chroma;
        b_prime = median_value;
    }
    else if ( (hue_degrees >= 180) && (hue_degrees < 240) )
    {
        r_prime = 0;
        g_prime = median_value;
        b_prime = chroma;
    }
    else if ( (hue_degrees >= 240) && (hue_degrees < 300) )
    {
        r_prime = median_value;
        g_prime = 0;
        b_prime = chroma;
    }
    else if ( (hue_degrees >= 300) && (hue_degrees < 360) )
    {
        r_prime = chroma;
        g_prime = 0;
        b_prime = median_value;
    }

    /* 4. Calculate the final values of RGB. */
    rgb[0] = (uint8_t) ( (r_prime + m) * max_value );
    rgb[1] = (uint8_t) ( (g_prime + m) * max_value );
    rgb[2] = (uint8_t) ( (b_prime + m) * max_value );
}