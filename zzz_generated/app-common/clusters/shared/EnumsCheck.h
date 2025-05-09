// DO NOT EDIT MANUALLY - Generated file
//
// Generated based on src/controller/data_model/controller-clusters.matter
//
#include <clusters/shared/Enums.h>

namespace chip {
namespace app {
namespace Clusters {

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::AreaTypeTag val)
{
    using EnumType = Globals::AreaTypeTag;
    switch (val)
    {

    case EnumType::kAisle:

    case EnumType::kAttic:

    case EnumType::kBackDoor:

    case EnumType::kBackYard:

    case EnumType::kBalcony:

    case EnumType::kBallroom:

    case EnumType::kBathroom:

    case EnumType::kBedroom:

    case EnumType::kBorder:

    case EnumType::kBoxroom:

    case EnumType::kBreakfastRoom:

    case EnumType::kCarport:

    case EnumType::kCellar:

    case EnumType::kCloakroom:

    case EnumType::kCloset:

    case EnumType::kConservatory:

    case EnumType::kCorridor:

    case EnumType::kCraftRoom:

    case EnumType::kCupboard:

    case EnumType::kDeck:

    case EnumType::kDen:

    case EnumType::kDining:

    case EnumType::kDrawingRoom:

    case EnumType::kDressingRoom:

    case EnumType::kDriveway:

    case EnumType::kElevator:

    case EnumType::kEnsuite:

    case EnumType::kEntrance:

    case EnumType::kEntryway:

    case EnumType::kFamilyRoom:

    case EnumType::kFoyer:

    case EnumType::kFrontDoor:

    case EnumType::kFrontYard:

    case EnumType::kGameRoom:

    case EnumType::kGarage:

    case EnumType::kGarageDoor:

    case EnumType::kGarden:

    case EnumType::kGardenDoor:

    case EnumType::kGuestBathroom:

    case EnumType::kGuestBedroom:

    case EnumType::kGuestRoom:

    case EnumType::kGym:

    case EnumType::kHallway:

    case EnumType::kHearthRoom:

    case EnumType::kKidsRoom:

    case EnumType::kKidsBedroom:

    case EnumType::kKitchen:

    case EnumType::kLaundryRoom:

    case EnumType::kLawn:

    case EnumType::kLibrary:

    case EnumType::kLivingRoom:

    case EnumType::kLounge:

    case EnumType::kMediaTVRoom:

    case EnumType::kMudRoom:

    case EnumType::kMusicRoom:

    case EnumType::kNursery:

    case EnumType::kOffice:

    case EnumType::kOutdoorKitchen:

    case EnumType::kOutside:

    case EnumType::kPantry:

    case EnumType::kParkingLot:

    case EnumType::kParlor:

    case EnumType::kPatio:

    case EnumType::kPlayRoom:

    case EnumType::kPoolRoom:

    case EnumType::kPorch:

    case EnumType::kPrimaryBathroom:

    case EnumType::kPrimaryBedroom:

    case EnumType::kRamp:

    case EnumType::kReceptionRoom:

    case EnumType::kRecreationRoom:

    case EnumType::kRoof:

    case EnumType::kSauna:

    case EnumType::kScullery:

    case EnumType::kSewingRoom:

    case EnumType::kShed:

    case EnumType::kSideDoor:

    case EnumType::kSideYard:

    case EnumType::kSittingRoom:

    case EnumType::kSnug:

    case EnumType::kSpa:

    case EnumType::kStaircase:

    case EnumType::kSteamRoom:

    case EnumType::kStorageRoom:

    case EnumType::kStudio:

    case EnumType::kStudy:

    case EnumType::kSunRoom:

    case EnumType::kSwimmingPool:

    case EnumType::kTerrace:

    case EnumType::kUtilityRoom:

    case EnumType::kWard:

    case EnumType::kWorkshop:

    case EnumType::kToilet:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::AtomicRequestTypeEnum val)
{
    using EnumType = Globals::AtomicRequestTypeEnum;
    switch (val)
    {

    case EnumType::kBeginWrite:

    case EnumType::kCommitWrite:

    case EnumType::kRollbackWrite:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::ChangeIndicationEnum val)
{
    using EnumType = detail::ChangeIndicationEnum;
    switch (val)
    {

    case EnumType::kOK:

    case EnumType::kWarning:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::ChangeIndicationEnum val)
{
    using EnumType = detail::ChangeIndicationEnum;
    switch (val)
    {

    case EnumType::kOK:

    case EnumType::kWarning:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::DegradationDirectionEnum val)
{
    using EnumType = detail::DegradationDirectionEnum;
    switch (val)
    {

    case EnumType::kUp:

    case EnumType::kDown:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::DegradationDirectionEnum val)
{
    using EnumType = detail::DegradationDirectionEnum;
    switch (val)
    {

    case EnumType::kUp:

    case EnumType::kDown:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::LandmarkTag val)
{
    using EnumType = Globals::LandmarkTag;
    switch (val)
    {

    case EnumType::kAirConditioner:

    case EnumType::kAirPurifier:

    case EnumType::kBackDoor:

    case EnumType::kBarStool:

    case EnumType::kBathMat:

    case EnumType::kBathtub:

    case EnumType::kBed:

    case EnumType::kBookshelf:

    case EnumType::kChair:

    case EnumType::kChristmasTree:

    case EnumType::kCoatRack:

    case EnumType::kCoffeeTable:

    case EnumType::kCookingRange:

    case EnumType::kCouch:

    case EnumType::kCountertop:

    case EnumType::kCradle:

    case EnumType::kCrib:

    case EnumType::kDesk:

    case EnumType::kDiningTable:

    case EnumType::kDishwasher:

    case EnumType::kDoor:

    case EnumType::kDresser:

    case EnumType::kLaundryDryer:

    case EnumType::kFan:

    case EnumType::kFireplace:

    case EnumType::kFreezer:

    case EnumType::kFrontDoor:

    case EnumType::kHighChair:

    case EnumType::kKitchenIsland:

    case EnumType::kLamp:

    case EnumType::kLitterBox:

    case EnumType::kMirror:

    case EnumType::kNightstand:

    case EnumType::kOven:

    case EnumType::kPetBed:

    case EnumType::kPetBowl:

    case EnumType::kPetCrate:

    case EnumType::kRefrigerator:

    case EnumType::kScratchingPost:

    case EnumType::kShoeRack:

    case EnumType::kShower:

    case EnumType::kSideDoor:

    case EnumType::kSink:

    case EnumType::kSofa:

    case EnumType::kStove:

    case EnumType::kTable:

    case EnumType::kToilet:

    case EnumType::kTrashCan:

    case EnumType::kLaundryWasher:

    case EnumType::kWindow:

    case EnumType::kWineCooler:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::LevelValueEnum val)
{
    using EnumType = detail::LevelValueEnum;
    switch (val)
    {

    case EnumType::kUnknown:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

    case EnumType::kCritical:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::LocationTag val)
{
    using EnumType = Globals::LocationTag;
    switch (val)
    {

    case EnumType::kIndoor:

    case EnumType::kOutdoor:

    case EnumType::kInside:

    case EnumType::kOutside:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementMediumEnum val)
{
    using EnumType = detail::MeasurementMediumEnum;
    switch (val)
    {

    case EnumType::kAir:

    case EnumType::kWater:

    case EnumType::kSoil:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::MeasurementTypeEnum val)
{
    using EnumType = Globals::MeasurementTypeEnum;
    switch (val)
    {

    case EnumType::kUnspecified:

    case EnumType::kVoltage:

    case EnumType::kActiveCurrent:

    case EnumType::kReactiveCurrent:

    case EnumType::kApparentCurrent:

    case EnumType::kActivePower:

    case EnumType::kReactivePower:

    case EnumType::kApparentPower:

    case EnumType::kRMSVoltage:

    case EnumType::kRMSCurrent:

    case EnumType::kRMSPower:

    case EnumType::kFrequency:

    case EnumType::kPowerFactor:

    case EnumType::kNeutralCurrent:

    case EnumType::kElectricalEnergy:

    case EnumType::kReactiveEnergy:

    case EnumType::kApparentEnergy:

    case EnumType::kSoilMoisture:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementTypeEnum val)
{
    using EnumType = detail::MeasurementTypeEnum;
    switch (val)
    {

    case EnumType::kUnspecified:

    case EnumType::kVoltage:

    case EnumType::kActiveCurrent:

    case EnumType::kReactiveCurrent:

    case EnumType::kApparentCurrent:

    case EnumType::kActivePower:

    case EnumType::kReactivePower:

    case EnumType::kApparentPower:

    case EnumType::kRMSVoltage:

    case EnumType::kRMSCurrent:

    case EnumType::kRMSPower:

    case EnumType::kFrequency:

    case EnumType::kPowerFactor:

    case EnumType::kNeutralCurrent:

    case EnumType::kElectricalEnergy:

    case EnumType::kReactiveEnergy:

    case EnumType::kApparentEnergy:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementTypeEnum val)
{
    using EnumType = detail::MeasurementTypeEnum;
    switch (val)
    {

    case EnumType::kUnspecified:

    case EnumType::kVoltage:

    case EnumType::kActiveCurrent:

    case EnumType::kReactiveCurrent:

    case EnumType::kApparentCurrent:

    case EnumType::kActivePower:

    case EnumType::kReactivePower:

    case EnumType::kApparentPower:

    case EnumType::kRMSVoltage:

    case EnumType::kRMSCurrent:

    case EnumType::kRMSPower:

    case EnumType::kFrequency:

    case EnumType::kPowerFactor:

    case EnumType::kNeutralCurrent:

    case EnumType::kElectricalEnergy:

    case EnumType::kReactiveEnergy:

    case EnumType::kApparentEnergy:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementTypeEnum val)
{
    using EnumType = detail::MeasurementTypeEnum;
    switch (val)
    {

    case EnumType::kUnspecified:

    case EnumType::kVoltage:

    case EnumType::kActiveCurrent:

    case EnumType::kReactiveCurrent:

    case EnumType::kApparentCurrent:

    case EnumType::kActivePower:

    case EnumType::kReactivePower:

    case EnumType::kApparentPower:

    case EnumType::kRMSVoltage:

    case EnumType::kRMSCurrent:

    case EnumType::kRMSPower:

    case EnumType::kFrequency:

    case EnumType::kPowerFactor:

    case EnumType::kNeutralCurrent:

    case EnumType::kElectricalEnergy:

    case EnumType::kReactiveEnergy:

    case EnumType::kApparentEnergy:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::MeasurementUnitEnum val)
{
    using EnumType = detail::MeasurementUnitEnum;
    switch (val)
    {

    case EnumType::kPPM:

    case EnumType::kPPB:

    case EnumType::kPPT:

    case EnumType::kMGM3:

    case EnumType::kUGM3:

    case EnumType::kNGM3:

    case EnumType::kPM3:

    case EnumType::kBQM3:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::PositionTag val)
{
    using EnumType = Globals::PositionTag;
    switch (val)
    {

    case EnumType::kLeft:

    case EnumType::kRight:

    case EnumType::kTop:

    case EnumType::kBottom:

    case EnumType::kMiddle:

    case EnumType::kRow:

    case EnumType::kColumn:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::PowerThresholdSourceEnum val)
{
    using EnumType = Globals::PowerThresholdSourceEnum;
    switch (val)
    {

    case EnumType::kContract:

    case EnumType::kRegulator:

    case EnumType::kEquipment:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::ProductIdentifierTypeEnum val)
{
    using EnumType = detail::ProductIdentifierTypeEnum;
    switch (val)
    {

    case EnumType::kUPC:

    case EnumType::kGTIN8:

    case EnumType::kEAN:

    case EnumType::kGTIN14:

    case EnumType::kOEM:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::ProductIdentifierTypeEnum val)
{
    using EnumType = detail::ProductIdentifierTypeEnum;
    switch (val)
    {

    case EnumType::kUPC:

    case EnumType::kGTIN8:

    case EnumType::kEAN:

    case EnumType::kGTIN14:

    case EnumType::kOEM:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::RelativePositionTag val)
{
    using EnumType = Globals::RelativePositionTag;
    switch (val)
    {

    case EnumType::kUnder:

    case EnumType::kNextTo:

    case EnumType::kAround:

    case EnumType::kOn:

    case EnumType::kAbove:

    case EnumType::kFrontOf:

    case EnumType::kBehind:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::StreamUsageEnum val)
{
    using EnumType = detail::StreamUsageEnum;
    switch (val)
    {

    case EnumType::kInternal:

    case EnumType::kRecording:

    case EnumType::kAnalysis:

    case EnumType::kLiveView:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::StreamUsageEnum val)
{
    using EnumType = detail::StreamUsageEnum;
    switch (val)
    {

    case EnumType::kInternal:

    case EnumType::kRecording:

    case EnumType::kAnalysis:

    case EnumType::kLiveView:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::StreamUsageEnum val)
{
    using EnumType = detail::StreamUsageEnum;
    switch (val)
    {

    case EnumType::kInternal:

    case EnumType::kRecording:

    case EnumType::kAnalysis:

    case EnumType::kLiveView:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::StreamUsageEnum val)
{
    using EnumType = detail::StreamUsageEnum;
    switch (val)
    {

    case EnumType::kInternal:

    case EnumType::kRecording:

    case EnumType::kAnalysis:

    case EnumType::kLiveView:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::TariffPriceTypeEnum val)
{
    using EnumType = Globals::TariffPriceTypeEnum;
    switch (val)
    {

    case EnumType::kStandard:

    case EnumType::kCritical:

    case EnumType::kVirtual:

    case EnumType::kIncentive:

    case EnumType::kIncentiveSignal:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::TariffUnitEnum val)
{
    using EnumType = Globals::TariffUnitEnum;
    switch (val)
    {

    case EnumType::kKWh:

    case EnumType::kKVAh:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::TestGlobalEnum val)
{
    using EnumType = Globals::TestGlobalEnum;
    switch (val)
    {

    case EnumType::kSomeValue:

    case EnumType::kSomeOtherValue:

    case EnumType::kFinalValue:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(Globals::ThreeLevelAutoEnum val)
{
    using EnumType = Globals::ThreeLevelAutoEnum;
    switch (val)
    {

    case EnumType::kAuto:

    case EnumType::kLow:

    case EnumType::kMedium:

    case EnumType::kHigh:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::WebRTCEndReasonEnum val)
{
    using EnumType = detail::WebRTCEndReasonEnum;
    switch (val)
    {

    case EnumType::kIceFailed:

    case EnumType::kIceTimeout:

    case EnumType::kUserHangup:

    case EnumType::kUserBusy:

    case EnumType::kReplaced:

    case EnumType::kNoUserMedia:

    case EnumType::kInviteTimeout:

    case EnumType::kAnsweredElsewhere:

    case EnumType::kOutOfResources:

    case EnumType::kMediaTimeout:

    case EnumType::kLowPower:

    case EnumType::kUnknownReason:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

static auto __attribute__((unused)) EnsureKnownEnumValue(detail::WebRTCEndReasonEnum val)
{
    using EnumType = detail::WebRTCEndReasonEnum;
    switch (val)
    {

    case EnumType::kIceFailed:

    case EnumType::kIceTimeout:

    case EnumType::kUserHangup:

    case EnumType::kUserBusy:

    case EnumType::kReplaced:

    case EnumType::kNoUserMedia:

    case EnumType::kInviteTimeout:

    case EnumType::kAnsweredElsewhere:

    case EnumType::kOutOfResources:

    case EnumType::kMediaTimeout:

    case EnumType::kLowPower:

    case EnumType::kUnknownReason:

        return val;
    default:
        return EnumType::kUnknownEnumValue;
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
