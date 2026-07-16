#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace chip {

class ChipError
{
public:
    explicit constexpr ChipError(int value = 0) : mValue(value) {}

    constexpr bool operator==(const ChipError & other) const { return mValue == other.mValue; }
    constexpr bool operator!=(const ChipError & other) const { return mValue != other.mValue; }
    constexpr operator int() const { return mValue; }
    constexpr const char * Format() const { return "stub"; }

    static constexpr bool IsSuccess(const ChipError & error) { return error.mValue == 0; }

private:
    int mValue;
};

using CHIP_ERROR = ChipError;

inline constexpr CHIP_ERROR CHIP_NO_ERROR(0);
inline constexpr CHIP_ERROR CHIP_ERROR_INTERNAL(1);
inline constexpr CHIP_ERROR CHIP_ERROR_INCORRECT_STATE(2);
inline constexpr CHIP_ERROR CHIP_ERROR_INVALID_ARGUMENT(3);
inline constexpr CHIP_ERROR CHIP_ERROR_BUFFER_TOO_SMALL(4);
inline constexpr CHIP_ERROR CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE(5);
inline constexpr CHIP_ERROR CHIP_ERROR_READ_FAILED(6);

class ByteSpan
{
public:
    ByteSpan(const uint8_t * data, size_t size) : mData(data), mSize(size) {}
    const uint8_t * data() const { return mData; }
    size_t size() const { return mSize; }

private:
    const uint8_t * mData;
    size_t mSize;
};

class MutableByteSpan
{
public:
    MutableByteSpan(uint8_t * data, size_t size) : mData(data), mSize(size) {}
    uint8_t * data() const { return mData; }
    size_t size() const { return mSize; }
    void reduce_size(size_t size) { mSize = size; }

private:
    uint8_t * mData;
    size_t mSize;
};

class MutableCharSpan
{
public:
    MutableCharSpan(char * data, size_t size) : mData(data), mSize(size) {}
    char * data() const { return mData; }
    size_t size() const { return mSize; }
    void reduce_size(size_t size) { mSize = size; }

private:
    char * mData;
    size_t mSize;
};

class CharSpan
{
public:
    CharSpan() : mData(nullptr), mSize(0) {}
    CharSpan(const char * data, size_t size) : mData(data), mSize(size) {}
    const char * data() const { return mData; }
    size_t size() const { return mSize; }

private:
    const char * mData;
    size_t mSize;
};

namespace Platform {

inline void CopyString(char * dest, size_t destLength, const char * source)
{
    if (dest == nullptr || destLength == 0)
    {
        return;
    }

    if (source == nullptr)
    {
        dest[0] = '\0';
        return;
    }

    std::strncpy(dest, source, destLength);
    dest[destLength - 1] = '\0';
}

template <size_t N>
inline void CopyString(char (&dest)[N], const char * source)
{
    CopyString(dest, N, source);
}

inline void CopyString(char * dest, size_t destLength, ByteSpan source)
{
    if (dest == nullptr || destLength == 0)
    {
        return;
    }

    if (source.data() == nullptr)
    {
        dest[0] = '\0';
        return;
    }

    const size_t copyLen = source.size() < destLength - 1 ? source.size() : destLength - 1;
    if (copyLen > 0)
    {
        std::memcpy(dest, source.data(), copyLen);
    }
    dest[copyLen] = '\0';
}

template <size_t N>
inline void CopyString(char (&dest)[N], ByteSpan source)
{
    CopyString(dest, N, source);
}

inline void CopyString(char * dest, size_t destLength, CharSpan source)
{
    if (dest == nullptr || destLength == 0)
    {
        return;
    }

    if (source.data() == nullptr)
    {
        dest[0] = '\0';
        return;
    }

    const size_t copyLen = source.size() < destLength - 1 ? source.size() : destLength - 1;
    if (copyLen > 0)
    {
        std::memcpy(dest, source.data(), copyLen);
    }
    dest[copyLen] = '\0';
}

template <size_t N>
inline void CopyString(char (&dest)[N], CharSpan source)
{
    CopyString(dest, N, source);
}

} // namespace Platform

namespace Ble {

inline constexpr char CHIP_BLE_SERVICE_SHORT_UUID_STR[] = "fff6";
inline constexpr char CHIP_BLE_SERVICE_LONG_UUID_STR[]  = "0000fff6-0000-1000-8000-00805f9b34fb";

struct ChipBLEDeviceIdentificationInfo
{
    uint8_t OpCode;
    uint8_t DeviceDiscriminatorAndAdvVersion[2];
    uint8_t DeviceVendorId[2];
    uint8_t DeviceProductId[2];
    uint8_t AdditionalDataFlag;

    void Init() { std::memset(this, 0, sizeof(*this)); }
} __attribute__((packed));

} // namespace Ble

namespace DeviceLayer {

class PlatformManager
{
public:
    void LockChipStack() {}
    void UnlockChipStack() {}
};

inline PlatformManager & PlatformMgr()
{
    static PlatformManager manager;
    return manager;
}

class PlatformManagerImpl
{
public:
    template <typename T>
    CHIP_ERROR GLibMatterContextInvokeSync(CHIP_ERROR (*callback)(T *), T * userData)
    {
        return callback(userData);
    }
};

inline PlatformManagerImpl & PlatformMgrImpl()
{
    static PlatformManagerImpl manager;
    return manager;
}

namespace Internal {

inline constexpr size_t kMaxWiFiSSIDLength = 32;
inline constexpr size_t kMaxWiFiKeyLength  = 64;
inline constexpr size_t kWiFiBSSIDLength   = 6;

} // namespace Internal

namespace NetworkCommissioning {

enum class Status
{
    kSuccess,
    kUnknownError,
    kOutOfRange,
    kBoundsExceeded,
    kNetworkIDNotFound,
    kNetworkNotFound,
};

enum class WiFiBand
{
    k1g,
    k2g4,
    k3g65,
    k5g,
    k6g,
    k60g,
};

enum class WirelessSignalType
{
    kdBm,
};

class SecurityBitmap
{
public:
    void SetRaw(uint8_t raw) { mRaw = raw; }

private:
    uint8_t mRaw = 0;
};

struct WirelessSignal
{
    WirelessSignalType type = WirelessSignalType::kdBm;
    int8_t strength         = 0;
};

struct WiFiScanResponse
{
    uint8_t ssid[32] = {};
    uint8_t ssidLen  = 0;
    uint8_t bssid[6] = {};
    WirelessSignal signal;
    SecurityBitmap security;
    WiFiBand wiFiBand = WiFiBand::k2g4;
    uint16_t channel  = 0;
};

struct Network
{
    uint8_t networkID[32] = {};
    uint8_t networkIDLen  = 0;
    bool connected        = false;
};

template <typename T>
class Iterator
{
public:
    virtual ~Iterator()         = default;
    virtual size_t Count()      = 0;
    virtual bool Next(T & item) = 0;
    virtual void Release()      = 0;
};

using NetworkIterator = Iterator<Network>;

class BaseDriver
{
public:
    class NetworkStatusChangeCallback
    {
    public:
        virtual ~NetworkStatusChangeCallback() = default;
    };

    virtual ~BaseDriver() = default;
    virtual CHIP_ERROR Init(NetworkStatusChangeCallback *) { return CHIP_NO_ERROR; }
    virtual uint8_t GetMaxNetworks() { return 0; }
    virtual NetworkIterator * GetNetworks() { return nullptr; }
};

namespace Internal {

class WirelessDriver : public BaseDriver
{
public:
    class ConnectCallback
    {
    public:
        virtual ~ConnectCallback() = default;
        virtual void OnResult(Status, CharSpan, int32_t) {}
    };

    virtual uint8_t GetScanNetworkTimeoutSeconds() { return 0; }
    virtual uint8_t GetConnectNetworkTimeoutSeconds() { return 0; }
    virtual CHIP_ERROR CommitConfiguration() { return CHIP_NO_ERROR; }
    virtual CHIP_ERROR RevertConfiguration() { return CHIP_NO_ERROR; }
    virtual Status RemoveNetwork(ByteSpan, MutableCharSpan &, uint8_t &) { return Status::kUnknownError; }
    virtual Status ReorderNetwork(ByteSpan, uint8_t, MutableCharSpan &) { return Status::kUnknownError; }
    virtual void ConnectNetwork(ByteSpan, ConnectCallback *) {}
};

} // namespace Internal

class WiFiDriver : public Internal::WirelessDriver
{
public:
    class ScanCallback
    {
    public:
        virtual ~ScanCallback() = default;
        virtual void OnFinished(Status, CharSpan, Iterator<WiFiScanResponse> *) {}
    };

    virtual Status AddOrUpdateNetwork(ByteSpan, ByteSpan, MutableCharSpan &, uint8_t &) { return Status::kUnknownError; }
    virtual void ScanNetworks(ByteSpan, ScanCallback *) {}
    virtual uint32_t GetSupportedWiFiBandsMask() const { return 0; }
};

class EthernetDriver : public BaseDriver
{
public:
    virtual ~EthernetDriver() = default;
    virtual uint8_t GetMaxNetworks() { return 0; }
    virtual NetworkIterator * GetNetworks() { return nullptr; }
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip

using CHIP_ERROR = chip::CHIP_ERROR;

inline constexpr CHIP_ERROR CHIP_NO_ERROR                       = chip::CHIP_NO_ERROR;
inline constexpr CHIP_ERROR CHIP_ERROR_INTERNAL                 = chip::CHIP_ERROR_INTERNAL;
inline constexpr CHIP_ERROR CHIP_ERROR_INCORRECT_STATE          = chip::CHIP_ERROR_INCORRECT_STATE;
inline constexpr CHIP_ERROR CHIP_ERROR_INVALID_ARGUMENT         = chip::CHIP_ERROR_INVALID_ARGUMENT;
inline constexpr CHIP_ERROR CHIP_ERROR_BUFFER_TOO_SMALL         = chip::CHIP_ERROR_BUFFER_TOO_SMALL;
inline constexpr CHIP_ERROR CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE = chip::CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
inline constexpr CHIP_ERROR CHIP_ERROR_READ_FAILED              = chip::CHIP_ERROR_READ_FAILED;

#define CHIP_ERROR_FORMAT "s"
#define MATTER_PLATFORM_ERROR(error) CHIP_ERROR(error)

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0
#define CHIP_DEVICE_CONFIG_THREAD_BORDER_ROUTER 0
#define CHIP_DEVICE_CONFIG_THREAD_FTD 0
#define CHIP_DEVICE_CONFIG_THREAD_SSED 0

#define TEMPORARY_RETURN_IGNORED

#define VerifyOrReturn(condition, ...)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(condition))                                                                                                          \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

#define VerifyOrExit(condition, ...)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(condition))                                                                                                          \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            goto exit;                                                                                                             \
        }                                                                                                                          \
    } while (false)

#define VerifyOrReturnValue(condition, value, ...)                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(condition))                                                                                                          \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return (value);                                                                                                        \
        }                                                                                                                          \
    } while (false)

#define VerifyOrReturnError(condition, error, ...)                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(condition))                                                                                                          \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return (error);                                                                                                        \
        }                                                                                                                          \
    } while (false)

#define ReturnErrorOnFailure(expression, ...)                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __chip_error = (expression);                                                                                          \
        if (!::chip::ChipError::IsSuccess(__chip_error))                                                                           \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return __chip_error;                                                                                                   \
        }                                                                                                                          \
    } while (false)

#define SuccessOrExit(expression)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!::chip::ChipError::IsSuccess(expression))                                                                             \
        {                                                                                                                          \
            goto exit;                                                                                                             \
        }                                                                                                                          \
    } while (false)

#define ExitNow(...)                                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        __VA_ARGS__;                                                                                                               \
        goto exit;                                                                                                                 \
    } while (false)

#define DeviceLayer DeviceLayer
#define Ble Ble

#define ChipLogDetail(module, format, ...) static_cast<void>(0)
#define ChipLogProgress(module, format, ...) static_cast<void>(0)
#define ChipLogError(module, format, ...) static_cast<void>(0)
#define ChipLogByteSpan(module, span) static_cast<void>(0)

template <typename T>
class GAutoPtr
{
public:
    GAutoPtr() = default;
    ~GAutoPtr() { std::free(mPtr); }

    T * get() const { return mPtr; }
    T *& GetReceiver()
    {
        std::free(mPtr);
        mPtr = nullptr;
        return mPtr;
    }

private:
    T * mPtr = nullptr;
};
