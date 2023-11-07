#include <../Zephyr/ZephyrConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

    class LockSettingsStorage: ZephyrConfig
    {
        public:
        static const ZephyrConfig::Key kConfigKey_LockUser;
        static const ZephyrConfig::Key kConfigKey_Credential;
        static const ZephyrConfig::Key kConfigKey_LockUserName;
        static const ZephyrConfig::Key kConfigKey_CredentialData;
        static const ZephyrConfig::Key kConfigKey_UserCredentials;
        static const ZephyrConfig::Key kConfigKey_WeekDaySchedules;
        static const ZephyrConfig::Key kConfigKey_YearDaySchedules;
        static const ZephyrConfig::Key kConfigKey_HolidaySchedules;
    };
}
}
}
