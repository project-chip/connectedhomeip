#include "bct.h"

#define require_noerr(x, y)                                                                                                        \
    if (x)                                                                                                                         \
    {                                                                                                                              \
        goto y;                                                                                                                    \
    }
#define ServiceType "_http._tcp"
#define ServiceDomain "local"
#define ServicePort 12345
#define NewServiceName "New-BCT"
#define kMFiConfigFeatureAccessoryHasApp 0x01
#define kMFiConfigFeatureSupportTLV 0x04
#define LONG_TIME 100000000

static DNSServiceRef client;
static TXTRecordRef * g_txtRecord = NULL;
static int timeout                = LONG_TIME;
static uint8_t seed               = 1;

void HandleEvent(void)
{
    int err       = 0;
    int socket_id = 0, connectedSocket = 0;
    int selectResult;
    struct sockaddr_in6 sockAddr6;
    socklen_t sockAddrLen;
    fd_set fds;
    int dns_sd_fd = client ? DNSServiceRefSockFD(client) : -1;
    int nfds      = dns_sd_fd + 1;
    struct timeval tv;
    int result;
    fd_set readfds;
    int stopnow = 0;

    if (socket_id != 0)
    {
        close(socket_id);
    }

    // Setup socket parameters
    sockAddr6.sin6_family = AF_INET6;
    sockAddr6.sin6_addr   = in6addr_any;
    sockAddr6.sin6_port   = htons(ServicePort);

    // Create socket
    sockAddrLen = sizeof(struct sockaddr_in6);
    socket_id   = socket(sockAddr6.sin6_family, SOCK_STREAM, 0);
    err         = bind(socket_id, (struct sockaddr *) &sockAddr6, sizeof(struct sockaddr_in6));
    // Setup socket parameters
    if (err == 0)
    {
        err = listen(socket_id, 1);
        FD_ZERO(&fds);
        FD_SET(socket_id, &fds);
    }
    else
    {
        mdnslogInfo("ERROR: bind socket %d  failed\n", socket_id);
    }
    selectResult = select(socket_id + 1, &fds, &fds, NULL, (struct timeval *) &timeout);
    switch (selectResult)
    {
    case -1:
        mdnslogInfo("select error\n");
        break;
    case 0:
        break;
    default:
        if (FD_ISSET(socket_id, &fds))
        {
            connectedSocket = accept(socket_id, (struct sockaddr *) &sockAddr6, &sockAddrLen);
            mdnslogInfo("accept connectedSocket=%d\n", connectedSocket);
            vTaskDelay(3 * 1000 / portTICK_RATE_MS);
            close(connectedSocket);
            mdnslogInfo("close connectedSocket=%d\n", connectedSocket);
        }
        break;
    }
    while (!stopnow)
    {
        FD_ZERO(&readfds);
        if (client)
            FD_SET(dns_sd_fd, &readfds);
        tv.tv_sec  = timeout;
        tv.tv_usec = 0;
        result     = select(nfds, &readfds, (fd_set *) NULL, (fd_set *) NULL, &tv);
        if (result > 0)
        {
            DNSServiceErrorType error = kDNSServiceErr_NoError;
            if (client && FD_ISSET(dns_sd_fd, &readfds))
            {
                error = DNSServiceProcessResult(client);
            }
            if (error)
            {
                mdnslogInfo("DNSServiceProcessResult returned %d\n", error);
                stopnow = 1;
            }
        }
        else
        {
            mdnslogInfo("select() returned %d errno %d %s\n", result, errno, strerror(errno));
            if (errno != EINTR)
            {
                stopnow = 1;
            }
        }
    }
}
int RegisterBonjourService(char * servicename, char * type)
{
    DNSServiceErrorType err;
    DNSServiceFlags flags = 0;
    TXTRecordRef txtRecord;
    uint8_t feature = kMFiConfigFeatureSupportTLV;
    char seedString[16];
    char featureString[16];
    char * sourceVersionString = "1.0";
    char mac_addr[64];
    int sz = sizeof(mac_addr);

    mdnslogInfo("service name is %s\n", servicename);
    mdnslogInfo("type name is %s\n", type);

    // get MAC Address
    os_memset(mac_addr, 0, 64);
    nvdm_read_data_item("STA", "MacAddr", (uint8_t *) mac_addr, (uint32_t *) &sz);
    mdnslogInfo("mac_addr is %s\n", mac_addr);

    // get seed string
    os_memset(seedString, 0, 16);
    os_snprintf(seedString, 16, "%d", seed);
    mdnslogInfo("seedString is %s\n", seedString);

    // get feature string
    feature += kMFiConfigFeatureAccessoryHasApp;
    os_memset(featureString, 0, 16);
    os_snprintf(featureString, 16, "%d", feature);
    // Create TXT Record
    TXTRecordCreate(&txtRecord, 0, NULL);

    // Add Device ID (MAC Address)
    err = TXTRecordSetValue(&txtRecord, "deviceid", os_strlen(mac_addr), mac_addr);
    require_noerr(err, exit);

    // Add seed ID to the bonjour TXT record
    err = TXTRecordSetValue(&txtRecord, "seed", os_strlen(seedString), seedString);
    require_noerr(err, exit);

    // Add feature bit mask to the bonjour TXT record
    err = TXTRecordSetValue(&txtRecord, "feature", os_strlen(featureString), featureString);
    require_noerr(err, exit);

    // Add source version to the bonjour TXT record
    err = TXTRecordSetValue(&txtRecord, "srcver", os_strlen(sourceVersionString), sourceVersionString);
    require_noerr(err, exit);

    // Test more mDNS TXTRecord API
    /*
    int ret = TXTRecordContainsKey(TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord), "srcver");
    mdnslogInfo("TXTRecordContainsKey srcver: %s\n", ret == 1 ? "yes" : "no");
    if (ret == 1) {
        uint8_t valueLen = 0;
        const void * valueResult = TXTRecordGetValuePtr(TXTRecordGetLength(&txtRecord),
                TXTRecordGetBytesPtr(&txtRecord), "srcver", &valueLen);
        char* ss[20] = {0};
        os_strncpy(ss, valueResult, valueLen);
        mdnslogInfo("TXTRecordGetValuePtr [srcver] value: %s %d %d\n",
                ss, os_strlen(ss), valueLen);
        DNSServiceErrorType error = TXTRecordRemoveValue(&txtRecord, "srcver");
        mdnslogInfo("TXTRecordRemoveValue srcver %d", error);
        uint16_t count = TXTRecordGetCount(TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord));
        mdnslogInfo("TXTRecordGetCount count %d", count);
        for (int i = 0; i < count; i++) {
            char key[10];
            uint8_t valueLen = 0;
            const void *value = NULL;
            DNSServiceErrorType error1 = TXTRecordGetItemAtIndex(
                    TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord),
                    i, 10, key, &valueLen, &value);
            char* ss[20] = {0};
            os_strncpy(ss, value, valueLen);
            mdnslogInfo("TXTRecordGetItemAtIndex index=%d key=%s[%d] value=%s[%d]",
                    i, key, os_strlen(key), ss, valueLen);
        }
    }
    */

    mdnslogInfo("Register Bonjour Service: %s type: %s domain: %s port: %d\n", servicename, ServiceType, ServiceDomain,
                ServicePort);
    // Register Bonjour Service
    err = DNSServiceRegister(&client,                          // DNSServiceRef
                             flags,                            // DNSServiceFlags
                             kDNSServiceInterfaceIndexAny,     // interface index
                             servicename, type,                // service name
                             ServiceDomain,                    // domain
                             NULL,                             // host
                             htons(ServicePort),               // port
                             TXTRecordGetLength(&txtRecord),   // txt record length
                             TXTRecordGetBytesPtr(&txtRecord), // txt record pointer
                             NULL,                             // callback
                             NULL);                            // context

    require_noerr(err, exit);
    if (!err)
    {
        mdnslogInfo("Register Bonjour Service successfully!\n");
        g_txtRecord = &txtRecord;
    }
    else
    {
        mdnslogInfo("Register Bonjour Service failed.\n");
    }
    HandleEvent();
    seed++;
    return err;
exit:
    TXTRecordDeallocate(&txtRecord);
    return err;
}

int RemoveBonjourService()
{
    int err = -1;
    DNSServiceRefDeallocate(client);
    client = NULL;
    err    = 0;
    return err;
}
void reg_service_cmd(char ** argv)
{
    int err = 0;
    char service_name[128];
    char type_name[128];

    os_memset(service_name, 0, 128);
    os_strncpy(service_name, argv[0], os_strlen(argv[0]));

    os_memset(type_name, 0, 128);
    os_strncpy(type_name, argv[1], os_strlen(argv[1]));

    err = RegisterBonjourService(service_name, type_name);
    if (!err)
    {
        mdnslogInfo("ERROR: Register Bonjour Service %s.%s failed\n", service_name, type_name);
    }
    else
    {
        mdnslogInfo("Register Bonjour Service %s.%s succeeded\n", service_name, type_name);
    }
}

/*
static DNSRecordRef hinfo_recRef = NULL;
static DNSRecordRef testTXT_recRef = NULL;
static char hinfoW[13] = "\003ARM\010FreeRTOS";
static char testTXT[6] = "\005aa=bb";
*/
void rename_service_cmd(char ** argv)
{
    // Test DNSServiceAddRecord/DNSServiceRemoveRecord
    /*
    if (g_txtRecord != NULL) {
      int ret = TXTRecordContainsKey(TXTRecordGetLength(g_txtRecord), TXTRecordGetBytesPtr(g_txtRecord), "seed");
        mdnslogInfo("~~~~~~~~~~TXTRecordContainsKey seed: %s\n", ret == 1 ? "yes" : "no");
        if (ret == 1) {
            // need DNSServiceUpdateRecord after TXTRecordRemoveValue
            DNSServiceErrorType error = TXTRecordRemoveValue(g_txtRecord, "seed");
            mdnslogInfo("~~~~~~~~~~TXTRecordRemoveValue seed %d", error);
        }
        DNSServiceErrorType err = DNSServiceUpdateRecord(client,               // DNSServiceRef
                                     NULL,                                     // DNSRecordRef
                                     0,                                        // DNSServiceFlags
                                     TXTRecordGetLength(g_txtRecord),          // txt record length
                                     TXTRecordGetBytesPtr(g_txtRecord),        // txt record pointer
                                     0                                         // ttl
                                    );
        mdnslogInfo("~~~~~~~~~~DNSServiceUpdateRecord err=%d", err);

        DNSServiceErrorType err = DNSServiceAddRecord(client, &hinfo_recRef, 0, kDNSServiceType_HINFO, sizeof(hinfoW), &hinfoW[0],
    0); mdnslogInfo("~~~~~~~~~~DNSServiceAddRecord hinfo err=%d RecordRef=%d", err, hinfo_recRef); err = DNSServiceAddRecord(client,
    &testTXT_recRef, 0, kDNSServiceType_TXT, sizeof(testTXT), &testTXT[0], 0); mdnslogInfo("~~~~~~~~~~DNSServiceAddRecord chip
    err=%d RecordRef=%d", err, testTXT_recRef); g_txtRecord = NULL;
        // No need to DNSServiceUpdateRecord after DNSServiceAddRecord
        // err = DNSServiceUpdateRecord(client,  NULL,  0,  sizeof(chip), &chip[0], 0);
        // mdnslogInfo("~~~~~~~~~~DNSServiceUpdateRecord err=%d", err);
    } else {
        if (testTXT_recRef != NULL) {
            DNSServiceErrorType err = DNSServiceRemoveRecord(client, testTXT_recRef, 0);
            mdnslogInfo("~~~~~~~~~~DNSServiceRemoveRecord remove_testTXT_record err=%d", err);
        }
    }
    return;
    */

    int err = 0;
    err     = RemoveBonjourService();
    if (!err)
    {
        mdnslogInfo("Remove Bonjour Service failed\n");
    }
    if (argv && argv[0])
    {
        mdnslogInfo("new bonjour service name is %s\n", argv[0]);
        err = RegisterBonjourService(argv[0], ServiceType);
    }
    else
    {
        err = RegisterBonjourService(NewServiceName, ServiceType);
    }
    if (!err)
    {
        mdnslogInfo("Register Bonjour Service failed\n");
    }
}
