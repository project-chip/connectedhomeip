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

static unsigned int gsActivePresetsEmptyIndex = 0;
static std::array<PresetStruct::Type, kMaxPresets> gsActivePresets;

static unsigned int gsEditingPresetsEmptyIndex = 0;
static std::array<PresetStruct::Type, kMaxPresets> gsEditingPresets;

static unsigned int gsActiveSchedulesEmptyIndex = 0;
static std::array<ScheduleStruct::Type, kMaxSchedules> gsActiveSchedules;

static unsigned int gsEditingSchedulesEmptyIndex = 0;
static std::array<ScheduleStruct::Type, kMaxSchedules> gsEditingSchedules;

struct ExampleThermostatScheduleManager : public ThermostatMatterScheduleManager
{
    ExampleThermostatScheduleManager() : ThermostatMatterScheduleManager()
    {
        for (gsActivePresetsEmptyIndex = 0; gsActivePresetsEmptyIndex < sizeof(BuiltInPresets) / sizeof(BuiltInPresets[0]);
             ++gsActivePresetsEmptyIndex)
        {
            gsActivePresets[gsActivePresetsEmptyIndex] = BuiltInPresets[gsActivePresetsEmptyIndex];
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
    virtual CHIP_ERROR AppendPreset(chip::EndpointId aEndpoint, const PresetStruct::DecodableType & preset);

    // schedules
    virtual CHIP_ERROR GetScheduleTypeAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleTypeStruct::Type & scheduleType) const;
    virtual CHIP_ERROR GetScheduleAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleStruct::Type & schedule) const;
    virtual CHIP_ERROR ClearSchedules(chip::EndpointId aEndpoint);
    virtual CHIP_ERROR AppendSchedule(chip::EndpointId aEndpoint, const Structs::ScheduleStruct::DecodableType & schedule);

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
        Span<PresetStruct::Type> oldPresets = Span<PresetStruct::Type>(gsActivePresets).SubSpan(0, gsActivePresetsEmptyIndex);
        Span<PresetStruct::Type> newPresets = Span<PresetStruct::Type>(gsEditingPresets).SubSpan(0, gsEditingPresetsEmptyIndex);

        status = ValidatePresetsForCommitting(aEndpoint, oldPresets, newPresets);
        {
            StatusIB statusIB(status);
            SuccessOrExit(statusIB.ToChipError());    
        }
        // New presets look good, lets generate some new ID's for the new presets.
        for (unsigned int index = 0; index < gsEditingPresetsEmptyIndex; ++index)
        {
            if (gsEditingPresets[index].presetHandle.IsNull() || gsEditingPresets[index].presetHandle.Value().empty())
            {
                char handle[16];
                snprintf(handle, 16, "%s%d", userPresetPrefix, nextNewPresetHandle++);
                gsEditingPresets[index].presetHandle.SetNonNull(ByteSpan((const unsigned char *) handle, strlen(handle)));
            }
        }
    }

    if (gsEditingSchedulesEmptyIndex != 0)
    {
        Span<ScheduleStruct::Type> oldSchedules = Span<ScheduleStruct::Type>(gsActiveSchedules).SubSpan(0, gsActiveSchedulesEmptyIndex);
        Span<ScheduleStruct::Type> newSchedules = Span<ScheduleStruct::Type>(gsEditingSchedules).SubSpan(0, gsEditingSchedulesEmptyIndex);
        Span<PresetStruct::Type> presets = gsEditingPresetsEmptyIndex > 0 ? Span<PresetStruct::Type>(gsEditingPresets).SubSpan(0, gsEditingPresetsEmptyIndex) :
                                                                            Span<PresetStruct::Type>(gsActivePresets).SubSpan(0, gsActivePresetsEmptyIndex);

        status = ValidateSchedulesForCommitting(aEndpoint, oldSchedules, newSchedules, presets);
        {
            StatusIB statusIB(status);
            SuccessOrExit(statusIB.ToChipError());
        }

        // New schedules look good, lets generate some new ID's for the new schedules.
        for (unsigned int index = 0; index < gsEditingSchedulesEmptyIndex; ++index)
        {
            if (gsEditingSchedules[index].scheduleHandle.IsNull() || gsEditingSchedules[index].scheduleHandle.Value().empty())
            {
                char handle[16];
                snprintf(handle, 16, "%s%d", userSchedulePrefix, nextNewScheduleHandle++);
                gsEditingSchedules[index].scheduleHandle.SetNonNull(ByteSpan((const unsigned char *) handle, strlen(handle)));
            }
        }
    }


    // Everything *SHOULD* be validated now, so lets commit them.


    // copy the presets to the active list.
    gsActivePresets            = gsEditingPresets;
    gsEditingPresetsEmptyIndex = 0;

    // TODO: update thermostat attributes for new presets.
    
    // TODO: do the validation and commit for schedules.

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
    if (index < gsActivePresets.size())
    {
        preset = gsActivePresets[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::ClearPresets(chip::EndpointId aEndpoint)
{
    gsEditingPresetsEmptyIndex = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleThermostatScheduleManager::AppendPreset(chip::EndpointId aEndpoint, const PresetStruct::DecodableType & preset)
{
    if (gsEditingPresetsEmptyIndex >= gsEditingPresets.size())
        return CHIP_ERROR_INVALID_ARGUMENT;

    gsEditingPresets[gsEditingPresetsEmptyIndex] = preset;
    gsEditingPresetsEmptyIndex++;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleThermostatScheduleManager::GetScheduleTypeAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleTypeStruct::Type & scheduleType) const
{
#if 0
    static ScheduleTypeStruct::Type presetTypes[] = {
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
#endif
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::GetScheduleAtIndex(chip::EndpointId aEndpoint, size_t index, ScheduleStruct::Type & schedule) const
{
    if (index < gsActiveSchedules.size())
    {
        schedule = gsActiveSchedules[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ExampleThermostatScheduleManager::ClearSchedules(chip::EndpointId aEndpoint)
{
    gsEditingSchedulesEmptyIndex = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleThermostatScheduleManager::AppendSchedule(chip::EndpointId aEndpoint, const ScheduleStruct::DecodableType & schedule)
{
    if (gsEditingSchedulesEmptyIndex >= gsEditingSchedules.size())
        return CHIP_ERROR_INVALID_ARGUMENT;

    // DecodableType and Type are defined seperately
    gsEditingSchedules[gsEditingSchedulesEmptyIndex].scheduleHandle = schedule.scheduleHandle;
    gsEditingSchedules[gsEditingSchedulesEmptyIndex].systemMode = schedule.systemMode;
    gsEditingSchedules[gsEditingSchedulesEmptyIndex].name = schedule.name;
    gsEditingSchedules[gsEditingSchedulesEmptyIndex].presetHandle = schedule.presetHandle;
//    gsEditingSchedules[gsEditingSchedulesEmptyIndex].transitions = schedule.transitions;
    gsEditingSchedules[gsEditingSchedulesEmptyIndex].builtIn = schedule.builtIn;

    gsEditingSchedulesEmptyIndex++;

    return CHIP_NO_ERROR;
}

// Instantiate the manager for endpoint 1
static ExampleThermostatScheduleManager gThermostatPresetManager;
