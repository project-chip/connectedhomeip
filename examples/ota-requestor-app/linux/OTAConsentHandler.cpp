#include "OTAConsentHandler.h"


namespace chip {

// Define the global instance here
OTAConsentHandler gConsentHandler;

void OTAConsentHandler::SetRequestorCanConsent(chip::Optional<bool> consent) {
    mRequestorCanConsent = consent;
}

void OTAConsentHandler::SetRequestorCanConsent(bool consent) {
    mRequestorCanConsent.SetValue(consent);
}

chip::Optional<bool> OTAConsentHandler::GetRequestorConsent() const {
    return mRequestorCanConsent;
}

bool OTAConsentHandler::HasValue() const {
    return mRequestorCanConsent.HasValue();
}

} // namespace chip
