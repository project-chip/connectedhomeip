
/**
 * @file    BufferedSpi.h
 * @brief   Software Buffer - Extends mbed SPI functionallity
 * @author  Armelle Duboc
 * @version 1.0
 * @see
 *
 * Copyright (c) STMicroelectronics 2017
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BUFFEREDSPI_H
#define BUFFEREDSPI_H

#include "mbed.h"
#include "MyBuffer.h"

/** A spi port (SPI) for communication with wifi device
 *
 * Can be used for Full Duplex communication, or Simplex by specifying
 * one pin as NC (Not Connected)
 *
 * Example:
 * @code
 *  #include "mbed.h"
 *  #include "BufferedSerial.h"
 *
 *  BufferedSerial pc(USBTX, USBRX);
 *
 *  int main()
 *  {
 *      while(1)
 *      {
 *          Timer s;
 *
 *          s.start();
 *          pc.printf("Hello World - buffered\n");
 *          int buffered_time = s.read_us();
 *          wait(0.1f); // give time for the buffer to empty
 *
 *          s.reset();
 *          printf("Hello World - blocking\n");
 *          int polled_time = s.read_us();
 *          s.stop();
 *          wait(0.1f); // give time for the buffer to empty
 *
 *          pc.printf("printf buffered took %d us\n", buffered_time);
 *          pc.printf("printf blocking took %d us\n", polled_time);
 *          wait(0.5f);
 *      }
 *  }
 * @endcode
 */

/**
 *  @class BufferedSpi
 *  @brief Software buffers and interrupt driven tx and rx for Serial
 */
class BufferedSpi : public SPI {
private:
    DigitalOut    nss;
    MyBuffer <char> _txbuf;
    uint32_t      _buf_size;
    uint32_t      _tx_multiple;
    volatile int _timeout;
    void txIrq(void);
    void prime(void);

    InterruptIn *_datareadyInt;
    volatile int _cmddata_rdy_rising_event;
    void DatareadyRising(void);
    int wait_cmddata_rdy_rising_event(void);
    int wait_cmddata_rdy_high(void);


    Callback<void()> _cbs[2];

    Callback<void()> _sigio_cb;
    uint8_t          _sigio_event;

public:
    MyBuffer <char> _rxbuf;
    DigitalIn dataready;
    enum IrqType {
        RxIrq = 0,
        TxIrq,

        IrqCnt
    };

    /** Create a BufferedSpi Port, connected to the specified transmit and receive pins
     *  @param SPI mosi pin
     *  @param SPI miso pin
     *  @param SPI sclk pin
     *  @param SPI nss pin
     *  @param Dataready pin
     *  @param buf_size printf() buffer size
     *  @param tx_multiple amount of max printf() present in the internal ring buffer at one time
     *  @param name optional name
    */
    BufferedSpi(PinName mosi, PinName miso, PinName sclk, PinName nss, PinName datareadypin, uint32_t buf_size = 2500, uint32_t tx_multiple = 1, const char *name = NULL);

    /** Destroy a BufferedSpi Port
     */
    virtual ~BufferedSpi(void);

    /** call to SPI frequency Function
     */
    virtual void frequency(int hz);

    /** clear the transmit buffer
     */
    virtual void flush_txbuf(void);

    /** call to SPI format function
     */
    virtual void format(int bits, int mode);

    virtual void enable_nss(void);

    virtual void disable_nss(void);

    /** Check on how many bytes are in the rx buffer
     *  @return 1 if something exists, 0 otherwise
     */
    virtual int readable(void);

    /** Check to see if the tx buffer has room
     *  @return 1 always has room and can overwrite previous content if too small / slow
     */
    virtual int writeable(void);

    /** Get a single byte from the BufferedSpi Port.
     *  Should check readable() before calling this.
     *  @return A byte that came in on the SPI Port
     */
    virtual int getc(void);

    /** Write a single byte to the BufferedSpi Port.
     *  @param c The byte to write to the SPI Port
     *  @return The byte that was written to the SPI Port Buffer
     */
    virtual int putc(int c);

    /** Write a string to the BufferedSpi Port. Must be NULL terminated
     *  @param s The string to write to the Spi Port
     *  @return The number of bytes written to the Spi Port Buffer
     */
    virtual int puts(const char *s);

    /** Write a formatted string to the BufferedSpi Port.
     *  @param format The string + format specifiers to write to the Spi Port
     *  @return The number of bytes written to the Spi Port Buffer
     */
    virtual int printf(const char *format, ...);

    /** Write data to the Buffered Spi Port
     *  @param s A pointer to data to send
     *  @param length The amount of data being pointed to
     *  @return The number of bytes written to the Spi Port Buffer
     */
    virtual ssize_t buffwrite(const void *s, std::size_t length);

    /** Send datas to the Spi port that are already present
     *  in the internal _txbuffer
     *  @param length
     *  @return the number of bytes written on the SPI port
     */
    virtual ssize_t buffsend(size_t length);

    /** Read data from the Spi Port to the _rxbuf
     *  @param max: optional. = max sieze of the input read
     *  @return The number of bytes read from the SPI port and written to the _rxbuf
     */
    virtual ssize_t read();
    virtual ssize_t read(uint32_t max);

    /**
    * Allows timeout to be changed between commands
    *
    * @param timeout timeout of the connection in ms
    */
    void setTimeout(int timeout)
    {
        /*  this is a safe guard timeout at SPI level in case module is stuck */
        _timeout = timeout;
    }

    /** Register a callback once any data is ready for sockets
     *  @param func     Function to call on state change
     */
    virtual void sigio(Callback<void()> func);

    /** Attach a function to call whenever a serial interrupt is generated
     *  @param func A pointer to a void function, or 0 to set as none
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    virtual void attach(Callback<void()> func, IrqType type = RxIrq);

    /** Attach a member function to call whenever a serial interrupt is generated
     *  @param obj pointer to the object to call the member function on
     *  @param method pointer to the member function to call
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    template <typename T>
    void attach(T *obj, void (T::*method)(), IrqType type = RxIrq)
    {
        attach(Callback<void()>(obj, method), type);
    }

    /** Attach a member function to call whenever a serial interrupt is generated
     *  @param obj pointer to the object to call the member function on
     *  @param method pointer to the member function to call
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    template <typename T>
    void attach(T *obj, void (*method)(T *), IrqType type = RxIrq)
    {
        attach(Callback<void()>(obj, method), type);
    }
};
#endif
