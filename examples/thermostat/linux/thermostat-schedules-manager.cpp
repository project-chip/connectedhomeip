#include <app/clusters/thermostat-server/thermostat-server.h>

#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;

static std::vector<PresetStruct::Type> gsActivePresets;
static std::vector<PresetStruct::Type> gsEditingPresets;

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

static void 
onEditCommit(ThermostatMatterScheduleManager * mgr, ThermostatMatterScheduleManager::editType type)
{
    ChipLogProgress(Zcl, "ThermstatScheduleManager - onEditCommit %s", type == ThermostatMatterScheduleManager::Presets ? "Presets" : "Schedules");
    gsActivePresets = gsEditingPresets;
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
	gsEditingPresets.clear();
	return CHIP_NO_ERROR;
}

static CHIP_ERROR
appendPreset(ThermostatMatterScheduleManager *mgr, const PresetStruct::DecodableType &preset)
{
	gsEditingPresets.push_back(preset);	
	return CHIP_NO_ERROR;
}

// Instantiate the manager for endpoint 1
static ThermostatMatterScheduleManager gThermostatScheduleManager(
	1, 
	onEditStart, 
	onEditCancel, 
	onEditCommit, 
	getPresetTypeAtIndex, 
	getPresetAtIndex, 
	appendPreset, 
	clearPresets);