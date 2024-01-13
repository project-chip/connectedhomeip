#include <app/clusters/energy-preference-server/energy-preference-server.h>

using namespace chip;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Structs;

static constexpr const char * kEfficientLabel = "Efficient";
static constexpr const char * kComfortLabel   = "Comfort";

static BalanceStruct::Type gsEnergyBalances[] = {
    { .step = 0, .label = Optional<chip::CharSpan>(chip::CharSpan(kEfficientLabel, strlen(kEfficientLabel))) },
    { .step = 50, .label = Optional<chip::CharSpan>() },
    { .step = 100, .label = Optional<chip::CharSpan>(chip::CharSpan(kComfortLabel, strlen(kComfortLabel))) },
};

static constexpr const char * k1MinuteLabel    = "1 Minute";
static constexpr const char * k5MinutesLabel   = "5 Minutes";
static constexpr const char * k10MinutesLabel  = "10 Minutes";
static constexpr const char * k15MinutesLabel  = "15 Minutes";
static constexpr const char * k20MinutesLabel  = "20 Minutes";
static constexpr const char * k25MinutesLabel  = "25 Minutes";
static constexpr const char * k30MinutesLabel  = "30 Minutes";
static constexpr const char * k60MinutesLabel  = "60 Minutes";
static constexpr const char * k120MinutesLabel = "120 Minutes";
static constexpr const char * kNeverLabel      = "Never";

static BalanceStruct::Type gsPowerBalances[] = {
    { .step = 0, .label = Optional<chip::CharSpan>(chip::CharSpan(k1MinuteLabel, strlen(k1MinuteLabel))) },
    { .step = 12, .label = Optional<chip::CharSpan>(chip::CharSpan(k5MinutesLabel, strlen(k5MinutesLabel))) },
    { .step = 24, .label = Optional<chip::CharSpan>(chip::CharSpan(k10MinutesLabel, strlen(k10MinutesLabel))) },
    { .step = 36, .label = Optional<chip::CharSpan>(chip::CharSpan(k15MinutesLabel, strlen(k15MinutesLabel))) },
    { .step = 48, .label = Optional<chip::CharSpan>(chip::CharSpan(k20MinutesLabel, strlen(k20MinutesLabel))) },
    { .step = 60, .label = Optional<chip::CharSpan>(chip::CharSpan(k25MinutesLabel, strlen(k25MinutesLabel))) },
    { .step = 70, .label = Optional<chip::CharSpan>(chip::CharSpan(k30MinutesLabel, strlen(k30MinutesLabel))) },
    { .step = 80, .label = Optional<chip::CharSpan>(chip::CharSpan(k60MinutesLabel, strlen(k60MinutesLabel))) },
    { .step = 90, .label = Optional<chip::CharSpan>(chip::CharSpan(k120MinutesLabel, strlen(k120MinutesLabel))) },
    { .step = 100, .label = Optional<chip::CharSpan>(chip::CharSpan(kNeverLabel, strlen(kNeverLabel))) },
};

// assumes it'll be the only delegate for it's lifetime.
struct EPrefDelegate : public EnergyPreferenceDelegate
{
    EPrefDelegate();
    virtual ~EPrefDelegate();

    CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, BalanceStruct::Type & balance) override;
    CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, EnergyPriorityEnum & priority) override;
    CHIP_ERROR GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, BalanceStruct::Type & balance) override;

    size_t GetNumEnergyBalances(chip::EndpointId aEndpoint) override;
    size_t GetNumLowPowerModes(chip::EndpointId aEndpoint) override;
};

EPrefDelegate::EPrefDelegate() : EnergyPreferenceDelegate()
{
    SetMatterEnergyPreferencesDelegate(this);
}

EPrefDelegate::~EPrefDelegate()
{
    SetMatterEnergyPreferencesDelegate(nullptr);
}

size_t EPrefDelegate::GetNumEnergyBalances(chip::EndpointId aEndpoint)
{
    return (sizeof(gsEnergyBalances) / sizeof(gsEnergyBalances[0]));
}

size_t EPrefDelegate::GetNumLowPowerModes(chip::EndpointId aEndpoint)
{
    return (sizeof(gsEnergyBalances) / sizeof(gsEnergyBalances[0]));
}

CHIP_ERROR
EPrefDelegate::GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, BalanceStruct::Type & balance)
{
    if (aIndex < GetNumEnergyBalances(aEndpoint))
    {
        balance = gsEnergyBalances[aIndex];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
EPrefDelegate::GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, EnergyPriorityEnum & priority)
{
    static EnergyPriorityEnum priorities[] = { EnergyPriorityEnum::kEfficiency, EnergyPriorityEnum::kComfort };

    if (aIndex < (sizeof(priorities) / sizeof(priorities[0])))
    {
        priority = priorities[aIndex];
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
EPrefDelegate::GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, BalanceStruct::Type & balance)
{
    if (aIndex < GetNumLowPowerModes(aEndpoint))
    {
        balance = gsPowerBalances[aIndex];
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
}

static EPrefDelegate gsDelegate;
