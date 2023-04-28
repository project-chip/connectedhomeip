

#ifndef dht11_h
#define dht11_h

#ifdef CFG_PLF_RV32
#include "asr_pinmux.h"
#include "asr_gpio.h"
#include "asr_timer.h"
#define duet_gpio_dev_t asr_gpio_dev_t
#define duet_timer_dev_t asr_timer_dev_t
#else
#include "duet_pinmux.h"
#include "duet_gpio.h"
#include "duet_timer.h"
#endif

#define DHT11LIB_VERSION "0.4.1"

#define DHTLIB_OK                 0
#define DHTLIB_ERROR_CHECKSUM    -1
#define DHTLIB_ERROR_TIMEOUT     -2


#define DHT_PIN              GPIO13_INDEX

class DHT11
{
private:
    duet_gpio_dev_t gpio;
public:
    void    init();
    int     read(int pin);
    int     humidity;
    int     temperature;
    duet_timer_dev_t micro_timer;
};
#endif
//
// END OF FILE
//