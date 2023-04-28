#include "TM1650.h"

#ifdef CFG_PLF_RV32
#include "asr_i2c.h"
#include "asr_pinmux.h"
#define duet_i2c_init asr_i2c_init
#define duet_pinmux_config asr_pinmux_config
// #define duet_i2c_master_send(a,b,c,d,e) asr_i2c_master_write_data(a,b,c,d)
#else
#include "duet.h"
#include "duet_i2c.h"
#endif

#include <string.h>

int32_t i2c_master_send(duet_i2c_dev_t *i2c, uint16_t dev_addr, const uint8_t *data, uint16_t size, uint32_t timeout)
{
#ifdef CFG_PLF_RV32
return asr_i2c_master_write_data(i2c, dev_addr, (uint8_t *)data, size);
#else
return duet_i2c_master_send(i2c, dev_addr, data, size, timeout);
#endif
}

/** Constructor, uses default values for the parameters
 * so could be called with no parameters.
 * aNumDigits - number of display digits (default = 4)
 */
TM1650::TM1650(int aNumDigits, uint8_t mI2cChannel) {
    iNumDigits =  aNumDigits > TM1650_NUM_DIGITS ? TM1650_NUM_DIGITS : aNumDigits;
    i2cChannel = mI2cChannel;
}

/** Initialization
 * initializes the driver. Turns display on, but clears all digits.
 */
void TM1650::init() {
    iPosition = nullptr;
    for (int i=0; i<iNumDigits; i++) {
        iBuffer[i] = 0;
        iCtrl[i] = 0;
    }
#ifdef CFG_PLF_RV32
    i2cwire.config.i2c_mst_fifo_mode = i2c_mst_fifo_mode_EN;
    i2cwire.config.i2c_role = I2C_MASTER;                 // default role = master
    i2cwire.config.i2c_speed_mode = I2C_MODE_STANDARD;    // default speed = standard mode  I2C_MODE_FAST; //
    i2cwire.config.i2c_slave_addr = 0;       // default slave addr = 0x5a
    i2cwire.config.i2c_dma_function = I2C_DMA_DISABLE;

    i2cwire.port = i2cChannel;
    i2cwire.priv = 0;

    if (i2cwire.port == I2C0_INDEX)
    {
        duet_pinmux_config(PAD2, PF_I2C0);
        duet_pinmux_config(PAD3, PF_I2C0);
    }

    if (i2cwire.port == I2C1_INDEX)
    {
        duet_pinmux_config(PAD8, PF_I2C1);
        duet_pinmux_config(PAD9, PF_I2C1);
    }
#else
    i2cwire.config.address_width = 8;
    i2cwire.config.dev_addr = 0;
    i2cwire.config.freq = I2C_STANDARD_SPEED;
    i2cwire.config.mode = I2C_MASTER;
    i2cwire.port = i2cChannel;
    i2cwire.priv = 0;
#endif
    duet_i2c_init(&i2cwire);

    clear();
    displayOn();
}

/** Set brightness of all digits equally
 * aValue - brightness value with 1 being the lowest, and 7 being the brightest
 */
void TM1650::setBrightness(int aValue) {
    if (aValue < 0) aValue = 0;
    if (aValue > TM1650_MAX_BRIGHT) aValue = TM1650_MAX_BRIGHT;

    for (int i=0; i<iNumDigits; i++) {
        iCtrl[i] = (iCtrl[i] & TM1650_MSK_BRIGHT) | ( aValue << TM1650_BRIGHT_SHIFT );
        i2c_master_send(&i2cwire, TM1650_DCTRL_BASE+i, (const uint8_t*)&iCtrl[i], 1, 5);
    }
}

/** Turns the display on
 */
void TM1650::displayOn ()
// turn all digits on
{
  for (int i=0; i<iNumDigits; i++) {
    iCtrl[i] = (iCtrl[i] & TM1650_MSK_ONOFF) | TM1650_BIT_DOT;
    i2c_master_send(&i2cwire, TM1650_DCTRL_BASE+i, (const uint8_t*)&iCtrl[i], 1, 5);
  }
}
/** Turns the display off
 */
void TM1650::displayOff ()
// turn all digits off
{
  for (int i=0; i<iNumDigits; i++) {
    iCtrl[i] = (iCtrl[i] & TM1650_MSK_ONOFF);
    i2c_master_send(&i2cwire, TM1650_DCTRL_BASE+i, (const uint8_t*)&iCtrl[i], 1, 5);
  }
}

/** Directly write to the CONTROL register of the digital position
 * aPos = position to set the control register for
 * aValue = value to write to the position
 *
 * Internal control buffer is updated as well
 */
void TM1650::controlPosition(int aPos, char aValue) {
    if (aPos >= 0 && aPos < iNumDigits) {
        iCtrl[aPos] = aValue;
        i2c_master_send(&i2cwire, TM1650_DCTRL_BASE+aPos, (const uint8_t*)&aValue, 1, 5);
    }
}

/** Directly write to the digit register of the digital position
 * aPos = position to set the digit register for
 * aValue = value to write to the position
 *
 * Internal position buffer is updated as well
 */
void TM1650::setPosition(int aPos, char aValue) {
    if (aPos >= 0 && aPos < iNumDigits) {
        iBuffer[aPos] = aValue;
        i2c_master_send(&i2cwire, TM1650_DISPLAY_BASE+aPos, (const uint8_t*)&aValue, 1, 5);
    }
}

/** Directly set/clear a 'dot' next to a specific position
 * aPos = position to set/clear the dot for
 * aState = display the dot if true, clear if false
 *
 * Internal buffer is updated as well
 */
void    TM1650::setDot(int aPos, bool aState) {
    iBuffer[aPos] = iBuffer[aPos] & 0x7F |(aState ? 0b10000000 : 0);
    setPosition(aPos, iBuffer[aPos]);
}

/** Clear all digits. Keep the display on.
 */
void TM1650::clear()
// clears all digits
{
  for (int i=0; i<iNumDigits; i++) {
     iBuffer[i] = 0;
    i2c_master_send(&i2cwire, TM1650_DISPLAY_BASE+i, (const uint8_t*)&iBuffer[i], 1, 5);
  }
}

/** Display string on the display
 * aString = character array to be displayed
 *
 * Internal buffer is updated as well
 * Only first N positions of the string are displayed if
 *  the string is longer than the number of digits
 */
void TM1650::displayString(char *aString)
{
    for (int i=0; i<iNumDigits; i++) {
      char a = ((char) aString[i]) & 0b01111111;
      char dot = ((char) aString[i]) & 0b10000000;
      iBuffer[i] = TM1650_CDigits[a];
      if (a) {
        char tmp = TM1650_CDigits[a] | dot;
        i2c_master_send(&i2cwire, TM1650_DISPLAY_BASE+i, (const uint8_t*)&tmp, 1, 5);
      }
      else
        break;

    }
}

/** Display string on the display in a running fashion
 * aString = character array to be displayed
 *
 * Starts with first N positions of the string.
 * Subsequent characters are displayed with 1 char shift each time displayRunningShift() is called
 *
 * returns: number of iterations remaining to display the whole string
 */
int TM1650::displayRunning(char *aString) {

    strncpy(iString, aString, TM1650_MAX_STRING+1);
    iPosition = iString;
    iString[TM1650_MAX_STRING] = '\0'; //just in case.
        displayString(iPosition);

    int l = strlen(iPosition);
    if (l <= iNumDigits) return 0;
    return (l - iNumDigits);
}

/** Display next segment (shifting to the left) of the string set by displayRunning()
 * Starts with first N positions of the string.
 * Subsequent characters are displayed with 1 char shift each time displayRunningShift is called
 *
 * returns: number of iterations remaining to display the whole string
 */
int TM1650::displayRunningShift() {
        if (strlen(iPosition) <= iNumDigits) return 0;
        displayString(++iPosition);
    return (strlen(iPosition) - iNumDigits);
}
