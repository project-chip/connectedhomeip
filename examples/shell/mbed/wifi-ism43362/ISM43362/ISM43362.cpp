/* ISM43362 Example
*
* Copyright (c) STMicroelectronics 2018
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
#include <inttypes.h>
#include "ISM43362.h"
#include "mbed_debug.h"

// activate / de-activate debug
#define ism_debug 0

/* The minimum FW version to have AP scan one by one is C3.5.2.5 (ATcommand F0=2) */
#define SINGLE_AP_SCAN_FW_VERSION_NUMBER 3525

ISM43362::ISM43362(PinName mosi, PinName miso, PinName sclk, PinName nss, PinName resetpin, PinName datareadypin, PinName wakeup, bool debug)
    : _bufferspi(mosi, miso, sclk, nss, datareadypin),
      _parser(_bufferspi),
      _resetpin(resetpin),
      _packets(0), _packets_end(&_packets)
{
    DigitalOut wakeup_pin(wakeup);
    _bufferspi.format(16, 0); /* 16bits, ploarity low, phase 1Edge, master mode */
    _bufferspi.frequency(20000000); /* up to 20 MHz */
    _active_id = 0xFF;
    _FwVersionId = 0;

    _ism_debug = debug || ism_debug;
    reset();
}

/**
  * @brief  Parses and returns number from string.
  * @param  ptr: pointer to string
  * @param  cnt: pointer to the number of parsed digit
  * @retval integer value.
  */
#define CHARISHEXNUM(x)                 (((x) >= '0' && (x) <= '9') || \
                                         ((x) >= 'a' && (x) <= 'f') || \
                                         ((x) >= 'A' && (x) <= 'F'))
#define CHARISNUM(x)                    ((x) >= '0' && (x) <= '9')
#define CHAR2NUM(x)                     ((x) - '0')


extern "C" int32_t ParseNumber(char *ptr, uint8_t *cnt)
{
    uint8_t minus = 0, i = 0;
    int32_t sum = 0;

    if (*ptr == '-') {                                      /* Check for minus character */
        minus = 1;
        ptr++;
        i++;
    }
    if (*ptr == 'C') {  /* input string from get_firmware_version is Cx.x.x.x */
        ptr++;
    }

    while (CHARISNUM(*ptr) || (*ptr == '.')) { /* Parse number */
        if (*ptr == '.') {
            ptr++; // next char
        } else {
            sum = 10 * sum + CHAR2NUM(*ptr);
            ptr++;
            i++;
        }
    }

    if (cnt != NULL) {                   /* Save number of characters used for number */
        *cnt = i;
    }
    if (minus) {                         /* Minus detected */
        return 0 - sum;
    }
    return sum;                          /* Return number */
}

uint32_t ISM43362::get_firmware_version(void)
{
    char tmp_buffer[250];
    char *ptr, *ptr2;
    char _fw_version[16];

    /* Use %[^\n] instead of %s to allow having spaces in the string */
    if (!(_parser.send("I?") && _parser.recv("%[^\n^\r]\r\n", tmp_buffer) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: get_firmware_version is FAIL\r\n");
        return 0;
    }
    debug_if(_ism_debug, "\tISM43362: get_firmware_version = %s\r\n", tmp_buffer);

    // Get the first version in the string
    ptr = strtok((char *)tmp_buffer, ",");
    ptr = strtok(NULL, ",");
    ptr2 = strtok(NULL, ",");
    if (ptr == NULL) {
        debug_if(_ism_debug, "\tISM43362: get_firmware_version decoding is FAIL\r\n");
        return 0;
    }
    strncpy(_fw_version, ptr, ptr2 - ptr);
    _FwVersionId = ParseNumber(_fw_version, NULL);

    return _FwVersionId;
}

bool ISM43362::reset(void)
{
    char tmp_buffer[100];
    debug_if(_ism_debug, "\tISM43362: Reset Module\r\n");
    _resetpin = 0;
    wait_us(10000);
    _resetpin = 1;
    rtos::ThisThread::sleep_for(500);

    /* Wait for prompt line : the string is "> ". */
    /* As the space char is not detected by sscanf function in parser.recv, */
    /* we need to use %[\n] */
    if (!_parser.recv(">%[^\n]", tmp_buffer)) {
        debug_if(_ism_debug, "\tISM43362: Reset Module failed\r\n");
        return false;
    }
    return true;
}

void ISM43362::print_rx_buff(void)
{
    char tmp[150] = {0};
    uint16_t i = 0;
    debug_if(_ism_debug, "\tISM43362: ");
    while (i  < 150) {
        int c = _parser.getc();
        if (c < 0) {
            break;
        }
        tmp[i] = c;
        debug_if(_ism_debug, "0x%2X ", c);
        i++;
    }
    debug_if(_ism_debug, "\n");
    debug_if(_ism_debug, "\tISM43362: Buffer content =====%s=====\r\n", tmp);
}

/*  checks the standard OK response of the WIFI module, shouldbe:
 *  \r\nDATA\r\nOK\r\n>sp
 *  or
 *  \r\nERROR\r\nUSAGE\r\n>sp
 *  function returns true if OK, false otherwise. In case of error,
 *  print error content then flush buffer */
bool ISM43362::check_response(void)
{
    char tmp_buffer[100];
    if (!_parser.recv("OK\r\n")) {
        print_rx_buff();
        _parser.flush();
        return false;
    }

    /*  Then we should get the prompt: "> " */
    /* As the space char is not detected by sscanf function in parser.recv, */
    /* we need to use %[\n] */
    if (!_parser.recv(">%[^\n]", tmp_buffer)) {
        debug_if(_ism_debug, "\tISM43362: Missing prompt in WIFI resp\r\n");
        print_rx_buff();
        _parser.flush();
        return false;
    }

    /*  Inventek module do stuffing / padding of data with 0x15,
     *  in case buffer contains such */
    while (1) {
        int c = _parser.getc();
        if (c == 0x15) {
            // debug_if(_ism_debug, "\tISM43362: Flush char 0x%x\n", c);
            continue;
        } else {
            /*  How to put it back if needed ? */
            break;
        }
    }
    return true;
}

bool ISM43362::dhcp(bool enabled)
{
    return (_parser.send("C4=%d", enabled ? 1 : 0) && check_response());
}

/*
  * @brief  Update wifi_module_country_code.
  * @param  country_code: User's country code.
  * @retval boolean value : false ->error

WiFi module is only able to receive 4 country codes: CA, US, FR and JP.
CA, US -> 11 channels
FR     -> 13 channels
JP     -> 14 channels

Thus 3 country code groups can be defined :

- Country Code Group corresponding to 11 channels : CountryCodeElevenChannels,
- Country Code Group corresponding to 13 channels : CountryCodeThirteenChannels,
- Country Code Group corresponding to 14 channels : CountryCodeFourteenChannels.

For an user's country code (input country code), the function is specifying
to which Country Code Group it is belonging.

For instance,
user's country code=VN (Vietnam - 13 channels) is belonging
to CountryCodeThirteenChannels.

Thus function is specifying that WIFI module must be configured
with country code = FR.

Function is updating WIFI_module_country_code with FR.

*/

#ifdef MBED_CONF_ISM43362_WIFI_COUNTRY_CODE
bool ISM43362::check_country_code(const char *country_code)
{
    if ((!strcmp(country_code, "CA")) || (!strcmp(country_code, "US"))
            || (!strcmp(country_code, "FR")) || (!strcmp(country_code, "JP"))) {
        strcpy(WIFI_module_country_code, country_code);
        return true;
    }

    int k = 0;

    while (strcmp(CountryCodeThirteenChannels[k].cc, "ED")) {
        if (!strcmp(CountryCodeThirteenChannels[k].cc, country_code)) {
            strcpy(WIFI_module_country_code, "FR");
            return true;
        }

        k++;
    }

    k = 0;

    while (strcmp(CountryCodeElevenChannels[k].cc, "ED")) {
        if (!strcmp(CountryCodeElevenChannels[k].cc, country_code)) {
            strcpy(WIFI_module_country_code, "US");
            return true;
        }

        k++;
    }

    return false;
}
#endif

int ISM43362::connect(const char *ap, const char *passPhrase, ism_security_t ap_sec)
{
    char tmp[256];

    if (!(_parser.send("C1=%s", ap) && check_response())) {
        return NSAPI_ERROR_PARAMETER;
    }

    if (!(_parser.send("C2=%s", passPhrase) && check_response())) {
        return NSAPI_ERROR_PARAMETER;
    }

    /* Check security level is acceptable */
    if (ap_sec > ISM_SECURITY_WPA_WPA2) {
        debug_if(_ism_debug, "\tISM43362: Unsupported security level %d\n", ap_sec);
        return NSAPI_ERROR_UNSUPPORTED;
    }

    if (!(_parser.send("C3=%d", ap_sec) && check_response())) {
        return NSAPI_ERROR_PARAMETER;
    }

#ifdef MBED_CONF_ISM43362_WIFI_COUNTRY_CODE
    /* Check country code is acceptable */
    bool ret = check_country_code(MBED_CONF_ISM43362_WIFI_COUNTRY_CODE);
    if (ret == false) {
        printf("ISM43362::connect Country Code ERROR\n");
        return NSAPI_ERROR_PARAMETER;
    }

    if (!(_parser.send("CN=%s/0",  WIFI_module_country_code) && check_response())) {
        return NSAPI_ERROR_PARAMETER;
    }
#endif

    if (_parser.send("C0")) {
        while (_parser.recv("%[^\n]\n", tmp)) {
            if (strstr(tmp, "OK")) {
                _parser.flush();
                _conn_status = NSAPI_STATUS_GLOBAL_UP;
                _conn_stat_cb();
                return NSAPI_ERROR_OK;
            }
            if (strstr(tmp, "JOIN")) {
                _conn_status = NSAPI_STATUS_CONNECTING;
                _conn_stat_cb();
                if (strstr(tmp, "Failed")) {
                    _parser.flush();
                    return NSAPI_ERROR_AUTH_FAILURE;
                }
            }
        }
    }

    return NSAPI_ERROR_NO_CONNECTION;
}

bool ISM43362::disconnect(void)
{
    _conn_status = NSAPI_STATUS_DISCONNECTED;
    _conn_stat_cb();
    return (_parser.send("CD") && check_response());
}

const char *ISM43362::getIPAddress(void)
{
    char tmp_ip_buffer[250];
    char *ptr, *ptr2;

    /* Use %[^\n] instead of %s to allow having spaces in the string */
    if (!(_parser.send("C?")
            && _parser.recv("%[^\n^\r]\r\n", tmp_ip_buffer)
            && check_response())) {
        debug_if(_ism_debug, "\tISM43362: getIPAddress LINE KO: %s\n", tmp_ip_buffer);
        return 0;
    }

    /* Get the IP address in the result */
    /* TODO : check if the begining of the string is always = "eS-WiFi_AP_C47F51011231," */
    ptr = strtok((char *)tmp_ip_buffer, ",");
    ptr = strtok(NULL, ",");
    ptr = strtok(NULL, ",");
    ptr = strtok(NULL, ",");
    ptr = strtok(NULL, ",");
    ptr = strtok(NULL, ",");
    ptr2 = strtok(NULL, ",");
    if (ptr == NULL) {
        return 0;
    }
    strncpy(_ip_buffer, ptr, ptr2 - ptr);

    tmp_ip_buffer[59] = 0;
    debug_if(_ism_debug, "\tISM43362: receivedIPAddress: %s\n", _ip_buffer);

    return _ip_buffer;
}

const char *ISM43362::getMACAddress(void)
{
    if (!(_parser.send("Z5") && _parser.recv("%s\r\n", _mac_buffer) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: receivedMacAddress LINE KO: %s\n", _mac_buffer);
        return 0;
    }

    debug_if(_ism_debug, "\tISM43362: receivedMacAddress:%s, size=%d\r\n", _mac_buffer, sizeof(_mac_buffer));

    return _mac_buffer;
}

const char *ISM43362::getGateway()
{
    char tmp[250];
    /* Use %[^\n] instead of %s to allow having spaces in the string */
    if (!(_parser.send("C?") && _parser.recv("%[^\n^\r]\r\n", tmp) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: getGateway LINE KO: %s\r\n", tmp);
        return 0;
    }

    /* Extract the Gateway in the received buffer */
    char *ptr;
    ptr = strtok(tmp, ",");
    for (int i = 0; i < 7; i++) {
        if (ptr == NULL) {
            break;
        }
        ptr = strtok(NULL, ",");
    }

    strncpy(_gateway_buffer, ptr, sizeof(_gateway_buffer));

    debug_if(_ism_debug, "\tISM43362: getGateway: %s\r\n", _gateway_buffer);

    return _gateway_buffer;
}

const char *ISM43362::getNetmask()
{
    char tmp[250];
    /* Use %[^\n] instead of %s to allow having spaces in the string */
    if (!(_parser.send("C?") && _parser.recv("%[^\n^\r]\r\n", tmp) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: getNetmask LINE KO: %s\n", tmp);
        return 0;
    }

    /* Extract Netmask in the received buffer */
    char *ptr;
    ptr = strtok(tmp, ",");
    for (int i = 0; i < 6; i++) {
        if (ptr == NULL) {
            break;
        }
        ptr = strtok(NULL, ",");
    }

    strncpy(_netmask_buffer, ptr, sizeof(_netmask_buffer));

    debug_if(_ism_debug, "\tISM43362: getNetmask: %s\r\n", _netmask_buffer);

    return _netmask_buffer;
}

int8_t ISM43362::getRSSI()
{
    int8_t rssi;
    char tmp[25];

    if (!(_parser.send("CR") && _parser.recv("%s\r\n", tmp) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: getRSSI LINE KO: %s\r\n", tmp);
        return 0;
    }

    rssi = ParseNumber(tmp, NULL);

    debug_if(_ism_debug, "\tISM43362: getRSSI: %d\r\n", rssi);

    return rssi;
}
/**
  * @brief  Parses Security type.
  * @param  ptr: pointer to string
  * @retval Encryption type.
  */
extern "C" nsapi_security_t ParseSecurity(char *ptr)
{
    if (strstr(ptr, "Open")) {
        return NSAPI_SECURITY_NONE;
    } else if (strstr(ptr, "WEP")) {
        return NSAPI_SECURITY_WEP;
    } else if (strstr(ptr, "WPA2 AES")) {
        return NSAPI_SECURITY_WPA2;
    } else if (strstr(ptr, "WPA WPA2")) {
        return NSAPI_SECURITY_WPA_WPA2;
    } else if (strstr(ptr, "WPA2 TKIP")) {
        return NSAPI_SECURITY_UNKNOWN;    // no match in mbed
    } else if (strstr(ptr, "WPA2")) {
        return NSAPI_SECURITY_WPA2;    // catch any other WPA2 formula
    } else if (strstr(ptr, "WPA")) {
        return NSAPI_SECURITY_WPA;
    } else {
        return NSAPI_SECURITY_UNKNOWN;
    }
}

/**
  * @brief  Convert char in Hex format to integer.
  * @param  a: character to convert
  * @retval integer value.
  */
extern "C"  uint8_t Hex2Num(char a)
{
    if (a >= '0' && a <= '9') {                             /* Char is num */
        return a - '0';
    } else if (a >= 'a' && a <= 'f') {                      /* Char is lowercase character A - Z (hex) */
        return (a - 'a') + 10;
    } else if (a >= 'A' && a <= 'F') {                      /* Char is uppercase character A - Z (hex) */
        return (a - 'A') + 10;
    }

    return 0;
}

/**
  * @brief  Extract a hex number from a string.
  * @param  ptr: pointer to string
  * @param  cnt: pointer to the number of parsed digit
  * @retval Hex value.
  */
extern "C" uint32_t ParseHexNumber(char *ptr, uint8_t *cnt)
{
    uint32_t sum = 0;
    uint8_t i = 0;

    while (CHARISHEXNUM(*ptr)) {         /* Parse number */
        sum <<= 4;
        sum += Hex2Num(*ptr);
        ptr++;
        i++;
    }

    if (cnt != NULL) {                  /* Save number of characters used for number */
        *cnt = i;
    }
    return sum;                         /* Return number */
}

bool ISM43362::isConnected(void)
{
    return getIPAddress() != 0;
}

int ISM43362::scan(WiFiAccessPoint *res, unsigned limit)
{
    unsigned cnt = 0, num = 0;
    char *ptr;
    char tmp[256];
    bool found = false;
    bool AP_Scan_1by1 = false;

    /* Recent FW version provide new AT command: F0=2 which allows to solve the issue too many APs
      With too many APs (depending on SSID name length), there is no answer from firmware to F0 command.
      F0=2 command allows to scan AP one by one. Command MR is then used to get next AP. */
    if (_FwVersionId >= SINGLE_AP_SCAN_FW_VERSION_NUMBER) {
        AP_Scan_1by1 = true;
        if (!(_parser.send("F0=2"))) {
            debug_if(_ism_debug, "\tISM43362: F0=2 error\r\n");
            return 0;
        }
    } else {
        if (!(_parser.send("F0"))) {
            debug_if(_ism_debug, "\tISM43362: F0 error\r\n");
            return 0;
        }
    }
    debug_if(_ism_debug, "\tISM43362: scan feature %u\r\n", AP_Scan_1by1);


    /* Parse the received buffer and fill AP buffer */
    /* Use %[^\n] instead of %s to allow having spaces in the string */
    while (_parser.recv("%[^\n^\r]\r\n", tmp)) {
        found = false;
        debug_if(_ism_debug, "\tISM43362: received: %s\n", tmp);

        if (AP_Scan_1by1 == true) {
            /* In case of scan AP 1 by 1, end of list is detected thanks to OK or ERROR*/
            if (strncmp("ERROR", (char *)tmp, 5) == 0) {
                _parser.flush();
                return 0;
            }
            if ((strncmp("OK\r", (char *)tmp, 2) == 0)) {
                /* reached end */
                break;
            }
        }

        if (limit == 0 || cnt < limit) {
            nsapi_wifi_ap_t ap = {0};
            ptr = strtok(tmp, ",");
            num = 0;
            while ((ptr != NULL) && (!found)) {
                switch (num++) {
                    case 0: /* Ignore index */
                    case 4: /* Ignore Max Rate */
                    case 5: /* Ignore Network Type */
                    case 7: /* Ignore Radio Band */
                        break;
                    case 1:
                        ptr[strlen(ptr) - 1] = 0;
                        strncpy((char *)ap.ssid,  ptr + 1, 32);
                        break;
                    case 2:
                        for (int i = 0; i < 6; i++) {
                            ap.bssid[i] = ParseHexNumber(ptr + (i * 3), NULL);
                        }
                        break;
                    case 3:
                        ap.rssi = ParseNumber(ptr, NULL);
                        break;
                    case 6:
                        ap.security = ParseSecurity(ptr);
                        break;
                    case 8:
                        ap.channel = ParseNumber(ptr, NULL);
                        found = true;
                        break;
                    default:
                        break;
                }
                ptr = strtok(NULL, ",");
            }

            if (found == true) {
                if (res != NULL) {
                    res[cnt] = WiFiAccessPoint(ap);
                }
                cnt++;
            }
        } else {
            debug_if(_ism_debug, "\tISM43362: |-> discarded\r\n");
        }

        if (AP_Scan_1by1 == true) {
            _parser.flush();
            /* retrieve next AP */
            if (!(_parser.send("MR"))) {
                debug_if(_ism_debug, "\tISM43362: scan error\r\n");
                return 0;
            }
        }
    }

    /* We may stop before having read all the APs list, so flush the rest of
     * it as well as OK commands */
    _parser.flush();

    debug_if(_ism_debug, "\tISM43362: End of Scan: cnt=%d\n", cnt);

    return cnt;

}

int ISM43362::open(const char *type, int id, const char *addr, int port)
{
    static uint16_t rnglocalport = 0;

    if ((type == NULL) || (addr == NULL)) {
        debug_if(_ism_debug, "\tISM43362: parameter error\n");
        return NSAPI_ERROR_PARAMETER;
    }

    /* TODO : This is the implementation for the client socket, need to check if need to create openserver too */
    //IDs only 0-3
    if ((id < 0) || (id > 3)) {
        debug_if(_ism_debug, "\tISM43362: open: wrong id\n");
        return NSAPI_ERROR_PARAMETER;
    }

    /* Connection is either TCP or UDP */
    bool UDP_CONNECTION = false; /* TCP connection */

    if (memcmp(type, "1", sizeof("1")) == 0) {
		UDP_CONNECTION = true;
	}

    /* Set communication socket */
    _active_id = id;
    if (!(_parser.send("P0=%d", id) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: open: P0 issue\n");
        return NSAPI_ERROR_DEVICE_ERROR;
    }
    /* Set protocol */
    if (!(_parser.send("P1=%s", type) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: open: P1 issue\n");
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    /* The IANA range for ephemeral ports is 49152<96>65535. */
    /* implement automatic nr by sw because Queqtel assigns always the same initial nr */
    /* generate random local port  number between 49152 and 65535 */
    if (rnglocalport == 0) {
        /* just at first open since board reboot */
        rnglocalport = rand();
        rnglocalport = ((uint16_t)(rnglocalport & 0xFFFF) >> 2) + 49152;
    } else {
        /* from second time function execution, increment by one */
        rnglocalport += 1;
    }
    if (rnglocalport < 49152) {
        rnglocalport = 49152;
    }

    if (!UDP_CONNECTION) {/* TCP connection */

        /* Set local port */
        if (!(_parser.send("P2=%d", rnglocalport) && check_response())) {
            debug_if(_ism_debug, "\tISM43362: open: P2 issue\n");
            return NSAPI_ERROR_DEVICE_ERROR;
        }
    }

    /* Set address */
    if (!(_parser.send("P3=%s", addr) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: open: P3 issue\n");
        return NSAPI_ERROR_DEVICE_ERROR;
    }

       if (UDP_CONNECTION) {
        if (!(_parser.send("P4=%d", rnglocalport) && check_response())) {
            debug_if(_ism_debug, "\tISM43362: open: P4 issue\n");
            return NSAPI_ERROR_DEVICE_ERROR;
        }
    } else { /* TCP connection */
        if (!(_parser.send("P4=%d", port) && check_response())) {
            debug_if(_ism_debug, "\tISM43362: open: P4 issue\n");
            return NSAPI_ERROR_DEVICE_ERROR;
        }
    }

    /*  In case of UDP, force client mode ORIGIN. */
    if (UDP_CONNECTION) {
        /* Disable server */
        if (!(_parser.send("P5=0") && check_response())) {
            debug_if(_ism_debug, "\tISM43362: open: P5 issue\n");
            return NSAPI_ERROR_DEVICE_ERROR;
        }
    }

    /* Start client */
    if (!(_parser.send("P6=1") && check_response())) {
        debug_if(_ism_debug, "\tISM43362: open: P6 issue, id=%d, addr=%s\n", id, addr);
        return NSAPI_ERROR_DEVICE_ERROR;
    }

   if (UDP_CONNECTION) {
        if (!(_parser.send("P0=%d", id) && check_response())) {
            debug_if(_ism_debug, "\tISM43362: open: P0 issue\n");
            return NSAPI_ERROR_DEVICE_ERROR;
        }

        if (!(_parser.send("P4=%d", port) && check_response())) {
            debug_if(_ism_debug, "\tISM43362: open: P4 issue\n");
            return NSAPI_ERROR_DEVICE_ERROR;
        }
    }

    /* request as much data as possible - i.e. module max size */
    if (!(_parser.send("R1=%d", ES_WIFI_MAX_RX_PACKET_SIZE) && check_response())) {
        debug_if(_ism_debug, "\tISM43362: open: R1 issue\n");
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    /* Non blocking mode : set Read Transport Timeout to 1ms */
    if (!(_parser.send("R2=1") && check_response())) {
        debug_if(_ism_debug, "\tISM43362: open: R2 issue\n");
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    debug_if(_ism_debug, "\tISM43362: open ok with id %d type %s addr %s port %d\n", id, type, addr, port);

    return NSAPI_ERROR_OK;
}


bool ISM43362::send(int id, const void *data, uint32_t amount)
{
    // The Size limit has to be checked on caller side.
    if (amount > ES_WIFI_MAX_TX_PACKET_SIZE) {
        debug_if(_ism_debug, "\tISM43362 send: max issue\n");
        return false;
    }

    /* Activate the socket id in the wifi module */
    if ((id < 0) || (id > 3)) {
        return false;
    }
    if (_active_id != id) {
        _active_id = id;
        if (!(_parser.send("P0=%d", id) && check_response())) {
            debug_if(_ism_debug, "\tISM43362 send: P0 issue\n");
            return false;
        }
    }

    /* set Write Transport Packet Size */
    int i = _parser.printf("S3=%d\r", (int)amount);
    if (i < 0) {
        debug_if(_ism_debug, "\tISM43362 send: S3 issue\n");
        return false;
    }
    i = _parser.write((const char *)data, amount, i);
    if (i < 0) {
        return false;
    }

    if (!check_response()) {
        return false;
    }

    debug_if(_ism_debug, "\tISM43362 send: id %d amount %" PRIu32 "\n", id, amount);
    return true;
}

int ISM43362::check_recv_status(int id, void *data)
{
    int read_amount;

    debug_if(_ism_debug, "\tISM43362 check_recv_status: id %d\r\n", id);

    /* Activate the socket id in the wifi module */
    if ((id < 0) || (id > 3)) {
        debug_if(_ism_debug, "\tISM43362 check_recv_status: ERROR with id %d\r\n", id);
        return -1;
    }

    if (_active_id != id) {
        _active_id = id;
        if (!(_parser.send("P0=%d", id) && check_response())) {
            return -1;
        }
    }


    if (!_parser.send("R0")) {
        return -1;
    }
    read_amount = _parser.read((char *)data);

    if (read_amount < 0) {
        debug_if(_ism_debug, "\tISM43362 check_recv_status: ERROR in data RECV, timeout?\r\n");
        return -1; /* nothing to read */
    }

    /*  If there are spurious 0x15 at the end of the data, this is an error
     *  we hall can get rid off of them :-(
     *  This should not happen, but let's try to clean-up anyway
     */
    char *cleanup = (char *) data;
    while ((read_amount > 0) && (cleanup[read_amount - 1] == 0x15)) {
        // debug_if(_ism_debug, "\tISM43362 check_recv_status: spurious 0X15 trashed\r\n");
        /* Remove the trailling char then search again */
        read_amount--;
    }

    if ((read_amount >= 6) && (strncmp("OK\r\n> ", (char *)data, 6) == 0)) {
        // debug_if(_ism_debug, "\tISM43362 check_recv_status: recv 2 nothing to read=%d\r\n", read_amount);
        // read_amount -= 6;
        return 0; /* nothing to read */
    } else if ((read_amount >= 8) && (strncmp((char *)((uint32_t) data + read_amount - 8), "\r\nOK\r\n> ", 8)) == 0) {
        /* bypass ""\r\nOK\r\n> " if present at the end of the chain */
        read_amount -= 8;
    } else {
        debug_if(_ism_debug, "\tISM43362 check_recv_status: ERROR, flushing %d bytes: ", read_amount);
        // for (int i = 0; i < read_amount; i++) {
        //      debug_if(_ism_debug, "%2X ", cleanup[i]);
        // }
        // debug_if(_ism_debug, "\r\n (ASCII)", cleanup);
        cleanup[read_amount] = 0;
        debug_if(_ism_debug, "%s\r\n", cleanup);
        return -1; /* nothing to read */
    }

    debug_if(_ism_debug, "\tISM43362 check_recv_status: id %d read_amount=%d\r\n", id, read_amount);
    return read_amount;
}

bool ISM43362::close(int id)
{
    if ((id < 0) || (id > 3)) {
        debug_if(_ism_debug, "\tISM43362: Wrong socket number\n");
        return false;
    }
    /* Set connection on this socket */
    debug_if(_ism_debug, "\tISM43362: CLOSE socket id=%d\n", id);
    _active_id = id;
    if (!(_parser.send("P0=%d", id) && check_response())) {
        return false;
    }
    /* close this socket */
    if (!(_parser.send("P6=0") && check_response())) {
        return false;
    }
    return true;
}

bool ISM43362::readable()
{
    /* not applicable with SPI api */
    return true;
}

bool ISM43362::writeable()
{
    /* not applicable with SPI api */
    return true;
}

void ISM43362::attach(Callback<void()> status_cb)
{
    _conn_stat_cb = status_cb;
}

nsapi_connection_status_t ISM43362::connection_status() const
{
    debug_if(_ism_debug, "\tISM43362: connection_status %d\n", _conn_status);
    return _conn_status;
}
