/* ISM43362Interface Example
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

#ifndef ISM43362_H
#define ISM43362_H
#include "ATParser.h"

#define ES_WIFI_MAX_SSID_NAME_SIZE                  32
#define ES_WIFI_MAX_PSWD_NAME_SIZE                  32
#define ES_WIFI_PRODUCT_ID_SIZE                     32
#define ES_WIFI_PRODUCT_NAME_SIZE                   32
#define ES_WIFI_FW_REV_SIZE                         16
#define ES_WIFI_API_REV_SIZE                        16
#define ES_WIFI_STACK_REV_SIZE                      16
#define ES_WIFI_RTOS_REV_SIZE                       16

// The input range for AT Command 'R1' is 0 to 1200 bytes
// 'R1' Set Read Transport Packet Size (bytes)
#define ES_WIFI_MAX_RX_PACKET_SIZE                     1200
// Module maxume DATA payload for Tx packet is 1460
#define ES_WIFI_MAX_TX_PACKET_SIZE                     1460
typedef enum ism_security {
    ISM_SECURITY_NONE         = 0x0,      /*!< open access point */
    ISM_SECURITY_WEP          = 0x1,      /*!< phrase conforms to WEP */
    ISM_SECURITY_WPA          = 0x2,      /*!< phrase conforms to WPA */
    ISM_SECURITY_WPA2         = 0x3,      /*!< phrase conforms to WPA2 */
    ISM_SECURITY_WPA_WPA2     = 0x4,      /*!< phrase conforms to WPA/WPA2 */
    ISM_SECURITY_UNKNOWN      = 0xFF,     /*!< unknown/unsupported security in scan results */
} ism_security_t;

extern "C" int32_t ParseNumber(char *ptr, uint8_t *cnt);

/** ISM43362Interface class.
    This is an interface to a ISM43362 radio.
 */
class ISM43362 {
public:
    ISM43362(PinName mosi, PinName miso, PinName clk, PinName nss, PinName resetpin, PinName datareadypin, PinName wakeup, bool debug = false);

    /**
    * Check firmware version of ISM43362
    *
    * @return fw version or null if no version is read
    */
    uint32_t get_firmware_version(void);

    /**
    * Reset ISM43362
    *
    * @return true only if ISM43362 resets successfully
    */
    bool reset(void);

    /**
    * Enable/Disable DHCP
    *
    * @param enabled DHCP enabled when true
    * @return true only if ISM43362 enables/disables DHCP successfully
    */
    bool dhcp(bool enabled);

    /**
    * Connect ISM43362 to AP
    *
    * @param ap the name of the AP
    * @param passPhrase the password of AP
    * @param ap_sec the security level of network AP
    * @return nsapi_error enum
    */
    int connect(const char *ap, const char *passPhrase, ism_security_t ap_sec);

    /**
    * Disconnect ISM43362 from AP
    *
    * @return true only if ISM43362 is disconnected successfully
    */
    bool disconnect(void);

    /**
    * Get the IP address of ISM43362
    *
    * @return null-teriminated IP address or null if no IP address is assigned
    */
    const char *getIPAddress(void);

    /**
    * Get the MAC address of ISM43362
    *
    * @return null-terminated MAC address or null if no MAC address is assigned
    */
    const char *getMACAddress(void);

    /** Get the local gateway
    *
    *  @return         Null-terminated representation of the local gateway
    *                  or null if no network mask has been recieved
    */
    const char *getGateway();

    /** Get the local network mask
     *
     *  @return         Null-terminated representation of the local network mask
     *                  or null if no network mask has been recieved
     */
    const char *getNetmask();

    /* Return RSSI for active connection
     *
     * @return      Measured RSSI
     */
    int8_t getRSSI();

    /**
    * Check if ISM43362 is conenected
    *
    * @return true only if the chip has an IP address
    */
    bool isConnected(void);

    /** Scan for available networks
     *
     * @param  ap    Pointer to allocated array to store discovered AP, or 0 to only count available AP
     * @param  limit Size of allocated @a res array, or 0 to only count available AP
     * @return       Number of entries in @a res, or if @a count was 0 number of available networks, negative on error
     *               see @a nsapi_error
     */
    int scan(WiFiAccessPoint *res, unsigned limit);

    /**
    * Open a socketed connection
    *
    * @param type the type of socket to open "UDP" or "TCP"
    * @param id id to give the new socket, valid 0-4
    * @param port port to open connection with
    * @param addr the IP address of the destination
    * @return
    * @   NSAPI_ERROR_OK : socket opened successfully
    * @   NSAPI_ERROR_PARAMETER : invalid configuration
    * @   NSAPI_ERROR_DEVICE_ERROR :
    * @   failure interfacing with the network processor
    */
    int open(const char *type, int id, const char *addr, int port);

    /**
    * Sends data to an open socket
    *
    * @param id id of socket to send to
    * @param data data to be sent
    * @param amount amount of data to be sent - max 1024
    * @return true only if data sent successfully
    */
    bool send(int id, const void *data, uint32_t amount);

    /**
    * Receives data from an open socket
    *
    * @param id id to receive from
    * @param data placeholder for returned information
    * @param amount number of bytes to be received
    * @return the number of bytes received
    */
    int32_t recv(int id, void *data, uint32_t amount);

    /**
    * Closes a socket
    *
    * @param id id of socket to close, valid only 0-4
    * @return true only if socket is closed successfully
    */
    bool close(int id);

    /**
    * Checks if data is available
    */
    bool readable();

    /**
    * Checks if data can be written
    */
    bool writeable();

    /**
    * Attach a function to call whenever network state has changed
    *
    * @param func A pointer to a void function, or 0 to set as none
    */
    void attach(Callback<void()> func);

    /**
    * Check is datas are available to read for a socket
    * @param id socket id
    * @param data placeholder for returned information
    * @param amount size to read for the check
    * @return amount of read value, or -1 for errors
    */
    int check_recv_status(int id, void *data);

    /**
    * Attach a function to call whenever network state has changed
    *
    * @param obj pointer to the object to call the member function on
    * @param method pointer to the member function to call
    */
    template <typename T, typename M>
    void attach(T *obj, M method)
    {
        attach(Callback<void()>(obj, method));
    }

    /** Get the connection status
     *
     *  @return         The connection status according to ConnectionStatusType
     */
    nsapi_connection_status_t connection_status() const;


private:
    BufferedSpi _bufferspi;
    ATParser _parser;
    DigitalOut _resetpin;
    volatile int _active_id;
    void print_rx_buff(void);
    bool check_response(void);

#ifdef MBED_CONF_ISM43362_WIFI_COUNTRY_CODE
    bool check_country_code(const char *country_code);
    char WIFI_module_country_code[5];
#endif

    struct packet {
        struct packet *next;
        int id;
        uint32_t len;
        // data follows
    } *_packets, * *_packets_end;
    void _packet_handler();
    bool _ism_debug;

    char _ip_buffer[16];
    char _gateway_buffer[16];
    char _netmask_buffer[16];
    char _mac_buffer[18];
    uint32_t _FwVersionId;

    // Connection state reporting
    nsapi_connection_status_t _conn_status;
    mbed::Callback<void()> _conn_stat_cb;

    typedef struct {
        char cc[3];
    } COUNTRY_CODE;

    COUNTRY_CODE CountryCodeElevenChannels[12] = {"AS", "CA", "FM", "GU", "KY", "MP", "PR", "TW",
                                                  "UM", "US", "VI", "ED"
                                                 };

    COUNTRY_CODE CountryCodeThirteenChannels[127] = {"AE", "AG", "AN", "AR", "AT", "AU", "AW",
                                                     "AZ", "BA", "BB", "BD", "BE", "BG", "BH",
                                                     "BM", "BN", "BO", "BR", "BS", "BT", "BY",
                                                     "CH", "CN", "CL", "CO", "CR", "CU", "CV",
                                                     "CY", "CZ", "DE", "DK", "DM", "DO", "EC",
                                                     "EE", "EG", "ES", "FI", "FK", "FR", "GB",
                                                     "GF", "GG", "GI", "GP", "GR", "GT", "HK",
                                                     "HN", "HR", "HT", "HU", "ID", "IE", "IL",
                                                     "IM", "IN", "IS", "IT", "JE", "JM", "JO",
                                                     "KE", "KI", "KR", "KW", "LA", "LB", "LI",
                                                     "LK", "LS", "LT", "LU", "LV", "MA", "MC",
                                                     "MK", "MO", "MQ", "MR", "MT", "MU", "MV",
                                                     "MW", "MX", "MY", "NG", "NI", "NL", "NO",
                                                     "NZ", "OM", "PA", "PE", "PG", "PH", "PK",
                                                     "PL", "PM", "PT", "RE", "RO", "RU", "SA",
                                                     "SE", "SG", "SI", "SK", "SV", "TH", "TJ",
                                                     "TN", "TR", "TT", "TZ", "UA", "UY", "UZ",
                                                     "VA", "VE", "VG", "VN", "YT", "ZA", "ZM",
                                                     "ED"
                                                    };
};

#endif
