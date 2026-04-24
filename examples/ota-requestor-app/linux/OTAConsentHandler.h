#ifndef OTA_CONSENT_HANDLER_H
#define OTA_CONSENT_HANDLER_H

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h> // Ensure this is the correct include path

namespace chip {

class OTAConsentHandler {
public:
    OTAConsentHandler() = default;

    /**
     * FIX: Remove the 'chip::' prefix because we are already 
     * inside the chip namespace.
     */
    void SetRequestorCanConsent(Optional<bool> consent);

    void SetRequestorCanConsent(bool consent);

    /**
     * FIX: Just use Optional<bool> here as well.
     */
    Optional<bool> GetRequestorConsent() const;

    bool HasValue() const;

private:
    Optional<bool> mRequestorCanConsent;
};

extern OTAConsentHandler gConsentHandler;

} // namespace chip

#endif // OTA_CONSENT_HANDLER_H