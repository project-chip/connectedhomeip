#include "thermostat-server.h"

#include <app/util/af.h>

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;

static EmberAfStatus
FindScheduleByHandle(const chip::ByteSpan &handle, const Span<ScheduleStruct::Type> &list, ScheduleStruct::Type &outSchedule)
{
	for (auto & schedule : list)
	{
	    if (handle.data_equal(schedule.scheduleHandle))
	    {
	    	outSchedule = schedule;
	    	return EMBER_ZCL_STATUS_SUCCESS;
	    }
	}

	return EMBER_ZCL_STATUS_NOT_FOUND;
}

static EmberAfStatus
CheckScheduleHandleUnique(const chip::ByteSpan &handle, Span<ScheduleStruct::Type> &list)
{
    int count = 0;
    for (auto & schedule : list)
    {
        if (handle.data_equal(schedule.scheduleHandle))
        {
            if (count == 0)
            {
                count++;
            }
            else
            {
            	return EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
            }
        }
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

static bool
IsScheduleHandleReferenced(ThermostatMatterScheduleManager &mgr, const chip::ByteSpan &handle)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

	// Check Active Preset Handle
	DataModel::Nullable<chip::MutableByteSpan> activeScheduleHandle;
	status = ActiveScheduleHandle::Get(mgr.mEndpoint, activeScheduleHandle);
	VerifyOrDie(status == EMBER_ZCL_STATUS_SUCCESS);
	if ((activeScheduleHandle.IsNull() == false) && activeScheduleHandle.Value().data_equal(handle))
		return true;

	return false;
}

#if 0
namespace ScheduleStruct {
enum class Fields : uint8_t
{
    kSceduleHandle = 0,
    kSystemMode    = 1,
    kName          = 2,
    kPresetHandle  = 3,
    kTransitions   = 4,
    kBuiltIn       = 5,
};

struct Type
{
public:
    chip::ByteSpan sceduleHandle;
    ThermostatSystemMode systemMode = static_cast<ThermostatSystemMode>(0);
    DataModel::Nullable<chip::CharSpan> name;
    chip::ByteSpan presetHandle;
    DataModel::List<const Structs::ScheduleTransitionStruct::Type> transitions;
    bool builtIn = static_cast<bool>(0);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

struct DecodableType
{
public:
    chip::ByteSpan sceduleHandle;
    ThermostatSystemMode systemMode = static_cast<ThermostatSystemMode>(0);
    DataModel::Nullable<chip::CharSpan> name;
    chip::ByteSpan presetHandle;
    DataModel::DecodableList<Structs::ScheduleTransitionStruct::DecodableType> transitions;
    bool builtIn = static_cast<bool>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;
};

} // namespace ScheduleStruct

namespace ScheduleTypeStruct {
enum class Fields : uint8_t
{
    kSystemMode           = 0,
    kNumberOfSchedules    = 1,
    kScheduleTypeFeatures = 2,
};

struct Type
{
public:
    ThermostatSystemMode systemMode                                = static_cast<ThermostatSystemMode>(0);
    uint8_t numberOfSchedules                                      = static_cast<uint8_t>(0);
    chip::BitMask<ScheduleTypeFeaturesBitmap> scheduleTypeFeatures = static_cast<chip::BitMask<ScheduleTypeFeaturesBitmap>>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace ScheduleTypeStruct
#endif

static EmberAfStatus
CheckScheduleTypes(ThermostatMatterScheduleManager &mgr, ScheduleStruct::Type &schedule)
{
	EmberAfStatus status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
	size_t index = 0;
	ScheduleTypeStruct::Type scheduleType;

	VerifyOrDie(mgr.mGetScheduleTypeAtIndexCb);
	while (mgr.mGetScheduleTypeAtIndexCb(&mgr, index, scheduleType) != CHIP_ERROR_NOT_FOUND)
	{
		// look for the schedule type that supports this scenario (check 3)
		if (scheduleType.systemMode == schedule.systemMode)
		{
			// we have one, check the preset requirements (check 6)
			if (schedule.presetHandle.HasValue() && schedule.presetHandle.Value().empty() == false)
			{
				const bool presetsSupported = scheduleType.scheduleTypeFeatures.Has(ScheduleTypeFeaturesBitmap::kSupportsPresets);
				VerifyOrReturnError(presetsSupported == true, EMBER_ZCL_STATUS_CONSTRAINT_ERROR);

                // make sure the preset exists (check 7)
                VerifyOrDie(mgr.mGetPresetAtIndexCb);
                {
                    size_t preset_index = 0;
                    PresetStruct::Type preset;
                    bool presetFound = false;
                    while (mgr.mGetPresetAtIndexCb(&mgr, preset_index, preset) != CHIP_ERROR_NOT_FOUND)
                    {
                        VerifyOrDie(preset.presetHandle.IsNull() == false);
                        if (preset.presetHandle.Value().data_equal(schedule.presetHandle.Value()))
                        {
                            presetFound = true;
                            break;
                        }
                        preset_index++;
                    }
                    VerifyOrReturnError(presetFound == true, EMBER_ZCL_STATUS_CONSTRAINT_ERROR);
                }
			}

            // Check the name requirements (check 8)
            if (schedule.name.HasValue() == false && schedule.name.Value().empty() == false)
            {
                const bool nameSupported = scheduleType.scheduleTypeFeatures.Has(ScheduleTypeFeaturesBitmap::kSupportsNames);
                VerifyOrReturnError(nameSupported == true, EMBER_ZCL_STATUS_CONSTRAINT_ERROR);
            }

            // Check the off requirements (check 9)
            if (schedule.systemMode == ThermostatSystemMode::kOff)
            {
                const bool offSupported = scheduleType.scheduleTypeFeatures.Has(ScheduleTypeFeaturesBitmap::kSupportsOff);
                VerifyOrReturnError(offSupported == true, EMBER_ZCL_STATUS_CONSTRAINT_ERROR);
            }

			return EMBER_ZCL_STATUS_SUCCESS;
		}
		index++;
	}

	return status;
}

static EmberAfStatus
CheckNumberOfTransitions(ThermostatMatterScheduleManager &mgr, ScheduleStruct::Type &schedule)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR;

    // Check 4
    uint8_t numberOfScheduleTransitions;
    DataModel::Nullable<uint8_t> numberOfScheduleTransitionsPerDay;

    status = NumberOfScheduleTransitions::Get(mgr.mEndpoint, &numberOfScheduleTransitions);
    VerifyOrDie(status == EMBER_ZCL_STATUS_SUCCESS);
    VerifyOrExit(schedule.transitions.size() <= numberOfScheduleTransitions, status = EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED);

    status = NumberOfScheduleTransitionsPerDay::Get(mgr.mEndpoint, numberOfScheduleTransitionsPerDay);
    VerifyOrDie(status == EMBER_ZCL_STATUS_SUCCESS);

    if (numberOfScheduleTransitionsPerDay.IsNull() == false)
    {
        int weekDayTransitionCounts[7] = {0};

        for (auto transition : schedule.transitions)
        {
            weekDayTransitionCounts[0] += transition.dayOfWeek.Has(DayOfWeek::kSunday) ? 1 : 0;
            weekDayTransitionCounts[1] += transition.dayOfWeek.Has(DayOfWeek::kMonday) ? 1 : 0;
            weekDayTransitionCounts[2] += transition.dayOfWeek.Has(DayOfWeek::kTuesday) ? 1 : 0;
            weekDayTransitionCounts[3] += transition.dayOfWeek.Has(DayOfWeek::kWednesday) ? 1 : 0;
            weekDayTransitionCounts[4] += transition.dayOfWeek.Has(DayOfWeek::kThursday) ? 1 : 0;
            weekDayTransitionCounts[5] += transition.dayOfWeek.Has(DayOfWeek::kFriday) ? 1 : 0;
            weekDayTransitionCounts[6] += transition.dayOfWeek.Has(DayOfWeek::kSaturday) ? 1 : 0;
        }

        for (int weekDayIndex = 0; weekDayIndex < 7; weekDayIndex++)
        {
            VerifyOrExit(weekDayTransitionCounts[weekDayIndex] <= numberOfScheduleTransitionsPerDay.Value(), status = EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED);
        }
    }

exit:
    return status;
}

EmberAfStatus
ThermostatMatterScheduleManager::ValidateSchedulesForCommitting(Span<ScheduleStruct::Type> &oldlist, Span<ScheduleStruct::Type> &newlist)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    ScheduleStruct::Type querySchedule; // manager storage used for queries.

    // Check that new_list can fit.
    uint8_t numSchedules;
    status = NumberOfSchedules::Get(mEndpoint, &numSchedules);
    SuccessOrExit(status);
    VerifyOrExit(newlist.size() <= numSchedules, status = EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED);

    // For all exisiting schedules -- Walk the old list
    for (auto & old_schedule : oldlist)
    {
    	// Check 1. -- for each existing built in schedule, make sure it's still in the new list
    	if (old_schedule.builtIn.HasValue() && old_schedule.builtIn.Value())
    	{
    		status = FindScheduleByHandle(old_schedule.scheduleHandle, newlist, querySchedule);
    		VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR);
    		VerifyOrExit(querySchedule.builtIn == true, status = EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS);
    	}

    	// Check 2 -- If the schedule is currently being referenced but would be deleted.
		// if its a builtin schedule we don't need to search again, we know it's there from the above check.
		if (old_schedule.builtIn == false && IsScheduleHandleReferenced(*this, old_schedule.scheduleHandle))
    	{
    		status = FindScheduleByHandle(old_schedule.scheduleHandle, newlist, querySchedule);
    		VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, status = EMBER_ZCL_STATUS_INVALID_IN_STATE);
    	}
    }

    // Walk the new list
    for (auto & new_schedule : newlist)
    {
        if (new_schedule.scheduleHandle.empty() == false)
        {
            // Existing schedule checks

            // Make sure it's unique to the list
            status = CheckScheduleHandleUnique(new_schedule.scheduleHandle, newlist);
            SuccessOrExit(status);

            // Look for it in the old list
            ScheduleStruct::Type existingSchedule;
            status = FindScheduleByHandle(new_schedule.scheduleHandle, oldlist, existingSchedule);
            SuccessOrExit(status);

	        // Check BuiltIn
	        VerifyOrExit(new_schedule.builtIn == existingSchedule.builtIn, status = EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS);
        }
        else
        {
        	// new schedule checks
        	VerifyOrExit(new_schedule.builtIn == false, status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR);
        }

        // Check for system mode in Schedule Types
        status = CheckScheduleTypes(*this, new_schedule);
        SuccessOrExit(status);

        // Make sure the number of transitions does not exceed out limits
        status = CheckNumberOfTransitions(*this, new_schedule);
        SuccessOrExit(status);

    }

exit:
    return status;
}
