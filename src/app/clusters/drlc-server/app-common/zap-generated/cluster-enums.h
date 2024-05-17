namespace DemandResponseLoadControl {

// Enum for CriticalityLevelEnum
enum class CriticalityLevelEnum : uint8_t
{
    kUnknown           = 0x00,
    kGreen             = 0x01,
    kLevel1            = 0x02,
    kLevel2            = 0x03,
    kLevel3            = 0x04,
    kLevel4            = 0x05,
    kLevel5            = 0x06,
    kEmergency         = 0x07,
    kPlannedOutage     = 0x08,
    kServiceDisconnect = 0x09,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 10,
};

// Enum for HeatingSourceEnum
enum class HeatingSourceEnum : uint8_t
{
    kAny         = 0x00,
    kElectric    = 0x01,
    kNonElectric = 0x02,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 3,
};

// Enum for LoadControlEventChangeSourceEnum
enum class LoadControlEventChangeSourceEnum : uint8_t
{
    kAutomatic  = 0x00,
    kUserAction = 0x01,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 2,
};

// Enum for LoadControlEventStatusEnum
enum class LoadControlEventStatusEnum : uint8_t
{
    kUnknown         = 0x00,
    kReceived        = 0x01,
    kInProgress      = 0x02,
    kCompleted       = 0x03,
    kOptedOut        = 0x04,
    kOptedIn         = 0x05,
    kCanceled        = 0x06,
    kSuperseded      = 0x07,
    kPartialOptedOut = 0x08,
    kPartialOptedIn  = 0x09,
    kNoParticipation = 0x0A,
    kUnavailable     = 0x0B,
    kFailed          = 0x0C,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 13,
};

// Enum for PowerSavingsEnum
enum class PowerSavingsEnum : uint8_t
{
    kLow    = 0x00,
    kMedium = 0x01,
    kHigh   = 0x02,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 3,
};

// Bitmap for CancelControlBitmap
enum class CancelControlBitmap : uint16_t
{
    kRandomEnd = 0x1,
};

// Bitmap for DeviceClassBitmap
enum class DeviceClassBitmap : uint32_t
{
    kHvac             = 0x1,
    kStripHeater      = 0x2,
    kWaterHeater      = 0x4,
    kPoolPump         = 0x8,
    kSmartAppliance   = 0x10,
    kIrrigationPump   = 0x20,
    kCommercialLoad   = 0x40,
    kResidentialLoad  = 0x80,
    kExteriorLighting = 0x100,
    kInteriorLighting = 0x200,
    kEv               = 0x400,
    kGenerationSystem = 0x800,
    kSmartInverter    = 0x1000,
    kEvse             = 0x2000,
    kResu             = 0x4000,
    kEms              = 0x8000,
    kSem              = 0x10000,
};

// Bitmap for EventControlBitmap
enum class EventControlBitmap : uint16_t
{
    kRandomStart = 0x1,
};

// Bitmap for EventTransitionControlBitmap
enum class EventTransitionControlBitmap : uint16_t
{
    kRandomDuration = 0x1,
    kIgnoreOptOut   = 0x2,
};

// Bitmap for Feature
enum class Feature : uint32_t
{
    kEnrollmentGroups    = 0x1,
    kTemperatureOffset   = 0x2,
    kTemperatureSetpoint = 0x4,
    kLoadAdjustment      = 0x8,
    kDutyCycle           = 0x10,
    kPowerSavings        = 0x20,
    kHeatingSource       = 0x40,
};
} // namespace DemandResponseLoadControl
