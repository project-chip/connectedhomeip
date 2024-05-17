namespace DemandResponseLoadControl {
namespace Structs {

namespace HeatingSourceControlStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kHeatingSource), heatingSource);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kHeatingSource))
        {
            err = DataModel::Decode(reader, heatingSource);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace HeatingSourceControlStruct

namespace PowerSavingsControlStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kPowerSavings), powerSavings);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kPowerSavings))
        {
            err = DataModel::Decode(reader, powerSavings);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace PowerSavingsControlStruct

namespace DutyCycleControlStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kDutyCycle), dutyCycle);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kDutyCycle))
        {
            err = DataModel::Decode(reader, dutyCycle);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace DutyCycleControlStruct

namespace AverageLoadControlStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kLoadAdjustment), loadAdjustment);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kLoadAdjustment))
        {
            err = DataModel::Decode(reader, loadAdjustment);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace AverageLoadControlStruct

namespace TemperatureControlStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kCoolingTempOffset), coolingTempOffset);
    encoder.Encode(to_underlying(Fields::kHeatingtTempOffset), heatingtTempOffset);
    encoder.Encode(to_underlying(Fields::kCoolingTempSetpoint), coolingTempSetpoint);
    encoder.Encode(to_underlying(Fields::kHeatingTempSetpoint), heatingTempSetpoint);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kCoolingTempOffset))
        {
            err = DataModel::Decode(reader, coolingTempOffset);
        }
        else if (__context_tag == to_underlying(Fields::kHeatingtTempOffset))
        {
            err = DataModel::Decode(reader, heatingtTempOffset);
        }
        else if (__context_tag == to_underlying(Fields::kCoolingTempSetpoint))
        {
            err = DataModel::Decode(reader, coolingTempSetpoint);
        }
        else if (__context_tag == to_underlying(Fields::kHeatingTempSetpoint))
        {
            err = DataModel::Decode(reader, heatingTempSetpoint);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace TemperatureControlStruct

namespace LoadControlEventTransitionStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kDuration), duration);
    encoder.Encode(to_underlying(Fields::kControl), control);
    encoder.Encode(to_underlying(Fields::kTemperatureControl), temperatureControl);
    encoder.Encode(to_underlying(Fields::kAverageLoadControl), averageLoadControl);
    encoder.Encode(to_underlying(Fields::kDutyCycleControl), dutyCycleControl);
    encoder.Encode(to_underlying(Fields::kPowerSavingsControl), powerSavingsControl);
    encoder.Encode(to_underlying(Fields::kHeatingSourceControl), heatingSourceControl);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kDuration))
        {
            err = DataModel::Decode(reader, duration);
        }
        else if (__context_tag == to_underlying(Fields::kControl))
        {
            err = DataModel::Decode(reader, control);
        }
        else if (__context_tag == to_underlying(Fields::kTemperatureControl))
        {
            err = DataModel::Decode(reader, temperatureControl);
        }
        else if (__context_tag == to_underlying(Fields::kAverageLoadControl))
        {
            err = DataModel::Decode(reader, averageLoadControl);
        }
        else if (__context_tag == to_underlying(Fields::kDutyCycleControl))
        {
            err = DataModel::Decode(reader, dutyCycleControl);
        }
        else if (__context_tag == to_underlying(Fields::kPowerSavingsControl))
        {
            err = DataModel::Decode(reader, powerSavingsControl);
        }
        else if (__context_tag == to_underlying(Fields::kHeatingSourceControl))
        {
            err = DataModel::Decode(reader, heatingSourceControl);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace LoadControlEventTransitionStruct

namespace LoadControlEventStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kEventID), eventID);
    encoder.Encode(to_underlying(Fields::kProgramID), programID);
    encoder.Encode(to_underlying(Fields::kControl), control);
    encoder.Encode(to_underlying(Fields::kDeviceClass), deviceClass);
    encoder.Encode(to_underlying(Fields::kEnrollmentGroup), enrollmentGroup);
    encoder.Encode(to_underlying(Fields::kCriticality), criticality);
    encoder.Encode(to_underlying(Fields::kStartTime), startTime);
    encoder.Encode(to_underlying(Fields::kTransitions), transitions);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kEventID))
        {
            err = DataModel::Decode(reader, eventID);
        }
        else if (__context_tag == to_underlying(Fields::kProgramID))
        {
            err = DataModel::Decode(reader, programID);
        }
        else if (__context_tag == to_underlying(Fields::kControl))
        {
            err = DataModel::Decode(reader, control);
        }
        else if (__context_tag == to_underlying(Fields::kDeviceClass))
        {
            err = DataModel::Decode(reader, deviceClass);
        }
        else if (__context_tag == to_underlying(Fields::kEnrollmentGroup))
        {
            err = DataModel::Decode(reader, enrollmentGroup);
        }
        else if (__context_tag == to_underlying(Fields::kCriticality))
        {
            err = DataModel::Decode(reader, criticality);
        }
        else if (__context_tag == to_underlying(Fields::kStartTime))
        {
            err = DataModel::Decode(reader, startTime);
        }
        else if (__context_tag == to_underlying(Fields::kTransitions))
        {
            err = DataModel::Decode(reader, transitions);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace LoadControlEventStruct

namespace LoadControlProgramStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kProgramID), programID);
    encoder.Encode(to_underlying(Fields::kName), name);
    encoder.Encode(to_underlying(Fields::kEnrollmentGroup), enrollmentGroup);
    encoder.Encode(to_underlying(Fields::kRandomStartMinutes), randomStartMinutes);
    encoder.Encode(to_underlying(Fields::kRandomDurationMinutes), randomDurationMinutes);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kProgramID))
        {
            err = DataModel::Decode(reader, programID);
        }
        else if (__context_tag == to_underlying(Fields::kName))
        {
            err = DataModel::Decode(reader, name);
        }
        else if (__context_tag == to_underlying(Fields::kEnrollmentGroup))
        {
            err = DataModel::Decode(reader, enrollmentGroup);
        }
        else if (__context_tag == to_underlying(Fields::kRandomStartMinutes))
        {
            err = DataModel::Decode(reader, randomStartMinutes);
        }
        else if (__context_tag == to_underlying(Fields::kRandomDurationMinutes))
        {
            err = DataModel::Decode(reader, randomDurationMinutes);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}

} // namespace LoadControlProgramStruct
} // namespace Structs

namespace Commands {
namespace RegisterLoadControlProgramRequest {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kLoadControlProgram), loadControlProgram);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kLoadControlProgram))
        {
            err = DataModel::Decode(reader, loadControlProgram);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}
} // namespace RegisterLoadControlProgramRequest.
namespace UnregisterLoadControlProgramRequest {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kLoadControlProgramID), loadControlProgramID);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kLoadControlProgramID))
        {
            err = DataModel::Decode(reader, loadControlProgramID);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}
} // namespace UnregisterLoadControlProgramRequest.
namespace AddLoadControlEventRequest {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kEvent), event);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kEvent))
        {
            err = DataModel::Decode(reader, event);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}
} // namespace AddLoadControlEventRequest.
namespace RemoveLoadControlEventRequest {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    encoder.Encode(to_underlying(Fields::kEventID), eventID);
    encoder.Encode(to_underlying(Fields::kCancelControl), cancelControl);
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kEventID))
        {
            err = DataModel::Decode(reader, eventID);
        }
        else if (__context_tag == to_underlying(Fields::kCancelControl))
        {
            err = DataModel::Decode(reader, cancelControl);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}
} // namespace RemoveLoadControlEventRequest.
namespace ClearLoadControlEventsRequest {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    DataModel::WrappedStructEncoder encoder{ aWriter, aTag };
    return encoder.Finalize();
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }
    }
}
} // namespace ClearLoadControlEventsRequest.
} // namespace Commands

namespace Attributes {
CHIP_ERROR TypeInfo::DecodableType::Decode(TLV::TLVReader & reader, const ConcreteAttributePath & path)
{
    switch (path.mAttributeId)
    {
    case Attributes::DeviceClass::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, deviceClass);
    case Attributes::LoadControlPrograms::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, loadControlPrograms);
    case Attributes::NumberOfLoadControlPrograms::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, numberOfLoadControlPrograms);
    case Attributes::Events::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, events);
    case Attributes::ActiveEvents::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, activeEvents);
    case Attributes::NumberOfEventsPerProgram::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, numberOfEventsPerProgram);
    case Attributes::NumberOfTransistions::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, numberOfTransistions);
    case Attributes::DefaultRandomStart::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, defaultRandomStart);
    case Attributes::DefaultRandomDuration::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, defaultRandomDuration);
    case Attributes::GeneratedCommandList::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, generatedCommandList);
    case Attributes::AcceptedCommandList::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, acceptedCommandList);
    case Attributes::EventList::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, eventList);
    case Attributes::AttributeList::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, attributeList);
    case Attributes::FeatureMap::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, featureMap);
    case Attributes::ClusterRevision::TypeInfo::GetAttributeId():
        return DataModel::Decode(reader, clusterRevision);
    default:
        return CHIP_NO_ERROR;
    }
}
} // namespace Attributes

namespace Events {
namespace LoadControlEventStatusChange {
CHIP_ERROR Type::Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(aWriter.StartContainer(aTag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kEventID), eventID));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kTransitionIndex), transitionIndex));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kStatus), status));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kCriticality), criticality));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kControl), control));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kTemperatureControl), temperatureControl));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kAverageLoadControl), averageLoadControl));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kDutyCycleControl), dutyCycleControl));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kPowerSavingsControl), powerSavingsControl));
    ReturnErrorOnFailure(DataModel::Encode(aWriter, TLV::ContextTag(Fields::kHeatingSourceControl), heatingSourceControl));
    return aWriter.EndContainer(outer);
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    detail::StructDecodeIterator __iterator(reader);
    while (true)
    {
        auto __element = __iterator.Next();
        if (std::holds_alternative<CHIP_ERROR>(__element))
        {
            return std::get<CHIP_ERROR>(__element);
        }

        CHIP_ERROR err              = CHIP_NO_ERROR;
        const uint8_t __context_tag = std::get<uint8_t>(__element);

        if (__context_tag == to_underlying(Fields::kEventID))
        {
            err = DataModel::Decode(reader, eventID);
        }
        else if (__context_tag == to_underlying(Fields::kTransitionIndex))
        {
            err = DataModel::Decode(reader, transitionIndex);
        }
        else if (__context_tag == to_underlying(Fields::kStatus))
        {
            err = DataModel::Decode(reader, status);
        }
        else if (__context_tag == to_underlying(Fields::kCriticality))
        {
            err = DataModel::Decode(reader, criticality);
        }
        else if (__context_tag == to_underlying(Fields::kControl))
        {
            err = DataModel::Decode(reader, control);
        }
        else if (__context_tag == to_underlying(Fields::kTemperatureControl))
        {
            err = DataModel::Decode(reader, temperatureControl);
        }
        else if (__context_tag == to_underlying(Fields::kAverageLoadControl))
        {
            err = DataModel::Decode(reader, averageLoadControl);
        }
        else if (__context_tag == to_underlying(Fields::kDutyCycleControl))
        {
            err = DataModel::Decode(reader, dutyCycleControl);
        }
        else if (__context_tag == to_underlying(Fields::kPowerSavingsControl))
        {
            err = DataModel::Decode(reader, powerSavingsControl);
        }
        else if (__context_tag == to_underlying(Fields::kHeatingSourceControl))
        {
            err = DataModel::Decode(reader, heatingSourceControl);
        }
        else
        {
        }

        ReturnErrorOnFailure(err);
    }
}
} // namespace LoadControlEventStatusChange.
} // namespace Events

