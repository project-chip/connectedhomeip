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

static imcode FindScheduleByHandle(const chip::ByteSpan & handle, const Span<ScheduleStruct::Type> & list,
                                          ScheduleStruct::Type & outSchedule)
{
    for (auto & schedule : list)
    {
        if ((schedule.scheduleHandle.IsNull() == false) && handle.data_equal(schedule.scheduleHandle.Value()))
        {
            outSchedule = schedule;
            return imcode::Success;
        }
    }

    return imcode::NotFound;
}

static imcode CheckScheduleHandleUnique(const chip::ByteSpan & handle, Span<ScheduleStruct::Type> & list)
{
    int count = 0;
    for (auto & schedule : list)
    {
        if ((schedule.scheduleHandle.IsNull() == false) && handle.data_equal(schedule.scheduleHandle.Value()))
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

static bool IsScheduleHandleReferenced(ThermostatMatterScheduleManager & mgr, const chip::ByteSpan & handle)
{
    imcode status = imcode::Success;

    // Check Active Preset Handle
    DataModel::Nullable<chip::MutableByteSpan> activeScheduleHandle;
    status = ActiveScheduleHandle::Get(mgr.mEndpoint, activeScheduleHandle);
    VerifyOrDie(status == imcode::Success);
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
    ThermostatSystemModeEnum systemMode = static_cast<ThermostatSystemModeEnum>(0);
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
    ThermostatSystemModeEnum systemMode = static_cast<ThermostatSystemModeEnum>(0);
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
    ThermostatSystemModeEnum systemMode                                = static_cast<ThermostatSystemModeEnum>(0);
    uint8_t numberOfSchedules                                      = static_cast<uint8_t>(0);
    chip::BitMask<ScheduleTypeFeaturesBitmap> scheduleTypeFeatures = static_cast<chip::BitMask<ScheduleTypeFeaturesBitmap>>(0);

    CHIP_ERROR Decode(TLV::TLVReader & reader);

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;
};

using DecodableType = Type;

} // namespace ScheduleTypeStruct
#endif

static imcode CheckScheduleTypes(ThermostatMatterScheduleManager & mgr, ScheduleStruct::Type & schedule, Span<PresetStruct::Type> & presetList)
{
    imcode status = imcode::ConstraintError;
    size_t index         = 0;
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
                VerifyOrReturnError(presetsSupported == true, imcode::ConstraintError);

                // make sure the preset exists (check 7)
                {
                    size_t preset_index = 0;
                    bool presetFound = false;
                    while (preset_index < presetList.size())
                    {
                        auto &preset = presetList[preset_index];
                        VerifyOrDie(preset.presetHandle.IsNull() == false);
                        if (preset.presetHandle.Value().data_equal(schedule.presetHandle.Value()))
                        {
                            presetFound = true;
                            break;
                        }
                        preset_index++;
                    }
                    VerifyOrReturnError(presetFound == true, imcode::ConstraintError);
                }
            }

            // Check the name requirements (check 8)
            if (schedule.name.HasValue() == false && schedule.name.Value().empty() == false)
            {
                const bool nameSupported = scheduleType.scheduleTypeFeatures.Has(ScheduleTypeFeaturesBitmap::kSupportsNames);
                VerifyOrReturnError(nameSupported == true, imcode::ConstraintError);
            }

            // Check the off requirements (check 9)
            if (schedule.systemMode == SystemModeEnum::kOff)
            {
                const bool offSupported = scheduleType.scheduleTypeFeatures.Has(ScheduleTypeFeaturesBitmap::kSupportsOff);
                VerifyOrReturnError(offSupported == true, imcode::ConstraintError);
            }

            return imcode::Success;
        }
        index++;
    }

    return status;
}

static imcode CheckNumberOfTransitions(ThermostatMatterScheduleManager & mgr, ScheduleStruct::Type & schedule)
{
    imcode status = imcode::ConstraintError;

    // Check 4
    uint8_t numberOfScheduleTransitions;
    DataModel::Nullable<uint8_t> numberOfScheduleTransitionsPerDay;

    status = NumberOfScheduleTransitions::Get(mgr.mEndpoint, &numberOfScheduleTransitions);
    VerifyOrDie(status == imcode::Success);
    VerifyOrExit(schedule.transitions.size() <= numberOfScheduleTransitions, status = imcode::ResourceExhausted);

    status = NumberOfScheduleTransitionPerDay::Get(mgr.mEndpoint, numberOfScheduleTransitionsPerDay);
    VerifyOrDie(status == imcode::Success);

    if (numberOfScheduleTransitionsPerDay.IsNull() == false)
    {
        int weekDayTransitionCounts[7] = { 0 };

        for (auto const &transition : schedule.transitions)
        {
            weekDayTransitionCounts[0] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kSunday) ? 1 : 0;
            weekDayTransitionCounts[1] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kMonday) ? 1 : 0;
            weekDayTransitionCounts[2] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kTuesday) ? 1 : 0;
            weekDayTransitionCounts[3] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kWednesday) ? 1 : 0;
            weekDayTransitionCounts[4] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kThursday) ? 1 : 0;
            weekDayTransitionCounts[5] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kFriday) ? 1 : 0;
            weekDayTransitionCounts[6] += transition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kSaturday) ? 1 : 0;
        }

        for (int weekDayIndex = 0; weekDayIndex < 7; weekDayIndex++)
        {
            VerifyOrExit(weekDayTransitionCounts[weekDayIndex] <= numberOfScheduleTransitionsPerDay.Value(),
                         status = imcode::ResourceExhausted);
        }
    }

exit:
    return status;
}

imcode ThermostatMatterScheduleManager::ValidateSchedulesForCommitting(Span<ScheduleStruct::Type> & oldlist,
                                                                              Span<ScheduleStruct::Type> & newlist,
                                                                              Span<PresetStruct::Type> & presetlist)
{
    imcode status = imcode::Success;
    ScheduleStruct::Type querySchedule; // manager storage used for queries.

    // Check that new_list can fit.
    uint8_t numSchedules;
    status = NumberOfSchedules::Get(mEndpoint, &numSchedules);
    SuccessOrExit(StatusIB(status).ToChipError());
    VerifyOrExit(newlist.size() <= numSchedules, status = imcode::ResourceExhausted);

    // For all exisiting schedules -- Walk the old list
    for (auto & old_schedule : oldlist)
    {
        VerifyOrDie(old_schedule.scheduleHandle.IsNull() == false);

        // Check 1. -- for each existing built in schedule, make sure it's still in the new list
        if ((old_schedule.builtIn.IsNull() == false) && (old_schedule.builtIn.Value() == true))
        {
            status = FindScheduleByHandle(old_schedule.scheduleHandle.Value(), newlist, querySchedule);
            VerifyOrExit(status == imcode::Success, status = imcode::ConstraintError);
            VerifyOrExit(querySchedule.builtIn.IsNull() == false, status = imcode::UnsupportedAccess);
            VerifyOrExit(querySchedule.builtIn.Value() == true, status = imcode::UnsupportedAccess);
        }

        // Check 2 -- If the schedule is currently being referenced but would be deleted.
        // if its a builtin schedule we don't need to search again, we know it's there from the above check.
        if ((old_schedule.builtIn.IsNull() == true || old_schedule.builtIn.Value() == false) && IsScheduleHandleReferenced(*this, old_schedule.scheduleHandle.Value()))
        {
            VerifyOrDie(old_schedule.scheduleHandle.IsNull() == false);
            status = FindScheduleByHandle(old_schedule.scheduleHandle.Value(), newlist, querySchedule);
            VerifyOrExit(status == imcode::Success, status = imcode::InvalidInState);
        }
    }

    // Walk the new list
    for (auto & new_schedule : newlist)
    {
        if (new_schedule.scheduleHandle.IsNull() == false)
        {
            // Existing schedule checks

            // Make sure it's unique to the list
            status = CheckScheduleHandleUnique(new_schedule.scheduleHandle.Value(), newlist);
            SuccessOrExit(StatusIB(status).ToChipError());

            // Look for it in the old list
            ScheduleStruct::Type existingSchedule;
            status = FindScheduleByHandle(new_schedule.scheduleHandle.Value(), oldlist, existingSchedule);
            SuccessOrExit(StatusIB(status).ToChipError());

            // Check BuiltIn
            VerifyOrExit(new_schedule.builtIn.IsNull() == existingSchedule.builtIn.IsNull(), status = imcode::UnsupportedAccess);
            if (new_schedule.builtIn.IsNull() == false)
            {
                VerifyOrExit(new_schedule.builtIn.Value() == existingSchedule.builtIn.Value(), status = imcode::UnsupportedAccess);
            }
        }
        else
        {
            // new schedule checks
            VerifyOrExit((new_schedule.builtIn.IsNull() == true) || (new_schedule.builtIn.Value() == false), status = imcode::ConstraintError);
        }

        // Check for system mode in Schedule Types
        status = CheckScheduleTypes(*this, new_schedule, presetlist);
        SuccessOrExit(StatusIB(status).ToChipError());

        // Make sure the number of transitions does not exceed out limits
        status = CheckNumberOfTransitions(*this, new_schedule);
        SuccessOrExit(StatusIB(status).ToChipError());
    }

exit:
    return status;
}
