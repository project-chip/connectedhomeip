#include <app/clusters/thermostat-server/thermostat-server.h>
#include <app/MessageDef/StatusIB.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

using Protocols::InteractionModel::Status;

// built in presets will be prefixed with B
constexpr const char * kBuiltInPOneHandle = "BP1";
constexpr const char * kOccupiedName     = "Occupied Default";

constexpr const char * kBuiltinPTwoHandle = "BP2";
constexpr const char * kUnoccupiedName   = "Unoccupied Default";

constexpr const int kMaxPresets = 10;

// user presets will be prefixed with UP
static const char * userPresetPrefix = "UP";
static uint32_t nextNewPresetHandle  = 0;

// user schedules will be prefixed with US
static const char * userSchedulePrefix = "US";
static uint32_t nextNewScheduleHandle  = 0;

static PresetStruct::Type BuiltInPresets[] = {
    { .presetHandle =
          DataModel::Nullable<chip::ByteSpan>(chip::ByteSpan(Uint8::from_const_char(kBuiltInPOneHandle), strlen(kBuiltInPOneHandle))),
      .presetScenario = PresetScenarioEnum::kOccupied,
      .name           = Optional<DataModel::Nullable<chip::CharSpan>>(
          DataModel::Nullable<chip::CharSpan>(chip::CharSpan(kOccupiedName, strlen(kOccupiedName)))),
      .coolingSetpoint = Optional<int16_t>(2400),
      .heatingSetpoint = Optional<int16_t>(1800),
      .builtIn         = DataModel::Nullable<bool>(true) },
    { .presetHandle =
          DataModel::Nullable<chip::ByteSpan>(chip::ByteSpan(Uint8::from_const_char(kBuiltinPTwoHandle), strlen(kBuiltinPTwoHandle))),
      .presetScenario = PresetScenarioEnum::kUnoccupied,
      .name           = Optional<DataModel::Nullable<chip::CharSpan>>(
          DataModel::Nullable<chip::CharSpan>(chip::CharSpan(kUnoccupiedName, strlen(kUnoccupiedName)))),
      .coolingSetpoint = Optional<int16_t>(3200),
      .heatingSetpoint = Optional<int16_t>(1000),
      .builtIn         = DataModel::Nullable<bool>(true) }
};

// built in schedules will be prefixed with B
constexpr const char * kBuiltInSOneHandle = "BS1";
constexpr const char * kBuildInScheduleOneName = "Schedule1";
constexpr const char * kBuiltinSTwoHandle = "BS2";
constexpr const char * kBuildInScheduleTwoName = "Schedule2";
constexpr const int kMaxSchedules = 10;
constexpr const int kMaxTransitionsPerSchedule = 10;

static ScheduleStruct::Type BuiltInSchedules[] = {
    {
        .scheduleHandle = DataModel::Nullable<chip::ByteSpan>(chip::ByteSpan(Uint8::from_const_char(kBuiltInSOneHandle), strlen(kBuiltInSOneHandle))),
        .systemMode = SystemModeEnum::kAuto,
        .name           = Optional<chip::CharSpan>(chip::CharSpan(kBuildInScheduleOneName, strlen(kBuildInScheduleOneName))),
        .presetHandle = Optional<chip::ByteSpan>(chip::ByteSpan(Uint8::from_const_char(kBuiltInPOneHandle),  strlen(kBuiltInPOneHandle))),
        .transitions = DataModel::List<const Structs::ScheduleTransitionStruct::Type>(),
        .builtIn = DataModel::Nullable<bool>(true)
    },
};

template <typename T, std::size_t a, std::size_t b>
using array2d = std::array<std::array<T, a>, b>;

static unsigned int gsActivePresetsEmptyIndex = 0;
static unsigned int gsEditingPresetsEmptyIndex = 0;
static unsigned int gsCurrentPresetsDataArrayIndex = 0;
static array2d<PresetStruct::Type, 2, kMaxPresets> gsPresets;

static inline unsigned int GetActivePresetDataIndex() { return gsCurrentPresetsDataArrayIndex; }
static inline unsigned int GetEditPresetDataIndex() { return gsCurrentPresetsDataArrayIndex == 0 ? 1 : 0; }
static inline unsigned int ToggleActivePresetDataIndex() { return gsCurrentPresetsDataArrayIndex = GetEditPresetDataIndex(); }

static unsigned int gsActiveSchedulesEmptyIndex = 0;
static unsigned int gsEditingSchedulesEmptyIndex = 0;
static unsigned int gsCurrentSchedulesDataArrayIndex = 0;
static array2d<ScheduleStruct::Type, 2, kMaxSchedules> gsSchedules;
static array2d<ScheduleTransitionStruct::Type, 2, kMaxSchedules * kMaxTransitionsPerSchedule> gsTransitions;

static inline unsigned int GetActiveSchedulesDataIndex() { return gsCurrentSchedulesDataArrayIndex; }
static inline unsigned int GetEditSchedulesDataIndex() { return gsCurrentSchedulesDataArrayIndex == 0 ? 1 : 0; }
static inline unsigned int ToggleActiveSchedulesDataIndex() { return gsCurrentSchedulesDataArrayIndex = GetEditSchedulesDataIndex(); }

struct ExampleThermostatScheduleManager : public ThermostatMatterScheduleManager
{
    ExampleThermostatScheduleManager() : ThermostatMatterScheduleManager()
    {
        for (gsActivePresetsEmptyIndex = 0; gsActivePresetsEmptyIndex < sizeof(BuiltInPresets) / sizeof(BuiltInPresets[0]);
             ++gsActivePresetsEmptyIndex)
        {
            gsPresets[GetActivePresetDataIndex()][gsActivePresetsEmptyIndex] = BuiltInPresets[gsActivePresetsEmptyIndex];
        }
        SetActiveInstance(this);
    }

    virtual ~ExampleThermostatScheduleManager() {};

    virtual bool IsEditing() { return IsEditing (1); };
    virtual bool IsEditing(chip::EndpointId aEndpoint) { return mEditing; };
    
    virtual CHIP_ERROR StartEditing(chip::EndpointId aEndpoint);
    virtual CHIP_ERROR RollbackEdits();
    virtual CHIP_ERROR RollbackEdits(chip::EndpointId aEndpoint);    

    virtual chip::Protocols::InteractionModel::Status CommitEdits(chip::EndpointId aEndpoint);

    // presets
    virtual CHIP_ERROR GetPresetTypeAtIndex(chip::EndpointId aEndpoint, size_t aIndex, PresetTypeStruct::Type & outPresetType) const;
    virtual CHIP_ERROR GetPresetAtIndex(chip::EndpointId aEndpoint, size_t aIndex, PresetStruct::Type & outPreset) const;
    virtual CHIP_ERROR ClearPresets(chip::EndpointId aEndpoint);
    virtual chip::Protocols::InteractionModel::Status AppendPreset(chip::EndpointId aEndpoint, const PresetStruct::DecodableType & preset);

    // schedules
    virtual CHIP_ERROR GetScheduleTypeAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleTypeStruct::Type & scheduleType) const;
    virtual CHIP_ERROR GetScheduleAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleStruct::Type & schedule) const;
    virtual CHIP_ERROR ClearSchedules(chip::EndpointId aEndpoint);
    virtual chip::Protocols::InteractionModel::Status AppendSchedule(chip::EndpointId aEndpoint, const Structs::ScheduleStruct::DecodableType & schedule);

private:
    bool mEditing = false;
};


CHIP_ERROR ExampleThermostatScheduleManager::StartEditing(chip::EndpointId aEndpoint)
{
    ChipLogProgress(Zcl, "ExampleThermostatScheduleManager - StartEditing");
    if (mEditing == true)
        return CHIP_ERROR_BUSY;

    mEditing = true;
    gsEditingPresetsEmptyIndex = 0;
    gsEditingSchedulesEmptyIndex = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleThermostatScheduleManager::RollbackEdits()
{
    return RollbackEdits(1);
}

CHIP_ERROR ExampleThermostatScheduleManager::RollbackEdits(chip::EndpointId aEndpoint)
{
    ChipLogProgress(Zcl, "ExampleThermostatScheduleManager - RollbackEdits");
    gsEditingPresetsEmptyIndex = 0;
    gsEditingSchedulesEmptyIndex = 0;
    return CHIP_NO_ERROR;
}

Status ExampleThermostatScheduleManager::CommitEdits(chip::EndpointId aEndpoint)
{
    Status status = Status::Success;

    ChipLogProgress(Zcl, "ExampleThermostatScheduleManager - CommitEdits");

    if (gsEditingPresetsEmptyIndex != 0)
    {
        Span<PresetStruct::Type> oldPresets = Span<PresetStruct::Type>(gsPresets[GetActivePresetDataIndex()]).SubSpan(0, gsActivePresetsEmptyIndex);
        Span<PresetStruct::Type> newPresets = Span<PresetStruct::Type>(gsPresets[GetEditPresetDataIndex()]).SubSpan(0, gsEditingPresetsEmptyIndex);

        status = ValidatePresetsForCommitting(aEndpoint, oldPresets, newPresets);
        {
            StatusIB statusIB(status);
            SuccessOrExit(statusIB.ToChipError());    
        }
        // New presets look good, lets generate some new ID's for the new presets.
        for (unsigned int index = 0; index < gsEditingPresetsEmptyIndex; ++index)
        {
            if (gsPresets[GetEditPresetDataIndex()][index].presetHandle.IsNull() || gsPresets[GetEditPresetDataIndex()][index].presetHandle.Value().empty())
            {
                char handle[16];
                snprintf(handle, 16, "%s%d", userPresetPrefix, nextNewPresetHandle++);
                gsPresets[GetEditPresetDataIndex()][index].presetHandle.SetNonNull(ByteSpan((const unsigned char *) handle, strlen(handle)));
            }
        }
    }

    if (gsEditingSchedulesEmptyIndex != 0)
    {
        Span<ScheduleStruct::Type> oldSchedules = Span<ScheduleStruct::Type>(gsSchedules[GetActiveSchedulesDataIndex()]).SubSpan(0, gsActiveSchedulesEmptyIndex);
        Span<ScheduleStruct::Type> newSchedules = Span<ScheduleStruct::Type>(gsSchedules[GetEditSchedulesDataIndex()]).SubSpan(0, gsEditingSchedulesEmptyIndex);
        Span<PresetStruct::Type> presets = gsEditingPresetsEmptyIndex > 0 ? Span<PresetStruct::Type>(gsPresets[GetEditPresetDataIndex()]).SubSpan(0, gsEditingPresetsEmptyIndex) :
                                                                            Span<PresetStruct::Type>(gsPresets[GetActivePresetDataIndex()]).SubSpan(0, gsActivePresetsEmptyIndex);

        status = ValidateSchedulesForCommitting(aEndpoint, oldSchedules, newSchedules, presets);
        {
            StatusIB statusIB(status);
            SuccessOrExit(statusIB.ToChipError());
        }

        // New schedules look good, lets generate some new ID's for the new schedules.
        for (unsigned int index = 0; index < gsEditingSchedulesEmptyIndex; ++index)
        {
            if (gsSchedules[GetEditSchedulesDataIndex()][index].scheduleHandle.IsNull() || gsSchedules[GetEditSchedulesDataIndex()][index].scheduleHandle.Value().empty())
            {
                char handle[16];
                snprintf(handle, 16, "%s%d", userSchedulePrefix, nextNewScheduleHandle++);
                gsSchedules[GetEditSchedulesDataIndex()][index].scheduleHandle.SetNonNull(ByteSpan((const unsigned char *) handle, strlen(handle)));
            }
        }
    }

    // make the edit arrays the active arrays.
    if (gsEditingPresetsEmptyIndex != 0)
    {
        ToggleActivePresetDataIndex();
        gsActivePresetsEmptyIndex = gsEditingPresetsEmptyIndex;
        gsEditingPresetsEmptyIndex = 0;
    }
    if (gsEditingSchedulesEmptyIndex != 0)
    {
        ToggleActiveSchedulesDataIndex();
        gsActiveSchedulesEmptyIndex = gsEditingSchedulesEmptyIndex;
        gsEditingSchedulesEmptyIndex = 0;
    }

    // TODO: update thermostat attributes for new presets.
    
exit:
    return status;
}

CHIP_ERROR ExampleThermostatScheduleManager::GetPresetTypeAtIndex(chip::EndpointId aEndpoint, size_t index, PresetTypeStruct::Type & presetType) const
{
    static PresetTypeStruct::Type presetTypes[] = {
        { .presetScenario  = PresetScenarioEnum::kOccupied,
          .numberOfPresets = 4,
          .presetTypeFeatures =
              (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames)) },
        { .presetScenario  = PresetScenarioEnum::kUnoccupied,
          .numberOfPresets = 2,
          .presetTypeFeatures =
              (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames)) },
        { .presetScenario  = PresetScenarioEnum::kSleep,
          .numberOfPresets = 5,
          .presetTypeFeatures =
              (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames)) },
        { .presetScenario  = PresetScenarioEnum::kWake,
          .numberOfPresets = 3,
          .presetTypeFeatures =
              (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames)) },
        { .presetScenario  = PresetScenarioEnum::kVacation,
          .numberOfPresets = 1,
          .presetTypeFeatures =
              (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames)) },
    };

    if (index < 1)
    {
        presetType = presetTypes[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::GetPresetAtIndex(chip::EndpointId aEndpoint, size_t index, PresetStruct::Type & preset) const
{
    if (index < gsPresets[GetActivePresetDataIndex()].size())
    {
        preset = gsPresets[GetActivePresetDataIndex()][index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::ClearPresets(chip::EndpointId aEndpoint)
{
    gsEditingPresetsEmptyIndex = 0;
    return CHIP_NO_ERROR;
}

chip::Protocols::InteractionModel::Status ExampleThermostatScheduleManager::AppendPreset(chip::EndpointId aEndpoint, const PresetStruct::DecodableType & preset)
{
    if (gsEditingPresetsEmptyIndex >= gsPresets[GetEditPresetDataIndex()].size())
        return Status::ResourceExhausted;

    gsPresets[GetEditPresetDataIndex()][gsEditingPresetsEmptyIndex] = preset;
    gsEditingPresetsEmptyIndex++;

    return Status::Success;
}

CHIP_ERROR ExampleThermostatScheduleManager::GetScheduleTypeAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleTypeStruct::Type & scheduleType) const
{
    static ScheduleTypeStruct::Type scheduleTypes[] = {
        { .systemMode  = SystemModeEnum::kOff,
          .numberOfSchedules = 1,
          .scheduleTypeFeatures =
              (to_underlying(ScheduleTypeFeaturesBitmap::kSupportsPresets) | 
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsNames) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsOff)
                )
        },
        { .systemMode  = SystemModeEnum::kHeat,
          .numberOfSchedules = 1,
          .scheduleTypeFeatures =
              (to_underlying(ScheduleTypeFeaturesBitmap::kSupportsPresets) | 
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsNames) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsOff)
                ) 
        },
        { .systemMode  = SystemModeEnum::kCool,
          .numberOfSchedules = 1,
          .scheduleTypeFeatures =
              (to_underlying(ScheduleTypeFeaturesBitmap::kSupportsPresets) | 
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsNames) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsOff)
                ) 
        },
        { .systemMode  = SystemModeEnum::kAuto,
          .numberOfSchedules = 1,
          .scheduleTypeFeatures =
              (to_underlying(ScheduleTypeFeaturesBitmap::kSupportsPresets) | 
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsNames) |
                to_underlying(ScheduleTypeFeaturesBitmap::kSupportsOff)
                ) 
        }
    };

    if (index < 1)
    {
        scheduleType = scheduleTypes[index];
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::GetScheduleAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleStruct::Type & schedule) const
{
    if (index < gsSchedules[GetActiveSchedulesDataIndex()].size())
    {
        schedule = gsSchedules[GetActiveSchedulesDataIndex()][index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::ClearSchedules(chip::EndpointId aEndpoint)
{
    gsEditingSchedulesEmptyIndex = 0;
    return CHIP_NO_ERROR;
}

chip::Protocols::InteractionModel::Status ExampleThermostatScheduleManager::AppendSchedule(chip::EndpointId aEndpoint, const ScheduleStruct::DecodableType & schedule)
{
    if (gsEditingSchedulesEmptyIndex >= gsSchedules[GetEditSchedulesDataIndex()].size())
        return Status::ResourceExhausted;

    // append the transitions, then apply to the schedule
    auto iterator = schedule.transitions.begin();
    size_t transitionIdx = gsEditingSchedulesEmptyIndex * kMaxSchedules;
    while (iterator.Next())
    {
        if (transitionIdx > ((gsEditingSchedulesEmptyIndex * kMaxSchedules) + kMaxTransitionsPerSchedule))
            return Status::ResourceExhausted;
        gsTransitions[GetEditSchedulesDataIndex()][transitionIdx].dayOfWeek = iterator.GetValue().dayOfWeek;
        gsTransitions[GetEditSchedulesDataIndex()][transitionIdx].transitionTime = iterator.GetValue().transitionTime;
        gsTransitions[GetEditSchedulesDataIndex()][transitionIdx].presetHandle = iterator.GetValue().presetHandle;
        gsTransitions[GetEditSchedulesDataIndex()][transitionIdx].systemMode = iterator.GetValue().systemMode;
        gsTransitions[GetEditSchedulesDataIndex()][transitionIdx].coolingSetpoint = iterator.GetValue().coolingSetpoint;
        gsTransitions[GetEditSchedulesDataIndex()][transitionIdx].heatingSetpoint = iterator.GetValue().heatingSetpoint;
        transitionIdx++;
    }

    Span<const Structs::ScheduleTransitionStruct::Type> transitions(&(gsTransitions[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex * kMaxSchedules]), 
                                                                      transitionIdx - (gsEditingSchedulesEmptyIndex * kMaxSchedules));

    // DecodableType and Type are defined seperately
    gsSchedules[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex].scheduleHandle = schedule.scheduleHandle;
    gsSchedules[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex].systemMode = schedule.systemMode;
    gsSchedules[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex].name = schedule.name;
    gsSchedules[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex].presetHandle = schedule.presetHandle;
    gsSchedules[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex].transitions = DataModel::List<const ScheduleTransitionStruct::Type>(transitions) ;
    gsSchedules[GetEditSchedulesDataIndex()][gsEditingSchedulesEmptyIndex].builtIn = schedule.builtIn;

    gsEditingSchedulesEmptyIndex++;

    return Status::Success;
}

// Instantiate the manager for endpoint 1
static ExampleThermostatScheduleManager gThermostatPresetManager;
