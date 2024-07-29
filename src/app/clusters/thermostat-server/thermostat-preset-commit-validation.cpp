#if 0

#include "thermostat-server.h"
#include <app/util/attribute-storage.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;

using imcode = Protocols::InteractionModel::Status;

static imcode FindPresetByHandle(const chip::ByteSpan & handle, const Span<PresetStruct::Type> & list,
                                        PresetStruct::Type & outPreset)
{
    for (auto & preset : list)
    {
        if ((preset.presetHandle.IsNull() == false) && handle.data_equal(preset.presetHandle.Value()))
        {
            outPreset = preset;
            return imcode::Success;
        }
    }

    return imcode::NotFound;
}

static imcode CheckPresetHandleUnique(const chip::ByteSpan & handle, Span<PresetStruct::Type> & list)
{
    int count = 0;
    for (auto & preset : list)
    {
        if ((preset.presetHandle.IsNull() == false) && handle.data_equal(preset.presetHandle.Value()))
        {
            if (count == 0)
            {
                count++;
            }
            else
            {
                return imcode::ConstraintError;
            }
        }
    }

    return imcode::Success;
}

#if 0
struct Type
{
public:
    chip::ByteSpan sceduleHandle;
    ThermostatSystemModeEnum systemMode = static_cast<ThermostatSystemModeEnum>(0);
    DataModel::Nullable<chip::CharSpan> name;
    chip::ByteSpan presetHandle;
    DataModel::List<const Structs::ScheduleTransitionStruct::Type> transitions;
    bool builtIn = static_cast<bool>(0);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};
#endif

static bool IsPresetHandleReferenced(ThermostatMatterScheduleManager & mgr, const chip::ByteSpan & handle)
{
    imcode status = imcode::Success;
    uint32_t ourFeatureMap;
    FeatureMap::Get(mgr.mEndpoint, &ourFeatureMap);
    const bool enhancedSchedulesSupported = ourFeatureMap & to_underlying(Feature::kMatterScheduleConfiguration);

    // Check Active Preset Handle
    DataModel::Nullable<chip::MutableByteSpan> activePresetHandle;
    status = ActivePresetHandle::Get(mgr.mEndpoint, activePresetHandle);
    VerifyOrDie(status == imcode::Success);
    if ((activePresetHandle.IsNull() == false) && activePresetHandle.Value().data_equal(handle))
        return true;

    // Check for the preset on the Schedules
    if (enhancedSchedulesSupported && mgr.mGetScheduleAtIndexCb)
    {
        size_t index = 0;
        ScheduleStruct::Type schedule;
        while (mgr.mGetScheduleAtIndexCb(&mgr, index, schedule) != CHIP_ERROR_NOT_FOUND)
        {
            if (schedule.presetHandle.HasValue() && schedule.presetHandle.Value().data_equal(handle))
                return true;

            for (auto transition : schedule.transitions)
            {
                if (transition.presetHandle.HasValue() && transition.presetHandle.Value().data_equal(handle))
                    return true;
            }
            index++;
        }
    }

    return false;
}

#if 0
// Enum for PresetScenarioEnum
enum class PresetScenarioEnum : uint8_t
{
    kUnspecified = 0x00,
    kOccupied    = 0x01,
    kUnoccupied  = 0x02,
    kSleep       = 0x03,
    kWake        = 0x04,
    kVacation    = 0x05,
    kUserDefined = 0x06,
    // All received enum values that are not listed above will be mapped
    // to kUnknownEnumValue. This is a helper enum value that should only
    // be used by code to process how it handles receiving and unknown
    // enum value. This specific should never be transmitted.
    kUnknownEnumValue = 7,
};

struct Type
{
public:
    PresetScenarioEnum presetScenario                          = static_cast<PresetScenarioEnum>(0);
    uint8_t numberOfPresets                                    = static_cast<uint8_t>(0);
    chip::BitMask<PresetTypeFeaturesBitmap> presetTypeFeatures = static_cast<chip::BitMask<PresetTypeFeaturesBitmap>>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

// Bitmap for PresetTypeFeaturesBitmap
enum class PresetTypeFeaturesBitmap : uint8_t
{
    kAutomatic     = 0x1,
    kSupportsNames = 0x2,
};

#endif

static imcode CheckPresetType(ThermostatMatterScheduleManager & mgr, PresetStruct::Type & preset)
{
    imcode status = imcode::ConstraintError;
    size_t index         = 0;
    PresetTypeStruct::Type presetType;

    VerifyOrDie(mgr.mGetPresetTypeAtIndexCb);
    while (mgr.mGetPresetTypeAtIndexCb(&mgr, index, presetType) != CHIP_ERROR_NOT_FOUND)
    {
        // look for the preset type that supports this scenario
        if (presetType.presetScenario == preset.presetScenario)
        {
            // we have one, check the name requirements
            if ((preset.name.HasValue() == true) && (preset.name.Value().IsNull() == false) &&
                (preset.name.Value().Value().empty() == false))
            {
                const bool nameSupported = presetType.presetTypeFeatures.Has(PresetTypeFeaturesBitmap::kSupportsNames);
                VerifyOrReturnError(nameSupported == true, imcode::ConstraintError);
            }

            return imcode::Success;
        }
        index++;
    }

    return status;
}

imcode ThermostatMatterScheduleManager::ValidatePresetsForCommitting(Span<PresetStruct::Type> & oldlist,
                                                                            Span<PresetStruct::Type> & newlist)
{
    imcode status = imcode::Success;
    PresetStruct::Type queryPreset; // preset storage used for queries.

    // Check that new_list can fit.
    uint8_t numPresets;
    status = NumberOfPresets::Get(mEndpoint, &numPresets);
    SuccessOrExit(StatusIB(status).ToChipError());
    VerifyOrExit(newlist.size() <= numPresets, status = imcode::ResourceExhausted);

    // For all exisiting presets -- Walk the old list
    for (auto & old_preset : oldlist)
    {
        VerifyOrDie(old_preset.presetHandle.IsNull() == false);

        // Check 1. -- for each existing built in preset, make sure it's still in the new list
        if (old_preset.builtIn.IsNull() == false && old_preset.builtIn.Value())
        {
            status = FindPresetByHandle(old_preset.presetHandle.Value(), newlist, queryPreset);
            VerifyOrExit(status == imcode::Success, status = imcode::ConstraintError);
            VerifyOrExit(queryPreset.builtIn.IsNull() == false, status = imcode::UnsupportedAccess);
            VerifyOrExit(queryPreset.builtIn.Value() == true, status = imcode::UnsupportedAccess);
        }

        // Check 2 and 3 and 4. -- If the preset is currently being referenced but would be deleted.
        // if its a builtin preset we don't need to search again, we know it's there from the above check.
        if ((old_preset.builtIn.IsNull() || old_preset.builtIn.Value() == false) &&
            IsPresetHandleReferenced(*this, old_preset.presetHandle.Value()))
        {
            status = FindPresetByHandle(old_preset.presetHandle.Value(), newlist, queryPreset);
            VerifyOrExit(status == imcode::Success, status = imcode::InvalidInState);
        }
    }

    // Walk the new list
    for (auto & new_preset : newlist)
    {
        if (new_preset.presetHandle.IsNull() == false && new_preset.presetHandle.Value().empty() == false)
        {
            // Existing Preset checks

            // Make sure it's unique to the list
            status = CheckPresetHandleUnique(new_preset.presetHandle.Value(), newlist);
            SuccessOrExit(StatusIB(status).ToChipError());

            // Look for it in the old list
            PresetStruct::Type existingPreset;
            status = FindPresetByHandle(new_preset.presetHandle.Value(), oldlist, existingPreset);
            SuccessOrExit(StatusIB(status).ToChipError());

            // Check BuiltIn
            VerifyOrExit(new_preset.builtIn == existingPreset.builtIn, status = imcode::UnsupportedAccess);
        }
        else
        {
            // new preset checks
            VerifyOrExit(new_preset.builtIn == false, status = imcode::ConstraintError);
        }

        // Check for Preset Scenario in Preset Types and that the Name support is valid (3 and 4)
        status = CheckPresetType(*this, new_preset);
        SuccessOrExit(StatusIB(status).ToChipError());
    }

exit:
    return status;
}
#endif