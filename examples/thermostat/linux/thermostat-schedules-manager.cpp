#include <app/clusters/thermostat-server/thermostat-server.h>

#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

// built in presets will be prefixed with B
constexpr const char * kBuiltInOneHandle = "B1";
constexpr const char * kOccupiedName = "Occupied Default";

constexpr const char * kBuiltinTwoHandle = "B2";
constexpr const char * kUnoccupiedName = "Unoccupied Default";

constexpr const int kMaxPresets = 10;

// user presets will be prefixed with U 
static const char * userPresetPrefix = "U";
static uint32_t nextNewHandle = 0;

static PresetStruct::Type BuiltInPresets[] = 
{
	{
		.presetHandle = DataModel::Nullable<chip::ByteSpan>(chip::ByteSpan(Uint8::from_const_char(kBuiltInOneHandle), strlen(kBuiltInOneHandle))),
		.presetScenario = PresetScenarioEnum::kOccupied,
		.name = Optional<DataModel::Nullable<chip::CharSpan>>(DataModel::Nullable<chip::CharSpan>(chip::CharSpan(kOccupiedName, strlen(kOccupiedName)))),
		.coolingSetpoint = Optional<int16_t>(2400),
		.heatingSetpoint = Optional<int16_t>(1800),
		.builtIn = DataModel::Nullable<bool>(true)
	},
	{
		.presetHandle = DataModel::Nullable<chip::ByteSpan>(chip::ByteSpan(Uint8::from_const_char(kBuiltinTwoHandle), strlen(kBuiltinTwoHandle))),
		.presetScenario = PresetScenarioEnum::kUnoccupied,
		.name = Optional<DataModel::Nullable<chip::CharSpan>>(DataModel::Nullable<chip::CharSpan>(chip::CharSpan(kUnoccupiedName, strlen(kUnoccupiedName)))),
		.coolingSetpoint = Optional<int16_t>(3200),
		.heatingSetpoint = Optional<int16_t>(1000),
		.builtIn = DataModel::Nullable<bool>(true)
	}
};

static unsigned int gsActivePresetsEmptyIndex = 0;
static std::array<PresetStruct::Type, kMaxPresets> gsActivePresets;

static unsigned int gsEditingPresetsEmptyIndex = 0;
static std::array<PresetStruct::Type, kMaxPresets> gsEditingPresets;

static void 
onEditStart(ThermostatMatterScheduleManager * mgr, ThermostatMatterScheduleManager::editType type)
{
    ChipLogProgress(Zcl, "ThermstatScheduleManager - onEditStart %s", type == ThermostatMatterScheduleManager::Presets ? "Presets" : "Schedules");
}

static void 
onEditCancel(ThermostatMatterScheduleManager * mgr, ThermostatMatterScheduleManager::editType type)
{
    ChipLogProgress(Zcl, "ThermstatScheduleManager - onEditCancel %s", type == ThermostatMatterScheduleManager::Presets ? "Presets" : "Schedules");
}

static EmberAfStatus
onEditCommit(ThermostatMatterScheduleManager * mgr, ThermostatMatterScheduleManager::editType type)
{
	EmberAfStatus status;

	if (type == ThermostatMatterScheduleManager::Presets)
	{
	    ChipLogProgress(Zcl, "ThermstatScheduleManager - onEditCommit %s", type == ThermostatMatterScheduleManager::Presets ? "Presets" : "Schedules");
		Span<PresetStruct::Type> oldPresets = Span<PresetStruct::Type>(gsActivePresets).SubSpan(0, gsActivePresetsEmptyIndex);
		Span<PresetStruct::Type> newPresets = Span<PresetStruct::Type>(gsEditingPresets).SubSpan(0, gsEditingPresetsEmptyIndex);
		
		status = mgr->ThermostatMatterScheduleManager::ValidatePresetsForCommitting(oldPresets, newPresets);
		SuccessOrExit(status);

		// New presets look good, lets generate some new ID's for the new presets.
		for (unsigned int index=0; index < gsEditingPresetsEmptyIndex; ++index)
		{
			if (gsEditingPresets[index].presetHandle.IsNull() || gsEditingPresets[index].presetHandle.Value().empty())
			{
				char handle[16];
				snprintf(handle, 16, "%s%d", userPresetPrefix, nextNewHandle++);
				gsEditingPresets[index].presetHandle.SetNonNull(ByteSpan((const unsigned char *)handle, strlen(handle)));
			}
		}

		// copy the presets to the active list.
	    gsActivePresets = gsEditingPresets;

	    // TODO: update thermostat attributes for new presets.
	}
	else if (type == ThermostatMatterScheduleManager::Schedules)
	{
			// TODO: schedules impl
	}
	else
	{
		VerifyOrDie(false);
	}

exit:
    return status;
}

static CHIP_ERROR
getPresetTypeAtIndex(ThermostatMatterScheduleManager * mgr, size_t index, PresetTypeStruct::Type &presetType)
{
	static PresetTypeStruct::Type presetTypes[] =
	{
		{
			.presetScenario = PresetScenarioEnum::kOccupied,
			.numberOfPresets = 4,
			.presetTypeFeatures = (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames))
		},
		{
			.presetScenario = PresetScenarioEnum::kUnoccupied,
			.numberOfPresets = 2,
			.presetTypeFeatures = (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames))
		},
		{
			.presetScenario = PresetScenarioEnum::kSleep,
			.numberOfPresets = 5,
			.presetTypeFeatures = (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames))
		},
		{
			.presetScenario = PresetScenarioEnum::kWake,
			.numberOfPresets = 3,
			.presetTypeFeatures = (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames))
		},
		{
			.presetScenario = PresetScenarioEnum::kVacation,
			.numberOfPresets = 1,
			.presetTypeFeatures = (to_underlying(PresetTypeFeaturesBitmap::kAutomatic) | to_underlying(PresetTypeFeaturesBitmap::kSupportsNames))
		},
	};

	if (index < 1)
	{
		presetType = presetTypes[index];
		return CHIP_NO_ERROR;
	}
	return CHIP_ERROR_NOT_FOUND;
}

static CHIP_ERROR
getPresetAtIndex(ThermostatMatterScheduleManager * mgr, size_t index, PresetStruct::Type &preset)
{
	if (index < gsActivePresets.size())
	{
		preset = gsActivePresets[index];
		return CHIP_NO_ERROR;
	}
	return CHIP_ERROR_NOT_FOUND;	
}

static CHIP_ERROR
clearPresets(ThermostatMatterScheduleManager * mgr)
{
	gsEditingPresetsEmptyIndex = 0;
	return CHIP_NO_ERROR;
}

static CHIP_ERROR
appendPreset(ThermostatMatterScheduleManager *mgr, const PresetStruct::DecodableType &preset)
{
	if (gsEditingPresetsEmptyIndex >= gsEditingPresets.size())
		return CHIP_ERROR_INVALID_ARGUMENT;

	gsEditingPresets[gsEditingPresetsEmptyIndex] = preset;
	gsEditingPresetsEmptyIndex++;

	return CHIP_NO_ERROR;
}

struct ExampleThermostatPresetManager : public ThermostatMatterScheduleManager
{
	ExampleThermostatPresetManager(chip::EndpointId endpoint)
	: ThermostatMatterScheduleManager(
		endpoint,
		onEditStart, 
		onEditCancel, 
		onEditCommit, 
		getPresetTypeAtIndex, 
		getPresetAtIndex, 
		appendPreset, 
		clearPresets)
	{
		for (gsActivePresetsEmptyIndex=0; gsActivePresetsEmptyIndex<sizeof(BuiltInPresets)/sizeof(BuiltInPresets[0]); ++gsActivePresetsEmptyIndex)
		{
			gsActivePresets[gsActivePresetsEmptyIndex] = BuiltInPresets[gsActivePresetsEmptyIndex];
		}
	}
};

// Instantiate the manager for endpoint 1
static ExampleThermostatPresetManager gThermostatPresetManager(1);