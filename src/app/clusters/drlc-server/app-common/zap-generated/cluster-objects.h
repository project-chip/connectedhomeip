namespace DemandResponseLoadControl {
namespace Structs {
namespace HeatingSourceControlStruct {
enum class Fields : uint8_t
{
    kHeatingSource = 0,
};

struct Type
{
public:
    HeatingSourceEnum heatingSource = static_cast<HeatingSourceEnum>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace HeatingSourceControlStruct
namespace PowerSavingsControlStruct {
enum class Fields : uint8_t
{
    kPowerSavings = 0,
};

struct Type
{
public:
    PowerSavingsEnum powerSavings = static_cast<PowerSavingsEnum>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace PowerSavingsControlStruct
namespace DutyCycleControlStruct {
enum class Fields : uint8_t
{
    kDutyCycle = 0,
};

struct Type
{
public:
    chip::Percent dutyCycle = static_cast<chip::Percent>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace DutyCycleControlStruct
namespace AverageLoadControlStruct {
enum class Fields : uint8_t
{
    kLoadAdjustment = 0,
};

struct Type
{
public:
    int8_t loadAdjustment = static_cast<int8_t>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace AverageLoadControlStruct
namespace TemperatureControlStruct {
enum class Fields : uint8_t
{
    kCoolingTempOffset   = 0,
    kHeatingtTempOffset  = 1,
    kCoolingTempSetpoint = 2,
    kHeatingTempSetpoint = 3,
};

struct Type
{
public:
    Optional<DataModel::Nullable<uint16_t>> coolingTempOffset;
    Optional<DataModel::Nullable<uint16_t>> heatingtTempOffset;
    Optional<DataModel::Nullable<int16_t>> coolingTempSetpoint;
    Optional<DataModel::Nullable<int16_t>> heatingTempSetpoint;

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace TemperatureControlStruct
namespace LoadControlEventTransitionStruct {
enum class Fields : uint8_t
{
    kDuration             = 0,
    kControl              = 1,
    kTemperatureControl   = 2,
    kAverageLoadControl   = 3,
    kDutyCycleControl     = 4,
    kPowerSavingsControl  = 5,
    kHeatingSourceControl = 6,
};

struct Type
{
public:
    uint16_t duration                                   = static_cast<uint16_t>(0);
    chip::BitMask<EventTransitionControlBitmap> control = static_cast<chip::BitMask<EventTransitionControlBitmap>>(0);
    Optional<Structs::TemperatureControlStruct::Type> temperatureControl;
    Optional<Structs::AverageLoadControlStruct::Type> averageLoadControl;
    Optional<Structs::DutyCycleControlStruct::Type> dutyCycleControl;
    Optional<Structs::PowerSavingsControlStruct::Type> powerSavingsControl;
    Optional<Structs::HeatingSourceControlStruct::Type> heatingSourceControl;

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace LoadControlEventTransitionStruct
namespace LoadControlEventStruct {
enum class Fields : uint8_t
{
    kEventID         = 0,
    kProgramID       = 1,
    kControl         = 2,
    kDeviceClass     = 3,
    kEnrollmentGroup = 4,
    kCriticality     = 5,
    kStartTime       = 6,
    kTransitions     = 7,
};

struct Type
{
public:
    chip::ByteSpan eventID;
    DataModel::Nullable<chip::ByteSpan> programID;
    chip::BitMask<EventControlBitmap> control    = static_cast<chip::BitMask<EventControlBitmap>>(0);
    chip::BitMask<DeviceClassBitmap> deviceClass = static_cast<chip::BitMask<DeviceClassBitmap>>(0);
    Optional<uint8_t> enrollmentGroup;
    CriticalityLevelEnum criticality = static_cast<CriticalityLevelEnum>(0);
    DataModel::Nullable<uint32_t> startTime;
    DataModel::List<const Structs::LoadControlEventTransitionStruct::Type> transitions;

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

struct DecodableType
{
public:
    chip::ByteSpan eventID;
    DataModel::Nullable<chip::ByteSpan> programID;
    chip::BitMask<EventControlBitmap> control    = static_cast<chip::BitMask<EventControlBitmap>>(0);
    chip::BitMask<DeviceClassBitmap> deviceClass = static_cast<chip::BitMask<DeviceClassBitmap>>(0);
    Optional<uint8_t> enrollmentGroup;
    CriticalityLevelEnum criticality = static_cast<CriticalityLevelEnum>(0);
    DataModel::Nullable<uint32_t> startTime;
    DataModel::DecodableList<Structs::LoadControlEventTransitionStruct::DecodableType> transitions;

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;
};
static constexpr bool kInUse = false;

LoadControlEventStruct& operator=(const LoadControlEventsStruct& other) 
{
    if( this == &other ) return *this;

    eventID = ByteSpan(other.eventID);
    programID = ByteSpan(other.programID);
    control = other.control;
    deviceClass = other.devieClass;
    enrollmentGroup = other.enrollmentGroup;
    criticality = other.criticality;
    startTime = other.startTime;
    transitions = other.transitions;

    kIsFabricScoped = other.kIsFabricScoped;
    kInUse = other.kInUse;
}


} // namespace LoadControlEventStruct
namespace LoadControlProgramStruct {
enum class Fields : uint8_t
{
    kProgramID             = 0,
    kName                  = 1,
    kEnrollmentGroup       = 2,
    kRandomStartMinutes    = 3,
    kRandomDurationMinutes = 4,
};

struct Type
{
public:
    chip::ByteSpan programID;
    chip::CharSpan name;
    DataModel::Nullable<uint8_t> enrollmentGroup;
    DataModel::Nullable<uint8_t> randomStartMinutes;
    DataModel::Nullable<uint8_t> randomDurationMinutes;

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

static constexpr bool kInUse = false;

LoadControlProgramStruct& operator=(const LoadControlProgramsStruct& other) 
{
    if( this == &other ) return *this;

    free(programID);
    programID = strdup(other.programID);
    free(name);
    name = strdup(other.name);
    enrollmentGroup = other.enrollmentGroup;
    randomStartMinutes = other.randomStartMinutes;
    randomDurationMinutes = other.randomDurationMinutes;
    kIsFabricScoped = other.kIsFabricScoped;
    kInUse = other.kInUse;
}

} // namespace LoadControlProgramStruct
} // namespace Structs

namespace Commands {
// Forward-declarations so we can reference these later.

namespace RegisterLoadControlProgramRequest {
struct Type;
struct DecodableType;
} // namespace RegisterLoadControlProgramRequest

namespace UnregisterLoadControlProgramRequest {
struct Type;
struct DecodableType;
} // namespace UnregisterLoadControlProgramRequest

namespace AddLoadControlEventRequest {
struct Type;
struct DecodableType;
} // namespace AddLoadControlEventRequest

namespace RemoveLoadControlEventRequest {
struct Type;
struct DecodableType;
} // namespace RemoveLoadControlEventRequest

namespace ClearLoadControlEventsRequest {
struct Type;
struct DecodableType;
} // namespace ClearLoadControlEventsRequest

} // namespace Commands

namespace Commands {
namespace RegisterLoadControlProgramRequest {
enum class Fields : uint8_t
{
    kLoadControlProgram = 0,
};

struct Type
{
public:
    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::RegisterLoadControlProgramRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    Structs::LoadControlProgramStruct::Type loadControlProgram;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

    using ResponseType = DataModel::NullObjectType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::RegisterLoadControlProgramRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    Structs::LoadControlProgramStruct::DecodableType loadControlProgram;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace RegisterLoadControlProgramRequest
namespace UnregisterLoadControlProgramRequest {
enum class Fields : uint8_t
{
    kLoadControlProgramID = 0,
};

struct Type
{
public:
    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::UnregisterLoadControlProgramRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    chip::ByteSpan loadControlProgramID;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

    using ResponseType = DataModel::NullObjectType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::UnregisterLoadControlProgramRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    chip::ByteSpan loadControlProgramID;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace UnregisterLoadControlProgramRequest
namespace AddLoadControlEventRequest {
enum class Fields : uint8_t
{
    kEvent = 0,
};

struct Type
{
public:
    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::AddLoadControlEventRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    Structs::LoadControlEventStruct::Type event;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

    using ResponseType = DataModel::NullObjectType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::AddLoadControlEventRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    Structs::LoadControlEventStruct::DecodableType event;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace AddLoadControlEventRequest
namespace RemoveLoadControlEventRequest {
enum class Fields : uint8_t
{
    kEventID       = 0,
    kCancelControl = 1,
};

struct Type
{
public:
    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::RemoveLoadControlEventRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    chip::ByteSpan eventID;
    chip::BitMask<CancelControlBitmap> cancelControl = static_cast<chip::BitMask<CancelControlBitmap>>(0);

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

    using ResponseType = DataModel::NullObjectType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::RemoveLoadControlEventRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    chip::ByteSpan eventID;
    chip::BitMask<CancelControlBitmap> cancelControl = static_cast<chip::BitMask<CancelControlBitmap>>(0);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace RemoveLoadControlEventRequest
namespace ClearLoadControlEventsRequest {
enum class Fields : uint8_t
{
};

struct Type
{
public:
    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::ClearLoadControlEventsRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

    using ResponseType = DataModel::NullObjectType;

    static constexpr bool MustUseTimedInvoke() { return false; }
};

struct DecodableType
{
public:
    static constexpr CommandId GetCommandId() { return Commands::ClearLoadControlEventsRequest::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
}; // namespace ClearLoadControlEventsRequest
} // namespace Commands

namespace Attributes {

namespace DeviceClass {
struct TypeInfo
{
    using Type             = chip::BitMask<chip::app::Clusters::DemandResponseLoadControl::DeviceClassBitmap>;
    using DecodableType    = chip::BitMask<chip::app::Clusters::DemandResponseLoadControl::DeviceClassBitmap>;
    using DecodableArgType = chip::BitMask<chip::app::Clusters::DemandResponseLoadControl::DeviceClassBitmap>;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::DeviceClass::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace DeviceClass
namespace LoadControlPrograms {
struct TypeInfo
{
    using Type =
        chip::app::DataModel::List<const chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlProgramStruct::Type>;
    using DecodableType = chip::app::DataModel::DecodableList<
        chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlProgramStruct::DecodableType>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<
        chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlProgramStruct::DecodableType> &;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::LoadControlPrograms::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace LoadControlPrograms
namespace NumberOfLoadControlPrograms {
struct TypeInfo
{
    using Type             = uint8_t;
    using DecodableType    = uint8_t;
    using DecodableArgType = uint8_t;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::NumberOfLoadControlPrograms::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace NumberOfLoadControlPrograms
namespace Events {
struct TypeInfo
{
    using Type =
        chip::app::DataModel::List<const chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::Type>;
    using DecodableType = chip::app::DataModel::DecodableList<
        chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::DecodableType>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<
        chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::DecodableType> &;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::Events::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace Events
namespace ActiveEvents {
struct TypeInfo
{
    using Type =
        chip::app::DataModel::List<const chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::Type>;
    using DecodableType = chip::app::DataModel::DecodableList<
        chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::DecodableType>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<
        chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::DecodableType> &;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::ActiveEvents::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace ActiveEvents
namespace NumberOfEventsPerProgram {
struct TypeInfo
{
    using Type             = uint8_t;
    using DecodableType    = uint8_t;
    using DecodableArgType = uint8_t;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::NumberOfEventsPerProgram::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace NumberOfEventsPerProgram
namespace NumberOfTransistions {
struct TypeInfo
{
    using Type             = uint8_t;
    using DecodableType    = uint8_t;
    using DecodableArgType = uint8_t;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::NumberOfTransistions::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace NumberOfTransistions
namespace DefaultRandomStart {
struct TypeInfo
{
    using Type             = uint8_t;
    using DecodableType    = uint8_t;
    using DecodableArgType = uint8_t;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::DefaultRandomStart::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace DefaultRandomStart
namespace DefaultRandomDuration {
struct TypeInfo
{
    using Type             = uint8_t;
    using DecodableType    = uint8_t;
    using DecodableArgType = uint8_t;

    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr AttributeId GetAttributeId() { return Attributes::DefaultRandomDuration::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace DefaultRandomDuration
namespace GeneratedCommandList {
struct TypeInfo : public Clusters::Globals::Attributes::GeneratedCommandList::TypeInfo
{
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
};
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
struct TypeInfo : public Clusters::Globals::Attributes::AcceptedCommandList::TypeInfo
{
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
};
} // namespace AcceptedCommandList
namespace EventList {
struct TypeInfo : public Clusters::Globals::Attributes::EventList::TypeInfo
{
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
};
} // namespace EventList
namespace AttributeList {
struct TypeInfo : public Clusters::Globals::Attributes::AttributeList::TypeInfo
{
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
};
} // namespace AttributeList
namespace FeatureMap {
struct TypeInfo : public Clusters::Globals::Attributes::FeatureMap::TypeInfo
{
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
};
} // namespace FeatureMap
namespace ClusterRevision {
struct TypeInfo : public Clusters::Globals::Attributes::ClusterRevision::TypeInfo
{
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
};
} // namespace ClusterRevision

struct TypeInfo
{
    struct DecodableType
    {
        static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

        CHIP_ERROR Decode(TLV::TLVReader & reader, const ConcreteAttributePath & path);

        Attributes::DeviceClass::TypeInfo::DecodableType deviceClass =
            static_cast<chip::BitMask<chip::app::Clusters::DemandResponseLoadControl::DeviceClassBitmap>>(0);
        Attributes::LoadControlPrograms::TypeInfo::DecodableType loadControlPrograms;
        Attributes::NumberOfLoadControlPrograms::TypeInfo::DecodableType numberOfLoadControlPrograms = static_cast<uint8_t>(0);
        Attributes::Events::TypeInfo::DecodableType events;
        Attributes::ActiveEvents::TypeInfo::DecodableType activeEvents;
        Attributes::NumberOfEventsPerProgram::TypeInfo::DecodableType numberOfEventsPerProgram = static_cast<uint8_t>(0);
        Attributes::NumberOfTransistions::TypeInfo::DecodableType numberOfTransistions         = static_cast<uint8_t>(0);
        Attributes::DefaultRandomStart::TypeInfo::DecodableType defaultRandomStart             = static_cast<uint8_t>(0);
        Attributes::DefaultRandomDuration::TypeInfo::DecodableType defaultRandomDuration       = static_cast<uint8_t>(0);
        Attributes::GeneratedCommandList::TypeInfo::DecodableType generatedCommandList;
        Attributes::AcceptedCommandList::TypeInfo::DecodableType acceptedCommandList;
        Attributes::EventList::TypeInfo::DecodableType eventList;
        Attributes::AttributeList::TypeInfo::DecodableType attributeList;
        Attributes::FeatureMap::TypeInfo::DecodableType featureMap           = static_cast<uint32_t>(0);
        Attributes::ClusterRevision::TypeInfo::DecodableType clusterRevision = static_cast<uint16_t>(0);
    };
};
} // namespace Attributes
namespace Events {
namespace LoadControlEventStatusChange {
static constexpr PriorityLevel kPriorityLevel = PriorityLevel::Info;

enum class Fields : uint8_t
{
    kEventID              = 0,
    kTransitionIndex      = 1,
    kStatus               = 2,
    kCriticality          = 3,
    kControl              = 4,
    kTemperatureControl   = 5,
    kAverageLoadControl   = 6,
    kDutyCycleControl     = 7,
    kPowerSavingsControl  = 8,
    kHeatingSourceControl = 9,
};

struct Type
{
public:
    static constexpr PriorityLevel GetPriorityLevel() { return kPriorityLevel; }
    static constexpr EventId GetEventId() { return Events::LoadControlEventStatusChange::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }
    static constexpr bool kIsFabricScoped = false;

    chip::ByteSpan eventID;
    DataModel::Nullable<uint8_t> transitionIndex;
    LoadControlEventStatusEnum status         = static_cast<LoadControlEventStatusEnum>(0);
    CriticalityLevelEnum criticality          = static_cast<CriticalityLevelEnum>(0);
    chip::BitMask<EventControlBitmap> control = static_cast<chip::BitMask<EventControlBitmap>>(0);
    Optional<DataModel::Nullable<Structs::TemperatureControlStruct::Type>> temperatureControl;
    Optional<DataModel::Nullable<Structs::AverageLoadControlStruct::Type>> averageLoadControl;
    Optional<DataModel::Nullable<Structs::DutyCycleControlStruct::Type>> dutyCycleControl;
    Optional<DataModel::Nullable<Structs::PowerSavingsControlStruct::Type>> powerSavingsControl;
    Optional<DataModel::Nullable<Structs::HeatingSourceControlStruct::Type>> heatingSourceControl;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

struct DecodableType
{
public:
    static constexpr PriorityLevel GetPriorityLevel() { return kPriorityLevel; }
    static constexpr EventId GetEventId() { return Events::LoadControlEventStatusChange::Id; }
    static constexpr ClusterId GetClusterId() { return Clusters::DemandResponseLoadControl::Id; }

    chip::ByteSpan eventID;
    DataModel::Nullable<uint8_t> transitionIndex;
    LoadControlEventStatusEnum status         = static_cast<LoadControlEventStatusEnum>(0);
    CriticalityLevelEnum criticality          = static_cast<CriticalityLevelEnum>(0);
    chip::BitMask<EventControlBitmap> control = static_cast<chip::BitMask<EventControlBitmap>>(0);
    Optional<DataModel::Nullable<Structs::TemperatureControlStruct::DecodableType>> temperatureControl;
    Optional<DataModel::Nullable<Structs::AverageLoadControlStruct::DecodableType>> averageLoadControl;
    Optional<DataModel::Nullable<Structs::DutyCycleControlStruct::DecodableType>> dutyCycleControl;
    Optional<DataModel::Nullable<Structs::PowerSavingsControlStruct::DecodableType>> powerSavingsControl;
    Optional<DataModel::Nullable<Structs::HeatingSourceControlStruct::DecodableType>> heatingSourceControl;

    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace LoadControlEventStatusChange
} // namespace Events
} // namespace DemandResponseLoadControl
