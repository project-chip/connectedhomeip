//
//    FILE: dht11.cpp
// VERSION: 0.4.1
// PURPOSE: DHT11 Temperature & Humidity Sensor library for Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET: http://www.micro4you.com/files/sensor/DHT11.pdf
//
// HISTORY:
// George Hadjikyriacou - Original version (??)
// Mod by SimKard - Version 0.2 (24/11/2010)
// Mod by Rob Tillaart - Version 0.3 (28/03/2011)
// + added comments
// + removed all non DHT11 specific code
// + added references
// Mod by Rob Tillaart - Version 0.4 (17/03/2012)
// + added 1.0 support
// Mod by Rob Tillaart - Version 0.4.1 (19/05/2012)
// + added error codes
//

#include "dht11.h"

#include <lega_rtos_api.h>

#ifdef CFG_PLF_RV32
#define duet_gpio_init asr_gpio_init
#define duet_gpio_output_low asr_gpio_output_low
#define duet_gpio_output_high asr_gpio_output_high
#define duet_gpio_input_get asr_gpio_input_get
#define DUET_TIMER2_INDEX 1
#define DUET_OUTPUT_PUSH_PULL ASR_OUTPUT_PUSH_PULL
#define DUET_INPUT_HIGH_IMPEDANCE ASR_INPUT_HIGH_IMPEDANCE
#define duet_timer_init asr_timer_init
#define duet_timer_start asr_timer_start
#define duet_timer_get asr_timer_get
#else
#include <duet_common.h>
#endif

void timercallback(void *data)
{
    // use as freerun timer, do noting.
    return;
}

// Return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
void DHT11::init()
{
    micro_timer.port = DUET_TIMER2_INDEX;
    micro_timer.config.reload_mode = TIMER_RELOAD_AUTO;
    micro_timer.config.cb = timercallback;
    micro_timer.config.arg = NULL;

    micro_timer.config.period = 0xFFFFFFFF; //us

    duet_timer_init(&micro_timer);
    duet_timer_start(&micro_timer);
}

int DHT11::read(int pin)
{
    // BUFFER TO RECEIVE
    uint8_t bits[5];
    uint8_t cnt = 7;
    uint8_t idx = 0;

    // EMPTY BUFFER
    for (int i = 0; i < 5; i++)
    {
        bits[i] = 0;
    }

    gpio.port = pin;
    gpio.config = DUET_OUTPUT_PUSH_PULL;
    duet_gpio_init(&gpio);

    duet_gpio_output_low(&gpio);
    lega_rtos_delay_milliseconds(18);
    duet_gpio_output_high(&gpio);
    delay(40);

    gpio.port = pin;
    gpio.config = DUET_INPUT_HIGH_IMPEDANCE;
    duet_gpio_init(&gpio);


    uint32_t readPin;
    unsigned long t;
    unsigned int loopCnt;

    loopCnt = 10000;
    t = duet_timer_get(&micro_timer);
    do
    {
        duet_gpio_input_get(&gpio, &readPin);
        if (loopCnt-- == 0) {
            return DHTLIB_ERROR_TIMEOUT;
        }
    } while (readPin == 0);


    loopCnt = 10000;
    t = duet_timer_get(&micro_timer);
    do
    {
        duet_gpio_input_get(&gpio, &readPin);
        if (loopCnt-- == 0) {
            return DHTLIB_ERROR_TIMEOUT;
        }
    } while (readPin == 1);

    // READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
    for (int i=0; i < 40; i++)
    {
        loopCnt = 10000;
        do
        {
            duet_gpio_input_get(&gpio, &readPin);
            if (loopCnt-- == 0) {
                return DHTLIB_ERROR_TIMEOUT;
            }
        } while (readPin == 0);

        t = duet_timer_get(&micro_timer);

        loopCnt = 10000;
        do
        {
            duet_gpio_input_get(&gpio, &readPin);
            if (loopCnt-- == 0) {
                return DHTLIB_ERROR_TIMEOUT;
            }
        } while (readPin == 1);

        if ((t - duet_timer_get(&micro_timer)) > 40) {
            bits[idx] |= (1 << cnt);
        }

        if (cnt == 0)   // next byte?
        {
            cnt = 7;    // restart at MSB
            idx++;      // next byte!
        }
        else
        {
            cnt--;
        }
    }

    // WRITE TO RIGHT VARS
        // as bits[1] and bits[3] are allways zero they are omitted in formulas.
    humidity    = bits[0] * 10 + bits[1];
    temperature = bits[2] * 10 + bits[3];

    uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];

    if (bits[4] != sum) {
        return DHTLIB_ERROR_CHECKSUM;
    }

    return DHTLIB_OK;
}
//
// END OF FILE
//