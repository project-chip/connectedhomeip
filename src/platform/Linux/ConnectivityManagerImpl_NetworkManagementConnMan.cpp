/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *  @file
 *    This implements a C++ binding for a Linux platform
 *    implementation of the Matter Connectivity Manager against the
 *    third-party, open source D-Bus-based Connection Manager (also
 *    known as, connman) network manager.
 *
 *  As with other D-Bus based Matter Linux platform infrastructure,
 *  this implementation relies upon the dedicated GLib thread context
 *  for animating and dispatching connman D-Bus connection activity.
 *
 *  Because there is coordination between the Matter thread context
 *  and the GLib thread context, cross-thread mutual exclusion is
 *  absolutely required and a few, short, mechanical rules must be
 *  observed:
 *
 *    1. There shall be no invocation of the synchronous (that is,
 *       blocking) `InvokeOnGLibMainLoopSync` while holding the class
 *       lock.
 *    2. `*Locked` methods may temporarily unlock via
 *       `UnlockAndInvoke*` to perform a cross-thread synchronous
 *       scheduling/D-Bus call, but shall not wait for an asynchronous
 *       D-Bus completion under lock.
 *    3. All cross-thread  synchronous  calls  shall  be  wrapped by
 *      `UnlockAndInvoke*`.
 *    4. All `*OnGLib` methods shall not hold the class lock and shall
 *       not call `*Locked` methods or functions directly.
 *
 *  As for the class lock itself, there are three further rules:
 *
 *    1. Any method named `*Locked` shall not take a lock parameter and
 *       shall not manipulate the class lock. It simply requires that
 *       the class lock is held; this is verified and enforced by
 *       assertion.
 *    2. Any method that might need to temporarily unlock shall use a
 *       `std::unique_lock` in the parent "orchestrator" method.
 *    3. In any scope where something is passed to `UnlockAndInvoke`,
 *       a `std::unique_lock` instance of the class lock shall be held.
 *
 *  Where possible, all meaningful work is transitioned from the GLib
 *  thread context to the Matter context throught the use of
 *  `ScheduleLambda`.
 *
 */

#include "ConnectivityManagerImpl_NetworkManagementConnMan.h"

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>

#include <errno.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceControlServer.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>

#include "ConnectivityManagerImpl.h"
#include "WirelessDefs.h"

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer::NetworkCommissioning;
using namespace ::chip::app::Clusters::GeneralDiagnostics;
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;

// Preprocessor Definitions

#define CONNMAN_SERVICE "net.connman"

#define CONNMAN_AGENT_INTERFACE CONNMAN_SERVICE ".Agent"

#define CONNMAN_MANAGER_INTERFACE CONNMAN_SERVICE ".Manager"
#define CONNMAN_MANAGER_PATH "/"
#define CONNMAN_MANAGER_OBJECT_PATH_STEM CONNMAN_MANAGER_PATH "net/connman"
#define CONNMAN_MANAGER_SERVICE_PATH_STEM CONNMAN_MANAGER_OBJECT_PATH_STEM "/service"
#define CONNMAN_MANAGER_TECHNOLOGY_PATH_STEM CONNMAN_MANAGER_OBJECT_PATH_STEM "/technology"

#define CONNMAN_SERVICE_INTERFACE CONNMAN_SERVICE ".Service"
#define CONNMAN_TECHNOLOGY_INTERFACE CONNMAN_SERVICE ".Technology"

// Note that dashes ("-") are not allowed in D-Bus
// paths.

#define MATTER_PATH "/org/csa/matter"
#define MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH MATTER_PATH "/ConnectivityManager"

#define kConnManObjectPropertyNameKey "Name"

#define kConnManObjectPropertyStateKey "State"

#define kConnManObjectPropertyStateIdleValue "idle"
#define kConnManObjectPropertyStateOfflineValue "offline"
#define kConnManObjectPropertyStateOnlineValue "online"
#define kConnManObjectPropertyStateReadyValue "ready"

#define kConnManObjectPropertyTypeKey "Type"

#define kConnManObjectPropertyTypeEthernetValue "ethernet"
#define kConnManObjectPropertyTypeWiFiValue "wifi"

#define CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connman: "

namespace chip {
namespace DeviceLayer {

#if CHIP_LINUX_NETWORK_MANAGER_CONNMAN

namespace {

// Type Declarations

template <typename Callable>
struct GLibInvokeContext
{
    Callable mCallable;
    CHIP_ERROR mStatus = CHIP_NO_ERROR;
};

class UnboxedVariant
{
public:
    explicit UnboxedVariant(GVariant * inMaybeVariant) noexcept;

    GVariant * get(void) const noexcept;
    explicit operator bool(void) const noexcept;

private:
    GAutoPtr<GVariant> mOwned;   // owns unboxed value if we had to unbox
    GVariant * mValue = nullptr; // points to either inMaybeVariant or mOwned.get()
};

// Global Variables

static constexpr char kConnManServiceName[]      = CONNMAN_SERVICE;
static constexpr char kConnManManagerInterface[] = CONNMAN_MANAGER_INTERFACE;
static constexpr char kConnManManagerPath[]      = CONNMAN_MANAGER_PATH;

// Service Agent Object Property Keys and Values

static constexpr char kConnManServiceAgentPropertyAlternatesKey[]         = "Alternates";
static constexpr char kConnManServiceAgentPropertyIdentityKey[]           = "Identity";
static constexpr char kConnManServiceAgentPropertyNameKey[]               = "Name";
static constexpr char kConnManServiceAgentPropertyPassphraseKey[]         = "Passphrase";
static constexpr char kConnManServiceAgentPropertyPasswordKey[]           = "Password";
static constexpr char kConnManServiceAgentPropertyPreviousPassphraseKey[] = "PreviousPassphrase";

static constexpr char kConnManServiceAgentPropertyRequirementKey[] = "Requirement";

static constexpr char kConnManServiceAgentPropertyRequirementAlternateValue[]     = "alternate";
static constexpr char kConnManServiceAgentPropertyRequirementInformationalValue[] = "informational";
static constexpr char kConnManServiceAgentPropertyRequirementMandatoryValue[]     = "mandatory";
static constexpr char kConnManServiceAgentPropertyRequirementOptionalValue[]      = "optional";

static constexpr char kConnManServiceAgentPropertySSIDKey[] = "SSID";

static constexpr char kConnManServiceAgentPropertyTypeKey[] = "Type";

static constexpr char kConnManServiceAgentPropertyTypePassphraseValue[] = "passphrase";
static constexpr char kConnManServiceAgentPropertyTypePSKValue[]        = "psk";
static constexpr char kConnManServiceAgentPropertyTypeResponseValue[]   = "response";
static constexpr char kConnManServiceAgentPropertyTypeSSIDValue[]       = "ssid";
static constexpr char kConnManServiceAgentPropertyTypeStringValue[]     = "string";
static constexpr char kConnManServiceAgentPropertyTypeWEPValue[]        = "wep";
static constexpr char kConnManServiceAgentPropertyTypeWPSPINValue[]     = "wpspin";

static constexpr char kConnManServiceAgentPropertyUsernameKey[] = "Username";
static constexpr char kConnManServiceAgentPropertyValueKey[]    = "Value";
static constexpr char kConnManServiceAgentPropertyWPSKey[]      = "WPS";

// Manager Object Property Keys and Values

static constexpr char kConnManManagerPropertyStateKey[] = kConnManObjectPropertyStateKey;

static constexpr char kConnManManagerPropertyStateIdleValue[]        = kConnManObjectPropertyStateIdleValue;
static constexpr char kConnManManagerPropertyStateOfflineValue[]     = kConnManObjectPropertyStateOfflineValue;
static constexpr char kConnManManagerPropertyStateOnlineValue[]      = kConnManObjectPropertyStateOnlineValue;
static constexpr char kConnManManagerPropertyStateReadyValue[]       = kConnManObjectPropertyStateReadyValue;
static constexpr char kConnManManagerPropertyStateUnavailableValue[] = "unavailable";

// Service Object Property Keys and Values

static constexpr char kConnManServicePropertyAutoConnectKey[]          = "AutoConnect";
static constexpr char kConnManServicePropertyDomainsKey[]              = "Domains";
static constexpr char kConnManServicePropertyDomainsConfigurationKey[] = "Domains.Configuration";

static constexpr char kConnManServicePropertyErrorKey[] = "Error";

static constexpr char kConnManServicePropertyErrorAuthFailedValue[]        = "auth-failed";
static constexpr char kConnManServicePropertyErrorBlockedValue[]           = "blocked";
static constexpr char kConnManServicePropertyErrorConnectFailedValue[]     = "connect-failed";
static constexpr char kConnManServicePropertyErrorDHCPFailedValue[]        = "dhcp-failed";
static constexpr char kConnManServicePropertyErrorInvalidKeyValue[]        = "invalid-key";
static constexpr char kConnManServicePropertyErrorLoginFailedValue[]       = "login-failed";
static constexpr char kConnManServicePropertyErrorOnlineCheckFailedValue[] = "online-check-failed";
static constexpr char kConnManServicePropertyErrorOutOfRangeValue[]        = "out-of-range";
static constexpr char kConnManServicePropertyErrorPINMissingValue[]        = "pin-missing";

static constexpr char kConnManServicePropertyEthernetKey[] = "Ethernet";

static constexpr char kConnManServicePropertyEthernetAddressKey[]   = "Address";
static constexpr char kConnManServicePropertyEthernetInterfaceKey[] = "Interface";
static constexpr char kConnManServicePropertyEthernetMTUKey[]       = "MTU";
static constexpr char kConnManServicePropertyEthernetMethodKey[]    = "Method";

static constexpr char kConnManServicePropertyFavoriteKey[]                 = "Favorite";
static constexpr char kConnManServicePropertyIPv4Key[]                     = "IPv4";
static constexpr char kConnManServicePropertyIPv4ConfigurationKey[]        = "IPv4.Configuration";
static constexpr char kConnManServicePropertyIPv6Key[]                     = "IPv6";
static constexpr char kConnManServicePropertyIPv6ConfigurationKey[]        = "IPv6.Configuration";
static constexpr char kConnManServicePropertymDNSKey[]                     = "mDNS";
static constexpr char kConnManServicePropertymDNSConfigurationKey[]        = "mDNS.Configuration";
static constexpr char kConnManServicePropertyNameKey[]                     = kConnManObjectPropertyNameKey;
static constexpr char kConnManServicePropertyNameserversKey[]              = "Nameservers";
static constexpr char kConnManServicePropertyNameserversConfigurationKey[] = "Nameservers.Configuration";
static constexpr char kConnManServicePropertyProviderKey[]                 = "Provider";
static constexpr char kConnManServicePropertyProxyConfigurationKey[]       = "Proxy.Configuration";
static constexpr char kConnManServicePropertyProxyKey[]                    = "Proxy";

static constexpr char kConnManServicePropertyRoamingKey[] = "Roaming";

static constexpr char kConnManServicePropertySecurityKey[] = "Security";

static constexpr char kConnManServicePropertySecurity8021XValue[]          = "ieee8021x";
static constexpr char kConnManServicePropertySecurityNoneValue[]           = "none";
static constexpr char kConnManServicePropertySecurityPSKValue[]            = "psk";
static constexpr char kConnManServicePropertySecurityRSNValue[]            = "rsn";
static constexpr char kConnManServicePropertySecurityWEPValue[]            = "wep";
static constexpr char kConnManServicePropertySecurityWPAValue[]            = "wpa";
static constexpr char kConnManServicePropertySecurityWPSAdvertisingValue[] = "wps_advertising";
static constexpr char kConnManServicePropertySecurityWPSValue[]            = "wps";

static constexpr char kConnManServicePropertyStateKey[] = "State";

static constexpr char kConnManServicePropertyStateAssociationValue[]   = "association";
static constexpr char kConnManServicePropertyStateConfigurationValue[] = "configuration";
static constexpr char kConnManServicePropertyStateDisconnectValue[]    = "disconnect";
static constexpr char kConnManServicePropertyStateFailureValue[]       = "failure";
static constexpr char kConnManServicePropertyStateIdleValue[]          = kConnManObjectPropertyStateIdleValue;
static constexpr char kConnManServicePropertyStateOfflineValue[]       = kConnManObjectPropertyStateOfflineValue;
static constexpr char kConnManServicePropertyStateOnlineValue[]        = kConnManObjectPropertyStateOnlineValue;
static constexpr char kConnManServicePropertyStateReadyValue[]         = kConnManObjectPropertyStateReadyValue;

static constexpr char kConnManServicePropertyStrengthKey[]                 = "Strength";
static constexpr char kConnManServicePropertyTimeserversKey[]              = "Timeservers";
static constexpr char kConnManServicePropertyTimeserversConfigurationKey[] = "Timeservers.Configuration";

static constexpr char kConnManServicePropertyTypeKey[] = kConnManObjectPropertyTypeKey;

static constexpr char kConnManServicePropertyTypeEthernetValue[] = kConnManObjectPropertyTypeEthernetValue;
static constexpr char kConnManServicePropertyTypeWiFiValue[]     = kConnManObjectPropertyTypeWiFiValue;

// Technology Object Property Keys and Values

static constexpr char kConnManTechnologyPropertyConnectedKey[] = "Connected";
static constexpr char kConnManTechnologyPropertyNameKey[]      = kConnManObjectPropertyNameKey;
static constexpr char kConnManTechnologyPropertyPoweredKey[]   = "Powered";
static constexpr char kConnManTechnologyPropertyTypeKey[]      = kConnManObjectPropertyTypeKey;

static constexpr char kConnManTechnologyPropertyTypeEthernetValue[] = kConnManObjectPropertyTypeEthernetValue;
static constexpr char kConnManTechnologyPropertyTypeWiFiValue[]     = kConnManObjectPropertyTypeWiFiValue;

// Network Name and Passphrase Validate Check Failure Reason Strings

static constexpr char kNetworkNameIsTooShort[]                 = "name is too short";
static constexpr char kNetworkNameIsTooLong[]                  = "name is too long";
static constexpr char kNetworkPassphraseIsTooShort[]           = "passphrase is too short";
static constexpr char kNetworkPassphraseIsTooLong[]            = "passphrase is too long";
static constexpr char kNetworkPassphraseHasInvalidCharacters[] = "passphrase contains an invalid character";

/**
 *  The maximum number of "active" (connman does not actually do
 *  active, directed scans) scans for a particular SSID before
 *  failing.
 */
static constexpr size_t kWiFiActiveScanLimit = 4;

/**
 *  The maximum number of scans for connecting to a particular SSID
 *  before failing.
 */
static constexpr size_t kWiFiConnectScanLimit = 4;

// Function and Method Implementation

// Unboxed Variant

// Construction

UnboxedVariant::UnboxedVariant(GVariant * inMaybeVariant) noexcept : mValue(inMaybeVariant)
{
    if (mValue != nullptr && g_variant_is_of_type(mValue, G_VARIANT_TYPE_VARIANT))
    {
        mOwned.reset(g_variant_get_variant(mValue));

        mValue = mOwned.get();
    }
}

GVariant * UnboxedVariant::get(void) const noexcept
{
    return mValue;
}

UnboxedVariant::operator bool(void) const noexcept
{
    return mValue != nullptr;
}

static CHIP_ERROR RemoveObjectLocked(GHashTable * inObjectTable, const char * inObjectPath, GHashTable * inObjectProxies) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectProxies != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_remove(inObjectTable, inObjectPath);
    g_hash_table_remove(inObjectProxies, inObjectPath);

exit:
    return retval;
}

static CHIP_ERROR GetObjectPropertiesFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath,
                                                    GVariant *& outProperties) noexcept
{
    GVariant * props  = nullptr;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    props = static_cast<GVariant *>(g_hash_table_lookup(inObjectTable, inObjectPath));
    VerifyOrExit(props != nullptr, retval = ChipError(ChipError::Range::kPOSIX, ENOENT));

    outProperties = props;

exit:
    return retval;
}

static CHIP_ERROR GetObjectByteValueFromPropertiesLocked(GVariant * inProperties, const char * inKey, uint8_t & outValue) noexcept
{
    GAutoPtr<GVariant> boxed;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_WRONG_KEY_TYPE);

    boxed.reset(g_variant_lookup_value(inProperties, inKey, nullptr));
    VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    UnboxedVariant value(boxed.get());
    VerifyOrExit(value, retval = CHIP_ERROR_INTERNAL);
    VerifyOrExit(g_variant_is_of_type(value.get(), G_VARIANT_TYPE_BYTE), retval = CHIP_ERROR_WRONG_KEY_TYPE);

    outValue = g_variant_get_byte(value.get());

exit:
    return retval;
}

static CHIP_ERROR GetObjectStringValueFromPropertiesLocked(GVariant * inProperties, const char * inKey,
                                                           const char *& outValue) noexcept
{
    GAutoPtr<GVariant> boxed;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_WRONG_KEY_TYPE);

    boxed.reset(g_variant_lookup_value(inProperties, inKey, nullptr));
    VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    UnboxedVariant value(boxed.get());
    VerifyOrExit(value, retval = CHIP_ERROR_INTERNAL);
    VerifyOrExit(g_variant_is_of_type(value.get(), G_VARIANT_TYPE_STRING), retval = CHIP_ERROR_WRONG_KEY_TYPE);

    outValue = g_variant_get_string(value.get(), nullptr);

exit:
    return retval;
}

static CHIP_ERROR GetObjectNameFromPropertiesLocked(GVariant * inProperties, const char *& outName) noexcept
{
    return GetObjectStringValueFromPropertiesLocked(inProperties, kConnManObjectPropertyNameKey, outName);
}

static CHIP_ERROR GetObjectStateFromPropertiesLocked(GVariant * inProperties, const char *& outState) noexcept
{
    return GetObjectStringValueFromPropertiesLocked(inProperties, kConnManObjectPropertyStateKey, outState);
}

static CHIP_ERROR GetObjectTypeFromPropertiesLocked(GVariant * inProperties, const char *& outType) noexcept
{
    return GetObjectStringValueFromPropertiesLocked(inProperties, kConnManObjectPropertyTypeKey, outType);
}

static CHIP_ERROR GetObjectStringValueFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath, const char * inKey,
                                                     const char *& outValue) noexcept
{
    GVariant * props;
    GAutoPtr<GVariant> variant;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inKey != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    props = static_cast<GVariant *>(g_hash_table_lookup(inObjectTable, inObjectPath));
    VerifyOrExit(props != nullptr, retval = CHIP_ERROR_KEY_NOT_FOUND);

    retval = GetObjectStringValueFromPropertiesLocked(props, inKey, outValue);
    SuccessOrExit(retval);

exit:
    return retval;
}

static CHIP_ERROR GetObjectTypeFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath, const char *& outType) noexcept
{
    return GetObjectStringValueFromPathLocked(inObjectTable, inObjectPath, kConnManObjectPropertyTypeKey, outType);
}

static CHIP_ERROR GetObjectPathFromProxyLocked(GDBusProxy * inProxy, const char *& outPath) noexcept
{
    const char * path;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inProxy != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    path = g_dbus_proxy_get_object_path(inProxy);
    VerifyOrExit(path != nullptr, retval = CHIP_ERROR_INTERNAL);

    outPath = path;

exit:
    return retval;
}

static CHIP_ERROR GetObjectTypeFromProxyLocked(GDBusProxy * inProxy, GHashTable * inObjectTable, const char *& outType) noexcept
{
    const char * path;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inProxy != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    retval = GetObjectPathFromProxyLocked(inProxy, path);
    SuccessOrExit(retval);

    retval = GetObjectTypeFromPathLocked(inObjectTable, path, outType);
    SuccessOrExit(retval);

exit:
    return retval;
}

static bool HasTechnologyLocked(GHashTable * inTechnologies, const char * inType) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    GAutoPtr<GVariant> variant;
    CHIP_ERROR status;
    bool retval = false;

    VerifyOrExit(inTechnologies != nullptr, retval = false);
    VerifyOrExit(inType != nullptr, retval = false);

    g_hash_table_iter_init(&iter, inTechnologies);

    while (!retval && g_hash_table_iter_next(&iter, &key, &value))
    {
        GVariant * const props = static_cast<GVariant *>(value);
        if (props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
            continue;

        const char * type = nullptr;
        status            = GetObjectTypeFromPropertiesLocked(props, type);
        if (ChipError::IsSuccess(status) && type && strcmp(type, inType) == 0)
        {
            retval = true;
        }
    }

exit:
    return retval;
}

static void LogProperty(const char * inDescription, const char * inPath, const char * inKey, GVariant * inValue) noexcept
{
    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: %s: key '%s' value type '%s'", inDescription, inPath,
                  inKey, g_variant_get_type_string(inValue));
}

static void LogProperties(const char * inDescription, const char * inPath, GVariant * inDictionary) noexcept
{
    GVariantIter it;
    const char * key = nullptr;
    GVariant * value = nullptr;

    if (inDictionary == nullptr || !g_variant_is_of_type(inDictionary, G_VARIANT_TYPE_VARDICT))
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: %s: not a{sv} (type=%s)", inDescription, inPath,
                     inDictionary ? g_variant_get_type_string(inDictionary) : "(null)");
    }
    else
    {
        g_variant_iter_init(&it, inDictionary);
        while (g_variant_iter_next(&it, "{&sv}", &key, &value))
        {
            LogProperty(inDescription, inPath, key, value);

            g_variant_unref(value);
        }
    }
}

static CHIP_ERROR Merge(GVariant * inBase, GVariant * inDelta, GAutoPtr<GVariant> & outResult) noexcept
{
    GAutoPtr<GHashTable> delta_keys;
    GVariantIter delta_iter;
    const char * delta_key = nullptr;
    GVariant * delta_value = nullptr;
    GVariantIter base_iter;
    const char * base_key = nullptr;
    GVariant * base_value = nullptr;
    GVariantBuilder builder;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inDelta != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(g_variant_is_of_type(inDelta, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_INVALID_ARGUMENT);

    // If there is no base variant, then just take a reference to the
    // delta variant. Otherwise, proceed merging base and delta.

    if (inBase == nullptr)
    {
        outResult.reset(g_variant_ref(inDelta));
    }
    else
    {
        VerifyOrExit(g_variant_is_of_type(inBase, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_INVALID_ARGUMENT);

        // GLib does not have a container with set semantics, so we use a
        // hash table to uniquely contain all of the keys in the delta
        // variant.

        delta_keys.reset(g_hash_table_new_full(g_str_hash, g_str_equal, g_free, nullptr));
        VerifyOrExit(delta_keys.get() != nullptr, retval = CHIP_ERROR_INTERNAL);

        // Get all of the delta keys into the newly-created hash table.

        g_variant_iter_init(&delta_iter, inDelta);

        while (g_variant_iter_next(&delta_iter, "{&sv}", &delta_key, &delta_value))
        {
            g_hash_table_replace(delta_keys.get(), g_strdup(delta_key), GINT_TO_POINTER(1));

            g_variant_unref(delta_value);
        }

        // Initialize the result variant builder and add all base entries
        // that are not otherwise overwritten by the delta.

        g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

        g_variant_iter_init(&base_iter, inBase);

        while (g_variant_iter_next(&base_iter, "{&sv}", &base_key, &base_value))
        {
            if (!g_hash_table_contains(delta_keys.get(), base_key))
            {
                g_variant_builder_add(&builder, "{sv}", base_key, base_value);
            }

            g_variant_unref(base_value);
        }

        // Finally, add with delta entries.

        g_variant_iter_init(&delta_iter, inDelta);

        while (g_variant_iter_next(&delta_iter, "{&sv}", &delta_key, &delta_value))
        {
            g_variant_builder_add(&builder, "{sv}", delta_key, delta_value);

            g_variant_unref(delta_value);
        }

        outResult.reset(g_variant_builder_end(&builder));
    }

exit:
    return retval;
}

static CHIP_ERROR MergeObjectPropertiesLocked(const char * inDescription, GHashTable * inObjectTable, const char * inObjectPath,
                                              GVariant * inObjectProperties) noexcept
{
    GVariant * current = nullptr;
    GAutoPtr<GVariant> merged;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectProperties != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(g_variant_is_of_type(inObjectProperties, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_INVALID_ARGUMENT);

    // If the specified object properties are empty, then there is
    // nothing to merge; otherwise, merge the properties key/value
    // pairs.

    LogProperties(inDescription, inObjectPath, inObjectProperties);

    if (g_variant_n_children(inObjectProperties) > 0)
    {
        retval = GetObjectPropertiesFromPathLocked(inObjectTable, inObjectPath, current);
        nlEXPECT(ChipError::IsSuccess(retval) || retval == ChipError(ChipError::Range::kPOSIX, ENOENT), exit);

        retval = Merge(current, inObjectProperties, merged);
        SuccessOrExit(retval);

        g_hash_table_replace(inObjectTable, g_strdup(inObjectPath), merged.release());
    }

exit:
    return retval;
}

static CHIP_ERROR MergeObjectPropertiesLocked(const char * inDescription, GHashTable * inObjectTable, const char * inObjectPath,
                                              const char * inKey, GVariant * inValue) noexcept
{
    GVariantBuilder builder;
    GAutoPtr<GVariant> property;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inKey != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inValue != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    g_variant_builder_add(&builder, "{sv}", inKey, inValue);

    property.reset(g_variant_builder_end(&builder));

    retval = MergeObjectPropertiesLocked(inDescription, inObjectTable, inObjectPath, property.get());
    SuccessOrExit(retval);

exit:
    return retval;
}

static CHIP_ERROR MergeObjectPropertiesLocked(const char * inDescription, GHashTable * inObjectTable, GVariant * inObjects) noexcept
{
    GVariantIter iter;
    const char * path = nullptr;
    GVariant * props  = nullptr;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjects != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(g_variant_is_of_type(inObjects, G_VARIANT_TYPE("a(oa{sv})")), retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_variant_iter_init(&iter, inObjects);

    while (g_variant_iter_next(&iter, "(&o@a{sv})", &path, &props))
    {
        retval = MergeObjectPropertiesLocked(inDescription, inObjectTable, path, props);
        SuccessOrExit(retval);

        props = nullptr;
    }

exit:
    return retval;
}

static bool ByteSpanEqualsString(const ByteSpan & inSpan, const char * inString) noexcept
{
    size_t length;
    bool retval;

    VerifyOrExit(inString != nullptr, retval = false);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: comparing '%.*s' v. '%s'...", __func__, inSpan.size(),
                  inSpan.data(), inString);

    length = strlen(inString);

    retval = ((inSpan.size() == length) && (memcmp(inSpan.data(), inString, length) == 0));

exit:
    return retval;
}

static ConnManService * GetWiFiServiceProxyFromSsidLocked(const ByteSpan & inSsid, GHashTable * inProxies,
                                                          GHashTable * inServices) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    CHIP_ERROR status;
    ConnManService * retval = nullptr;

    VerifyOrExit(inSsid.size() > 0, retval = nullptr);
    VerifyOrExit(inProxies != nullptr, retval = nullptr);
    VerifyOrExit(inServices != nullptr, retval = nullptr);

    g_hash_table_iter_init(&iter, inServices);

    while (retval == nullptr && g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const path = static_cast<const char *>(key);
        GVariant * const props  = static_cast<GVariant *>(value);
        if (path == nullptr || props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
            continue;

        const char * type = nullptr;
        status            = GetObjectTypeFromPropertiesLocked(props, type);
        if (!ChipError::IsSuccess(status) || type == nullptr || strcmp(type, kConnManServicePropertyTypeWiFiValue) != 0)
            continue;

        const char * name = nullptr;
        status            = GetObjectNameFromPropertiesLocked(props, name);
        if (ChipError::IsSuccess(status) && name != nullptr && ByteSpanEqualsString(inSsid, name))
            retval = static_cast<ConnManService *>(g_hash_table_lookup(inProxies, path));
    }

exit:
    return retval;
}

static ConnManTechnology * GetTechnologyProxyFromTypeLocked(const char * inType, GHashTable * inProxies,
                                                            GHashTable * inTechnologies) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    CHIP_ERROR status;
    ConnManTechnology * retval = nullptr;

    VerifyOrExit(inType != nullptr, retval = nullptr);
    VerifyOrExit(inProxies != nullptr, retval = nullptr);
    VerifyOrExit(inTechnologies != nullptr, retval = nullptr);

    g_hash_table_iter_init(&iter, inTechnologies);

    while (retval == nullptr && g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const path = static_cast<const char *>(key);
        GVariant * const props  = static_cast<GVariant *>(value);
        if (path == nullptr || props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
            continue;

        const char * type = nullptr;
        status            = GetObjectTypeFromPropertiesLocked(props, type);
        if (ChipError::IsSuccess(status) && type && strcmp(type, inType) == 0)
        {
            retval = static_cast<ConnManTechnology *>(g_hash_table_lookup(inProxies, path));
        }
    }

exit:
    return retval;
}

static CHIP_ERROR GetServiceInterfaceFromEthernetLocked(GVariant * inEthernet, GAutoPtr<char> & outInterface) noexcept
{
    const char * interface;
    gboolean status;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inEthernet != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(g_variant_is_of_type(inEthernet, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_WRONG_KEY_TYPE);

    status = g_variant_lookup(inEthernet, kConnManServicePropertyEthernetInterfaceKey, "&s", &interface);
    VerifyOrExit(status && interface, retval = CHIP_ERROR_KEY_NOT_FOUND);

    outInterface.reset(g_strdup(interface));
    VerifyOrExit(outInterface.get() != nullptr, retval = CHIP_ERROR_NO_MEMORY);

exit:
    return retval;
}

static CHIP_ERROR GetServiceInterfaceFromPropertiesLocked(GVariant * inProperties, GAutoPtr<char> & outInterface) noexcept
{
    GAutoPtr<GVariant> variant;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inProperties != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_WRONG_KEY_TYPE);

    variant.reset(g_variant_lookup_value(inProperties, kConnManServicePropertyEthernetKey, nullptr));
    VerifyOrExit(variant.get() != nullptr, retval = CHIP_ERROR_KEY_NOT_FOUND);

    retval = GetServiceInterfaceFromEthernetLocked(variant.get(), outInterface);
    SuccessOrExit(retval);

exit:
    return retval;
}

static CHIP_ERROR GetServiceInterfaceFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath,
                                                    GAutoPtr<char> & outInterface) noexcept
{
    GVariant * props;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    props = static_cast<GVariant *>(g_hash_table_lookup(inObjectTable, inObjectPath));
    VerifyOrExit(props != nullptr, retval = CHIP_ERROR_KEY_NOT_FOUND);

    retval = GetServiceInterfaceFromPropertiesLocked(props, outInterface);
    SuccessOrExit(retval);

exit:
    return retval;
}

static int GetServiceStateScore(const char * inState) noexcept
{
    int retval = 0;

    nlEXPECT(inState != nullptr, exit);

    if (strcmp(inState, kConnManServicePropertyStateOnlineValue) == 0)
        retval = 7;
    else if (strcmp(inState, kConnManServicePropertyStateReadyValue) == 0)
        retval = 6;
    else if (strcmp(inState, kConnManServicePropertyStateConfigurationValue) == 0)
        retval = 5;
    else if (strcmp(inState, kConnManServicePropertyStateAssociationValue) == 0)
        retval = 4;
    else if (strcmp(inState, kConnManServicePropertyStateIdleValue) == 0)
        retval = 3;
    else if (strcmp(inState, kConnManServicePropertyStateDisconnectValue) == 0)
        retval = 2;
    else if (strcmp(inState, kConnManServicePropertyStateOfflineValue) == 0)
        retval = 1;

exit:
    return retval;
}

static CHIP_ERROR GetBestServicePathForTypeLocked(const char * inType, GHashTable * inServices, GAutoPtr<char> & outPath) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    int best_score = -1;
    int current_score;
    GAutoPtr<char> best_path;
    CHIP_ERROR status;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inType != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inServices != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_iter_init(&iter, inServices);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const current_path = static_cast<const char *>(key);
        GVariant * const props          = static_cast<GVariant *>(value);
        const char * type               = nullptr;
        const char * state              = nullptr;

        if (props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
            continue;

        status = GetObjectTypeFromPropertiesLocked(props, type);
        if (!ChipError::IsSuccess(status) || type == nullptr)
            continue;

        if (strcmp(type, inType) != 0)
            continue;

        retval = GetObjectStateFromPropertiesLocked(props, state);
        if (!ChipError::IsSuccess(retval))
            continue;

        current_score = GetServiceStateScore(state);

        if (current_score > best_score)
        {
            best_score = current_score;
            best_path.reset(g_strdup(current_path));
        }
    }

    VerifyOrReturnError(best_path.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    outPath.reset(best_path.release());

exit:
    return retval;
}

static CHIP_ERROR GetBestServiceInterfaceForTypeLocked(const char * inType, GHashTable * inServices,
                                                       GAutoPtr<char> & outInterface) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    int best_score = -1;
    int current_score;
    GAutoPtr<char> best_interface;
    GAutoPtr<char> current_interface;
    CHIP_ERROR status;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inType != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inServices != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_iter_init(&iter, inServices);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        GVariant * const props = static_cast<GVariant *>(value);
        const char * type      = nullptr;
        const char * state     = nullptr;

        if (props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
            continue;

        status = GetObjectTypeFromPropertiesLocked(props, type);
        if (!ChipError::IsSuccess(status) || type == nullptr)
            continue;

        if (strcmp(type, inType) != 0)
            continue;

        TEMPORARY_RETURN_IGNORED GetObjectStringValueFromPropertiesLocked(props, kConnManServicePropertyStateKey, state);

        current_score = GetServiceStateScore(state);

        retval = GetServiceInterfaceFromPropertiesLocked(props, current_interface);
        if (!ChipError::IsSuccess(retval) || current_interface.get() == nullptr)
            continue;

        if (current_score > best_score)
        {
            best_score = current_score;
            best_interface.reset(current_interface.release());
        }
    }

    VerifyOrReturnError(best_interface.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    outInterface.reset(best_interface.release());

exit:
    return retval;
}

static CHIP_ERROR GetWiFiServicePropertiesFromSsidLocked(const ByteSpan & inSsid, GHashTable * inObjectTable,
                                                         GAutoPtr<GVariant> & outProperties) noexcept
{
    GHashTableIter iter;
    gpointer key      = nullptr;
    gpointer value    = nullptr;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inSsid.size() > 0, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inObjectTable != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_iter_init(&iter, inObjectTable);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const path = static_cast<const char *>(key);
        GVariant * const props  = static_cast<GVariant *>(value);

        if (path == nullptr || props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
            continue;

        const char * type = nullptr;
        retval            = GetObjectTypeFromPropertiesLocked(props, type);
        if (!ChipError::IsSuccess(retval) || type == nullptr || strcmp(type, kConnManServicePropertyTypeWiFiValue) != 0)
        {
            continue;
        }

        // ConnMan's "Name" for Wi-Fi services is the SSID string.

        const char * name = nullptr;
        retval            = GetObjectNameFromPropertiesLocked(props, name);
        if (ChipError::IsSuccess(retval) && name != nullptr && ByteSpanEqualsString(inSsid, name))
        {
            outProperties.reset(g_variant_ref(props));
            VerifyOrExit(outProperties.get() != nullptr, retval = CHIP_ERROR_NO_MEMORY);

            ExitNow(retval = CHIP_NO_ERROR);
        }
    }

    retval = CHIP_ERROR_KEY_NOT_FOUND;

exit:
    if (!ChipError::IsSuccess(retval))
    {
        outProperties.reset();
    }

    return retval;
}

static CHIP_ERROR GetWiFiServiceStrengthLocked(GVariant * inProperties, int8_t & outStrength) noexcept
{
    uint8_t value;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    retval = GetObjectByteValueFromPropertiesLocked(inProperties, kConnManServicePropertyStrengthKey, value);
    SuccessOrExit(retval);

    outStrength = std::clamp(static_cast<int8_t>(value), static_cast<int8_t>(0), static_cast<int8_t>(100));

exit:
    return retval;
}

/**
 *  @brief
 *    Validate the Wi-Fi network name.
 *
 *  This validates that the specified Wi-Fi network name is valid
 *  which means that the specified string is non-null and is between 1
 *  and 32, inclusive ASCII characters or UTF-8 bytes.
 *
 *  @param[in]   inNetworkName  An immutable reference to the network
 *                              name to validate.
 *  @param[out]  outReason
 *
 *  @retval  CHIP_NO_ERROR  If @a inNetworkName is valid.
 *  @retval  -EINVAL              If @a inNetworkName is invalid.
 *
 */
static CHIP_ERROR ValidateWiFiServiceSsid(const ByteSpan & inSsid, const char *& outReason) noexcept
{
    static constexpr size_t kMinLength = 1;
    static constexpr size_t kMaxLength = 32;
    size_t lLength;
    CHIP_ERROR lRetval = CHIP_NO_ERROR;

    lLength = inSsid.size();
    VerifyOrExit(lLength >= kMinLength, outReason = kNetworkNameIsTooShort; lRetval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(lLength <= kMaxLength, outReason = kNetworkNameIsTooLong; lRetval = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    return lRetval;
}

/**
 *  @brief
 *    Validate the Wi-Fi network passphrase.
 *
 *  This validates that the specified Wi-Fi network passphrase is valid,
 *  which means that the specified string is non-null and is either exactly 64
 *  hexadecimal ASCII characters (that is, of the [[:xdigit:]] character class)
 *  or is between 8 and 63, inclusive ASCII characters.
 *
 *  @param[in]   inPassphrase
 *    An immutable reference to the network passphrase to validate.
 *
 *  @param[out]  outReason
 *    A reference to a pointer to an immutable null-terminated C
 *    string containing the human-readable reason @a inPassphrase was
 *    invalid if the returned value is not @c CHIP_NO_ERROR.
 *
 *  @retval  CHIP_NO_ERROR
 *    If @a inPassphrase is valid.
 *
 *  @retval  CHIP_ERROR_INVALID_ARGUMENT
 *    If @a inPassphrase is either too short or too long.
 *
 *  @retval  -EILSEQ
 *    If @a inPassphrase contains invalid characters.
 *
 */
static CHIP_ERROR ValidateWiFiServicePassphrase(const ByteSpan & inPassphrase, const char *& outReason) noexcept
{
    static constexpr size_t kMinLength = 8;
    static constexpr size_t kMaxLength = Internal::kMaxWiFiKeyLength;
    size_t lLength;
    CHIP_ERROR lRetval = CHIP_NO_ERROR;

    // Do a quick character range sanity check.

    lLength = inPassphrase.size();
    VerifyOrExit(lLength >= kMinLength, outReason = kNetworkPassphraseIsTooShort; lRetval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(lLength <= kMaxLength, outReason = kNetworkPassphraseIsTooLong; lRetval = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    return lRetval;
}

static void _MaybeClearInterfaceName(const char * inInterfaceName, char * inOutInterfaceName) noexcept
{
    VerifyOrReturn(inInterfaceName != nullptr);
    VerifyOrReturn(inOutInterfaceName != nullptr);

    if (strncmp(inInterfaceName, inOutInterfaceName, Inet::InterfaceId::kMaxIfNameLength) == 0)
    {
        inOutInterfaceName[0] = '\0';
    }
}

static void _MaybeSetInterfaceName(const char * inDescription, const char * inInterfaceName, char * inOutInterfaceName) noexcept
{
    VerifyOrReturn(inDescription != nullptr);
    VerifyOrReturn(inInterfaceName != nullptr);
    VerifyOrReturn(inOutInterfaceName != nullptr);

    if (strncmp(inOutInterfaceName, inInterfaceName, Inet::InterfaceId::kMaxIfNameLength) != 0)
    {
        strncpy(inOutInterfaceName, inInterfaceName, Inet::InterfaceId::kMaxIfNameLength);
        inOutInterfaceName[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Got %s interface: %s", inDescription,
                        inOutInterfaceName);
    }
}

template <typename Callable>
static CHIP_ERROR InvokeOnGLibContextSyncTrampoline(GLibInvokeContext<Callable> * inContext) noexcept
{
    constexpr CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    inContext->mStatus = inContext->mCallable();

    return retval;
}

template <typename Callable>
static CHIP_ERROR InvokeOnGLibContextSync(Callable && inCallable) noexcept
{
    // Callable must be invocable and return CHIP_ERROR.
    using DecayedCallable = std::decay_t<Callable>;

    static_assert(std::is_same_v<std::invoke_result_t<DecayedCallable &>, CHIP_ERROR>,
                  "InvokeOnGLibMainLoopSync callable must return CHIP_ERROR");

    DecayedCallable callable(std::forward<Callable>(inCallable));
    GLibInvokeContext<DecayedCallable> context{ callable };

    ReturnErrorOnFailure(
        PlatformMgrImpl().GLibMatterContextInvokeSync(&InvokeOnGLibContextSyncTrampoline<DecayedCallable>, &context));

    return context.mStatus;
}

template <typename LockT, typename Callable>
auto UnlockAndInvoke(LockT & inOutLock, Callable && inCallable) noexcept -> decltype(inCallable())
{
    VerifyOrDie(inOutLock.owns_lock());

    inOutLock.unlock();

    auto retval = inCallable();

    inOutLock.lock();

    return retval;
}

template <typename LockT, typename Callable>
static CHIP_ERROR UnlockAndInvokeOnGLibContextSync(LockT & inOutLock, Callable && inCallable) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    return UnlockAndInvoke(inOutLock, [&]() { return InvokeOnGLibContextSync(std::forward<Callable>(inCallable)); });
}

} // namespace

// Matter Linux Connectivity Manager Implementation for connman

// Initialization

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::Init(ConnectivityManagerImpl & inConnectivityManagerImpl)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    // Initialize the base network management class.

    retval = NetworkManagementBasis::Init();
    SuccessOrExit(retval);

    mConnectivityManagerImpl = &inConnectivityManagerImpl;

    mEthernetIfName[0] = '\0';
    mWiFiActiveScanSsid.reset();
    mWiFiActiveScanCount = 0;
    mWiFiConnectPassphrase.reset();
    mWiFiConnectSsid.reset();
    mWiFiConnectScanCount         = 0;
    mWiFiIfName[0]                = '\0';
    mWiFiStationConnected         = false;
    mWiFiStationMode              = ConnectivityManager::kWiFiStationMode_NotSupported;
    mWiFiStationReconnectInterval = {};

exit:
    return retval;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::InitAgentOnGLib(ConnectivityManagerImpl_NetworkManagementConnMan * inSelf,
                                                                  GDBusConnection * inConnection, const char * inPath,
                                                                  ConnManAgent *& outSkeleton, GError ** outError) noexcept
{
    GAutoPtr<ConnManAgent> skeleton;
    GAutoPtr<GError> err;
    gboolean exported;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    VerifyOrExit(inSelf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inConnection != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    skeleton.reset(conn_man_agent_skeleton_new());
    VerifyOrExit(skeleton.get() != nullptr, retval = CHIP_ERROR_INTERNAL);

    // Register for the agent server methods: cancel, release, request
    // input, and report error.

    // Cancel

    g_signal_connect(skeleton.get(), "handle-cancel",
                     G_CALLBACK(+[](ConnManAgent * inAgent_, GDBusMethodInvocation * inInvocation_,
                                    ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) -> gboolean {
                         return inSelf_->OnAgentCancel(inAgent_, inInvocation_);
                     }),
                     inSelf);

    // Release

    g_signal_connect(skeleton.get(), "handle-release",
                     G_CALLBACK(+[](ConnManAgent * inAgent_, GDBusMethodInvocation * inInvocation_,
                                    ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) -> gboolean {
                         return inSelf_->OnAgentRelease(inAgent_, inInvocation_);
                     }),
                     inSelf);

    // Request Input

    g_signal_connect(
        skeleton.get(), "handle-request-input",
        G_CALLBACK(+[](ConnManAgent * inAgent_, GDBusMethodInvocation * inInvocation_, const gchar * inPath_,
                       GVariant * inProperties_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) -> gboolean {
            return inSelf_->OnAgentRequestInput(inAgent_, inInvocation_, inPath_, inProperties_);
        }),
        inSelf);

    // Report Error

    g_signal_connect(
        skeleton.get(), "handle-report-error",
        G_CALLBACK(+[](ConnManAgent * inAgent_, GDBusMethodInvocation * inInvocation_, const gchar * inPath_,
                       const gchar * inError_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) -> gboolean {
            return inSelf_->OnAgentReportError(inAgent_, inInvocation_, inPath_, inError_);
        }),
        inSelf);

    exported =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton.get()), inConnection, inPath, &err.GetReceiver());
    VerifyOrExit(exported && err.get() == nullptr, retval = CHIP_ERROR_INTERNAL);

    outSkeleton = CONN_MAN_AGENT(g_object_ref(skeleton.get()));

exit:
    if (!ChipError::IsSuccess(retval))
    {
        if (outError != nullptr)
        {
            *outError = err.get() ? g_error_copy(err.get()) : nullptr;
        }
    }

    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::ShutdownAgentLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                           const char * inPath) noexcept
{
    GAutoPtr<GDBusConnection> connection;
    GAutoPtr<ConnManAgent> skeleton;

    VerifyOrDie(inOutLock.owns_lock());

    if (mConnManAgentServer.mConnection)
    {
        connection.reset(static_cast<GDBusConnection *>(g_object_ref(mConnManAgentServer.mConnection.get())));
    }

    if (mConnManAgentServer.mSkeleton)
    {
        skeleton.reset(CONN_MAN_AGENT(g_object_ref(mConnManAgentServer.mSkeleton.get())));
    }

    mConnManAgentServer.mRegistered = false;
    mConnManAgentServer.mExported   = false;
    mConnManAgentServer.mSkeleton.reset();
    mConnManAgentServer.mConnection.reset();

    if (connection && skeleton)
    {
        TEMPORARY_RETURN_IGNORED UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
            ShutdownAgentOnGLib(this, connection.get(), inPath, skeleton.get());

            return CHIP_NO_ERROR;
        });
    }
}

void ConnectivityManagerImpl_NetworkManagementConnMan::ShutdownAgentOnGLib(
    ConnectivityManagerImpl_NetworkManagementConnMan * inSelf, GDBusConnection * inConnection, const char * inPath,
    ConnManAgent * inSkeleton) noexcept
{
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    (void) inSelf;
    (void) inPath;

    if (inConnection != nullptr && inSkeleton != nullptr)
    {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(inSkeleton), inConnection);
    }
}

// Observation

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    GAutoPtr<char> path;
    GVariant * props  = nullptr;
    const char * name = nullptr;
    size_t length;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    // Iterate over all Wi-Fi networks and get the best connected
    // network. From that network, copy its SSID.

    retval = GetBestServicePathForTypeLocked(kConnManServicePropertyTypeWiFiValue, mConnManClient.mServices.get(), path);
    SuccessOrExit(retval);

    retval = GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path.get(), props);
    SuccessOrExit(retval);

    retval = GetObjectNameFromPropertiesLocked(props, name);
    SuccessOrExit(retval);

    length = std::min(sizeof(outNetwork.networkID), strlen(name));

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Current connected network: %.*s", length, name);

    memcpy(outNetwork.networkID, name, length);
    outNetwork.networkIDLen = length;

exit:
    return retval;
}

// Event Handling

void ConnectivityManagerImpl_NetworkManagementConnMan::OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent)
{
    return;
}

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
// Ethernet Control Plane Management

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetEthernetIfName(void)
{
    return GetInterfaceName("Ethernet", kConnManServicePropertyTypeEthernetValue, &mEthernetIfName[0]);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::UpdateEthernetNetworkingStatus(void)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // TBD
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
// Wi-Fi Control Plane Management

// Observation

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiIfName(void)
{
    return GetInterfaceName("Wi-Fi", kConnManServicePropertyTypeWiFiValue, &mWiFiIfName[0]);
}

// Control

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::StartNetworkManagementOnGLib(void)
{
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Start network management");

    // Initiate an asynchronous request to establish

    conn_man_manager_proxy_new_for_bus(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kConnManServiceName, kConnManManagerPath, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            +[](GObject * sourceObject_, GAsyncResult * res_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                inSelf_->OnManagerReady(sourceObject_, res_);
            }),
        this);

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::StartNonConcurrentWiFiManagement(void)
{
    StartWiFiManagement();

    for (size_t cnt = 0; cnt < WIFI_START_CHECK_ATTEMPTS; cnt++)
    {
        if (IsWiFiManagementStarted())
        {
            TEMPORARY_RETURN_IGNORED DeviceControlServer::DeviceControlSvr().PostOperationalNetworkStartedEvent();

            ChipLogProgress(DeviceLayer, "Non-concurrent mode Wi-Fi Management Started.");

            return;
        }

        usleep(WIFI_START_CHECK_TIME_USEC);
    }

    ChipLogError(Ble, "Non-concurrent mode Wi-Fi Management taking too long to start.");
}

void ConnectivityManagerImpl_NetworkManagementConnMan::StartWiFiManagement(void)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    mConnManClient = GDBusConnManClient{};

    // IMPORTANT: Do not synchronously wait on the GLib thread while
    //            holding mConnManMutex.
    //
    // Use the explicit unlock/invoke/relock primitive to avoid
    // deadlock when GLib callbacks re-enter and attempt to take
    // mConnManMutex.

    CHIP_ERROR err = UnlockAndInvokeOnGLibContextSync(lock, [this]() -> CHIP_ERROR { return StartNetworkManagementOnGLib(); });

    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to start Wi-Fi management"));
}

// Wi-Fi Station Control Plane Management

// Introspection

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiManagementStarted(void)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const bool retval = !!mConnManClient.mManagerProxy;

    return retval;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationApplicationControlled(void)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const bool retval = (mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled);

    return retval;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationConnected(void)
{
    const bool retval = mWiFiStationConnected;

    // Get the Wi-Fi technology and return the value of the Connected property.
    //
    // We should possibly also consider the State property of the
    // currently-connected Wi-Fi network service.

    return retval;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationEnabled(void)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const bool retval = (mWiFiStationMode == ConnectivityManager::kWiFiStationMode_Enabled);

    return retval;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationProvisioned(void)
{
    bool retval = false;

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // TBD

    return retval;
}

// Observation

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiBssId(MutableByteSpan & outBssId)
{
    CHIP_ERROR retval = CHIP_ERROR_NOT_IMPLEMENTED;

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Use the nl80211 API to retrieve this, similar to 'iw dev wlan0
    // link', where "wlan0" is the network interface name of the
    // currently-connected Wi-Fi network service. Failing that, the
    // Wi-Fi backend could be used.

    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiSecurityType(
    app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    // ConnMan only distinguishes between "psk" and "rsn" security
    // types, lumping all of WPA-, WPA-2-, and WPA-3-Personal under
    // "psk". For now, just use 'kUnspecified' for now. We may be able
    // to get this through nl80211 or the Wi-Fi backend in the future.

    outSecurityType = SecurityTypeEnum::kUnspecified;

    return retval;
}

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiStationMode(void)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    return mWiFiStationMode;
}

System::Clock::Timeout ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiStationReconnectInterval(void)
{
    System::Clock::Timeout retval = mWiFiStationReconnectInterval;

    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiVersion(
    app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion)
{
    constexpr CHIP_ERROR retval = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    return retval;
}

// Mutation

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiStationMode(const ConnectivityManager::WiFiStationMode & inWiFiStationMode)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    ConnManTechnology * technology;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inWiFiStationMode != ConnectivityManager::kWiFiStationMode_NotSupported, retval = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != inWiFiStationMode)
    {
        ChipLogProgress(DeviceLayer, "Wi-Fi station mode change: %s -> %s",
                        ConnectivityManager::WiFiStationModeToStr(mWiFiStationMode),
                        ConnectivityManager::WiFiStationModeToStr(inWiFiStationMode));

        switch (inWiFiStationMode)
        {

            // If the mode is enabled/disabled, then set the Wi-Fi
            // technology "Powered" property to true/false.

        case ConnectivityManager::kWiFiStationMode_Disabled:
        case ConnectivityManager::kWiFiStationMode_Enabled: {
            const bool powered = (inWiFiStationMode == ConnectivityManager::kWiFiStationMode_Enabled);

            technology =
                GetTechnologyProxyFromTypeLocked(kConnManTechnologyPropertyTypeWiFiValue, mConnManClient.mTechnologyProxies.get(),
                                                 mConnManClient.mTechnologies.get());
            VerifyOrExit(technology != nullptr, retval = ChipError(ChipError::Range::kPOSIX, ENOENT));

            retval = TechnologySetPoweredLocked(lock, technology, powered);
            SuccessOrExit(retval);
            break;
        }

        default:
            break;
        }

        mWiFiStationMode = inWiFiStationMode;
    }

exit:
    return retval;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval)
{
    mWiFiStationReconnectInterval = inInterval;

    return CHIP_NO_ERROR;
}

// Worker

void ConnectivityManagerImpl_NetworkManagementConnMan::ClearWiFiStationProvision(void)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // TBD but probably remove all favorited networks.
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::CommitConfig(void)
{
    constexpr CHIP_ERROR retval = CHIP_NO_ERROR;

    // As presently implemented, there is nothing to do for this with
    // connman. On successful connection to a Wi-Fi (or other) network
    // services, the necessary credentials are serialized to
    // non-volatile storage.

    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ConnectWiFiNetworkAsync(
    const ByteSpan & inSsid, const ByteSpan & inCredentials,
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    const char * reason = nullptr;
    ConnManService * service;
    ConnManTechnology * technology;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    // Ensure that we have an initialized Connectivity Manager
    // implementation pointer.

    VerifyOrExit(mConnectivityManagerImpl != nullptr, retval = CHIP_ERROR_UNINITIALIZED);

    // Verify that there is not another Wi-Fi station connection in-flight.

    VerifyOrReturnError(!mConnectivityManagerImpl->IsWiFiStationConnecting(), CHIP_ERROR_BUSY);

    // Sanity check the SSID.

    retval = ValidateWiFiServiceSsid(inSsid, reason);
    ReturnErrorAndLogOnFailure(retval, DeviceLayer, "invalid Wi-Fi SSID '%.*s': %s", inSsid.size(), inSsid.data(), reason);

    // Sanity check the credentials.

    retval = ValidateWiFiServicePassphrase(inCredentials, reason);
    ReturnErrorAndLogOnFailure(retval, DeviceLayer, "invalid Wi-Fi passphrase: %s", reason);

    // Set the connection completion callback.

    mConnectivityManagerImpl->SetOneShotConnectCallback(inConnectCallback);

    // There will be one of two cases here given that connman cannot
    // connect to a service it does not know about or has not seen.
    //
    // The first, easiest, case is that connman does know about and
    // has seen the network service associated with the SSID. In that
    // case, 'GetWiFiServiceProxyFromSsidLocked' will succeed and the
    // network service connect will proceed normally.
    //
    // The second, more difficult, case is that connman does NOT know
    // about and has NOT seen the network service associated with the
    // SSID. In that case, 'GetWiFiServiceProxyFromSsidLocked' will
    // fail. As a result, we will need to initiate a Wi-Fi scan and
    // then attempt to proceed with the connect once the network
    // service has appeared in scan results.

    // Find a matching W-Fi network service proxy to the SSID.

    service = GetWiFiServiceProxyFromSsidLocked(inSsid, mConnManClient.mServiceProxies.get(), mConnManClient.mServices.get());
    if (service != nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting to '%.*s'...", inSsid.size(),
                        inSsid.data());

        // Cache the provided credentials.

        mWiFiConnectPassphrase = inCredentials;

        retval = HandleServiceConnectRequestLocked(lock, service);
        SuccessOrExit(retval);
    }
    else
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "scanning for '%.*s'...", inSsid.size(),
                        inSsid.data());

        technology = GetTechnologyProxyFromTypeLocked(kConnManTechnologyPropertyTypeWiFiValue,
                                                      mConnManClient.mTechnologyProxies.get(), mConnManClient.mTechnologies.get());
        VerifyOrExit(technology != nullptr, retval = ChipError(ChipError::Range::kPOSIX, ENOENT));

        // Cache the provided SSID.

        mWiFiConnectSsid = inSsid;

        // Cache the provided credentials.

        mWiFiConnectPassphrase = inCredentials;

        // Initialize the connect scan count.

        mWiFiConnectScanCount = 1;

        // Attempt to scan for the desired network service.

        retval = TechnologyScanLocked(lock, technology);
        SuccessOrExit(retval);
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ConnectWiFiNetworkWithPDCAsync(
    const ByteSpan & inSsid, const ByteSpan & inNetworkIdentity, const ByteSpan & inClientIdentity,
    const Crypto::P256Keypair & inClientIdentityKeypair,
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback)
{
    constexpr CHIP_ERROR retval = CHIP_ERROR_NOT_IMPLEMENTED;

    // ConnMan fully supports connecting Wi-Fi stations and access
    // points using certificate-based security; this implementation
    // would simply have to cache the input parameters to satisfy the
    // requisite agent input request from those cached input
    // parameters.

    return retval;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::StartWiFiScan(const ByteSpan & inSsid,
                                                                NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    bool has_technology;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    // Ensure that we have an initialized Connectivity Manager
    // implementation pointer.

    VerifyOrExit(mConnectivityManagerImpl != nullptr, retval = CHIP_ERROR_UNINITIALIZED);

    // If there is another ongoing scan request, reject the new one.

    VerifyOrExit(!mConnectivityManagerImpl->IsWiFiStationScanning(), retval = CHIP_ERROR_INCORRECT_STATE);

    // If Wi-Fi is not among the supported technologies, there is
    // nothing further to do.

    has_technology = HasTechnologyLocked(mConnManClient.mTechnologies.get(), kConnManTechnologyPropertyTypeWiFiValue);
    VerifyOrExit(has_technology, retval = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // ConnMan does not support active, directed scans. Consequently,
    // all we can reasonably do here is set a cache of the desired
    // SSID, do a Wi-Fi passive, broadcast scan, and then see if the
    // desired SSID is among the known Wi-Fi network services at the
    // completion of the scan.

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "scanning for '%.*s'...", inSsid.size(), inSsid.data());

    // Cache the provided SSID.

    mWiFiActiveScanSsid = inSsid;

    // Initialize the "active" scan count.

    mWiFiActiveScanCount = 1;

    mConnectivityManagerImpl->SetOneShotScanCallback(inScanCallback);

    retval = TechnologyScanLocked(lock,
                                  GetTechnologyProxyFromTypeLocked(kConnManTechnologyPropertyTypeWiFiValue,
                                                                   mConnManClient.mTechnologyProxies.get(),
                                                                   mConnManClient.mTechnologies.get()));
    SuccessOrExit(retval);

exit:
    if (!ChipError::IsSuccess(retval))
    {
        mConnectivityManagerImpl->SetOneShotScanCallback(nullptr);

        mWiFiActiveScanCount = 0;
        mWiFiActiveScanSsid.reset();
    }

    return retval;
}

// Wi-Fi Soft Access Point (AP) Control Plane Management

// Observation

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiApMode(void)
{
    constexpr ConnectivityManager::WiFiAPMode retval = ConnectivityManager::kWiFiAPMode_NotSupported;

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.

    return retval;
}

// Mutation

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode)
{
    constexpr CHIP_ERROR retval = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.

    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}

// Control

void ConnectivityManagerImpl_NetworkManagementConnMan::DemandStartWiFiAp(void)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}

void ConnectivityManagerImpl_NetworkManagementConnMan::StopOnDemandWiFiAp(void)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}

void ConnectivityManagerImpl_NetworkManagementConnMan::MaintainOnDemandWiFiAp(void)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

// Helper Methods

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetInterfaceName(const char * inDescription, const char * inType,
                                                                                char * inOutInterfaceName) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    GAutoPtr<char> interface;
    CHIP_ERROR status;
    const char * retval = nullptr;

    nlEXPECT(inDescription != nullptr, exit);
    nlEXPECT(inType != nullptr, exit);
    nlEXPECT(inOutInterfaceName != nullptr, exit);

    // If we have a valid interface name, return it. Otherwise,
    // perform a lazy update of the interface name based on the best
    // network services of the specified type.

    if (inOutInterfaceName[0] != '\0')
    {
        retval = inOutInterfaceName;
    }
    else
    {
        if (mConnManClient.mServices)
        {
            status = GetBestServiceInterfaceForTypeLocked(inType, mConnManClient.mServices.get(), interface);
            SuccessOrExit(status);

            if (interface.get() != nullptr)
            {
                _MaybeSetInterfaceName(inDescription, interface.get(), &inOutInterfaceName[0]);

                retval = inOutInterfaceName;
            }
        }
    }

exit:
    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetProperties(ConnManManager * inManager,
                                                                                  GVariant * inProperties,
                                                                                  const GError * inError) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    CHIP_ERROR status;

    VerifyOrReturn(inProperties != nullptr);

    if (inError == nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "got %zu manager property/ies",
                        g_variant_n_children(inProperties));

        status = UpdateManagerPropertiesLocked(inProperties);
        SuccessOrExit(status);

        status = HandleManagerPropertiesChangedLocked(inManager, inProperties);
        SuccessOrExit(status);
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to get manager properties: %s",
                     inError ? inError->message : "unknown error");
    }

exit:
    return;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetServices(ConnManManager * inManager, GVariant * inServices,
                                                                                const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    CHIP_ERROR status;

    VerifyOrReturn(inServices != nullptr);

    if (inError == nullptr)
    {
        GHashTableIter iter;
        gpointer key   = nullptr;
        gpointer value = nullptr;

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "got %zu manager service(s)",
                        g_variant_n_children(inServices));

        status = UpdateServicesLocked(inServices);
        SuccessOrExit(status);

        g_hash_table_iter_init(&iter, mConnManClient.mServices.get());

        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            const char * const path  = static_cast<const char *>(key);
            GVariant * const props   = static_cast<GVariant *>(value);
            const char * type        = nullptr;
            ConnManService * service = nullptr;

            status = UpdateServiceProxyLocked(lock, path, service);
            SuccessOrExit(status);

            // Prime service-derived state (for example, interface
            // name) from the initial service enumeration (that is,
            // "big get").

            status = GetObjectTypeFromPropertiesLocked(props, type);
            SuccessOrExit(status);

            status = HandleServicePropertiesChangedLocked(G_DBUS_PROXY(service), path, type, props);
            SuccessOrExit(status);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to get manager services: %s",
                     inError ? inError->message : "unknown error");
    }

exit:
    return;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetTechnologies(ConnManManager * inManager,
                                                                                    GVariant * inTechnologies,
                                                                                    const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    CHIP_ERROR status;

    VerifyOrReturn(inTechnologies != nullptr);

    if (inError == nullptr)
    {
        GHashTableIter iter;
        gpointer key   = nullptr;
        gpointer value = nullptr;

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "got %zu manager technology/ies",
                        g_variant_n_children(inTechnologies));

        status = UpdateTechnologiesLocked(inTechnologies);
        SuccessOrExit(status);

        g_hash_table_iter_init(&iter, mConnManClient.mTechnologies.get());

        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            const char * const path        = static_cast<const char *>(key);
            GVariant * const props         = static_cast<GVariant *>(value);
            const char * type              = nullptr;
            ConnManTechnology * technology = nullptr;

            status = UpdateTechnologyProxyLocked(lock, path, technology);
            SuccessOrExit(status);

            status = GetObjectTypeFromPropertiesLocked(props, type);
            SuccessOrExit(status);

            status = HandleTechnologyPropertiesChangedLocked(G_DBUS_PROXY(technology), path, type, props);
            SuccessOrExit(status);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to get manager technologies: %s",
                     inError ? inError->message : "unknown error");
    }

exit:
    return;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerPropertiesChangedLocked(ConnManManager * inManager,
                                                                                                  GVariant * inProperties) noexcept
{
    static const char * const kDescription = "manager";

    return HandleObjectPropertiesChangedLocked(
        kDescription, G_DBUS_PROXY(inManager), inProperties,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerPropertyChangedAnyLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerPropertyChanged(ConnManManager * inManager, const char * inKey,
                                                                                    GVariant * inValue) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    CHIP_ERROR status;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inKey != nullptr);
    VerifyOrReturn(inValue != nullptr);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "manager property changed");

    // First, merge the changed property back into the local cache
    // such that any subsequent gets in reaction to the changed
    // property correctly reflect the current, rather than previous,
    // state.

    status = MergeObjectPropertiesLocked("manager (property changed)", mConnManClient.mProperties.get(), CONNMAN_MANAGER_PATH,
                                         inKey, inValue);
    ReturnOnFailure(status);

    status = HandleManagerPropertyChangedAnyLocked(G_DBUS_PROXY(inManager), inKey, inValue);
    ReturnOnFailure(status);
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerPropertyChangedAnyLocked(GDBusProxy * inManager, const char * inKey,
                                                                                        GVariant * inMaybeVariant) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inMaybeVariant != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    UnboxedVariant value(inMaybeVariant);
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);

    retval = HandleManagerPropertyChangedLocked(CONN_MAN_MANAGER(inManager), inKey, value.get());
    SuccessOrExit(retval);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerPropertyChangedLocked(ConnManManager * inManager,
                                                                                                const char * inKey,
                                                                                                GVariant * inValue) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inManager != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inKey != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inValue != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "manager \"%s\" property changed", inKey);

exit:
    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerServicesChanged(ConnManManager * inManager,
                                                                                    GVariant * inServicesChanged,
                                                                                    const char * const * inServicesRemoved) noexcept

{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inServicesChanged != nullptr);

    // Handle removed service paths.

    HandleManagerServicesRemovedLocked(lock, inManager, inServicesRemoved);

    // Handle changed service objects.

    HandleManagerServicesChangedLocked(lock, inManager, inServicesChanged);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerServicesChangedLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                          ConnManManager * inManager,
                                                                                          GVariant * inServicesChanged) noexcept
{
    GVariantIter iter;
    const char * path = nullptr;
    GVariant * props  = nullptr;
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inManager != nullptr, status = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inServicesChanged != nullptr, status = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(g_variant_is_of_type(inServicesChanged, G_VARIANT_TYPE("a(oa{sv})")), status = CHIP_ERROR_INVALID_ARGUMENT);

    // Add or update any changed services to the hash table.

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%zu manager services changed",
                    g_variant_n_children(inServicesChanged));

    // Per the connman manager interface documentation:
    //
    //    "This signal indicates a change in the services. List of all
    //     services currently registered is passed. The list of all
    //     services is sorted. The dictionary with the properties
    //     might be empty in case none of the properties have
    //     changed. Or only contains the properties that have changed.
    //
    //    "For newly added services the whole set of properties will
    //     be present.
    //
    // With that in mind, we need to carefully merge the received
    // property changes, if any, against the base set of properties
    // from the initial "big get" or a first-time appearance of a
    // service via this handler.

    status = MergeObjectPropertiesLocked("services (changed)", mConnManClient.mServices.get(), inServicesChanged);
    SuccessOrExit(status);

    g_variant_iter_init(&iter, inServicesChanged);

    while (g_variant_iter_next(&iter, "(&o@a{sv})", &path, &props))
    {
        const char * type        = nullptr;
        ConnManService * service = nullptr;
        GVariant * current       = nullptr;

        ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: %s has %zu properties", __func__, path,
                      g_variant_n_children(props));

        status = UpdateServiceProxyLocked(inOutLock, path, service);
        SuccessOrExit(status);

        status = GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path, current);

        // Funnel the changed service properties through the same
        // handler used for per-service property changed signals.

        status = GetObjectTypeFromPropertiesLocked(current, type);
        SuccessOrExit(status);

        status = HandleServicePropertiesChangedLocked(G_DBUS_PROXY(service), path, type, props);
        SuccessOrExit(status);
    }

exit:
    if (!ChipError::IsSuccess(status))
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to handle changed services:%" CHIP_ERROR_FORMAT,
                     status.Format());
    }
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerServicesRemovedLocked(
    std::unique_lock<std::mutex> & inOutLock, ConnManManager * inManager, const char * const * inServicesRemoved) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturn(inManager != nullptr);

    // Drop any removed services from the hash table.

    if (inServicesRemoved != nullptr)
    {
        size_t removed = 0;

        for (const char * const * ppath = inServicesRemoved; *ppath != nullptr; ++ppath, ++removed)
        {
            MaybeClearInterfaceNameLocked(*ppath);

            RemoveServiceLocked(*ppath);
        }
    }
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerTechnologyAdded(ConnManManager * inManager, const char * inPath,
                                                                                    GVariant * inProperties) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    const char * type              = nullptr;
    ConnManTechnology * technology = nullptr;
    CHIP_ERROR status;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inPath != nullptr);
    VerifyOrReturn(inProperties != nullptr);

    VerifyOrReturn(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT));

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "manager technology added");

    g_hash_table_replace(mConnManClient.mTechnologies.get(), g_strdup(inPath), g_variant_ref(inProperties));

    status = UpdateTechnologyProxyLocked(lock, inPath, technology);
    ReturnOnFailure(status);

    status = GetObjectTypeFromPathLocked(mConnManClient.mTechnologies.get(), inPath, type);
    ReturnOnFailure(status);

    status = HandleTechnologyPropertiesChangedLocked(G_DBUS_PROXY(technology), inPath, type, inProperties);
    ReturnOnFailure(status);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerTechnologyRemoved(ConnManManager * inManager,
                                                                                      const char * inPath) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const char * type = nullptr;
    CHIP_ERROR status;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inPath != nullptr);

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "manager technology removed");

    // If the Wi-Fi technology type has been removed, update the Wi-Fi
    // station connected state and mode accordingly.

    status = GetObjectTypeFromPathLocked(mConnManClient.mTechnologies.get(), inPath, type);
    if (ChipError::IsSuccess(status))
    {
        if (strcmp(type, kConnManTechnologyPropertyTypeWiFiValue) == 0)
        {
            const auto newMode = ConnectivityManager::kWiFiStationMode_NotSupported;

            if (mWiFiStationMode != newMode)
            {
                ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Wi-Fi station mode: %s -> %s",
                                ConnectivityManager::WiFiStationModeToStr(mWiFiStationMode),
                                ConnectivityManager::WiFiStationModeToStr(newMode));

                mWiFiStationMode = newMode;
            }

            mWiFiStationConnected = false;
        }
    }

    RemoveTechnologyLocked(inPath);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleObjectPropertiesChangedLocked(
    const char * inDescription, GDBusProxy * inProxy, GVariant * inProperties,
    ObjectPropertiesChangedAnyLockedMethod inObjectPropertiesChangedAnyLockedMethod) noexcept
{
    size_t count;
    GVariantIter iter;
    const char * key = nullptr;
    GAutoPtr<GVariant> boxed;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inDescription != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPropertiesChangedAnyLockedMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_INVALID_ARGUMENT);

    count = g_variant_n_children(inProperties);
    if (count > 0)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%zu %s property/ies changed", count, inDescription);

        g_variant_iter_init(&iter, inProperties);

        while (g_variant_iter_next(&iter, "{&s@v}", &key, &boxed.GetReceiver()))
        {
            VerifyOrExit(key != nullptr, retval = CHIP_ERROR_INTERNAL);
            VerifyOrExit(boxed.get() != nullptr, retval = CHIP_ERROR_INTERNAL);

            retval = (this->*inObjectPropertiesChangedAnyLockedMethod)(inProxy, key, boxed.get());
            SuccessOrExit(retval);
        }
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleObjectPropertiesChangedLocked(
    const char * inDescription, GDBusProxy * inProxy, const char * inPath, const char * inType, GVariant * inProperties,
    TypedObjectPropertiesChangedAnyLockedMethod inTypedObjectPropertiesChangedAnyLockedMethod) noexcept
{
    size_t count;
    GVariantIter iter;
    const char * key = nullptr;
    GAutoPtr<GVariant> boxed;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inDescription != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inTypedObjectPropertiesChangedAnyLockedMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_INVALID_ARGUMENT);

    count = g_variant_n_children(inProperties);
    if (count > 0)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%zu %s %s property/ies changed", count, inType,
                        inDescription);

        g_variant_iter_init(&iter, inProperties);

        while (g_variant_iter_next(&iter, "{&s@v}", &key, &boxed.GetReceiver()))
        {
            VerifyOrExit(key != nullptr, retval = CHIP_ERROR_INTERNAL);
            VerifyOrExit(boxed.get() != nullptr, retval = CHIP_ERROR_INTERNAL);

            retval = (this->*inTypedObjectPropertiesChangedAnyLockedMethod)(inProxy, inPath, inType, key, boxed.get());
            SuccessOrExit(retval);
        }
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleObjectPropertyChangedAnyLocked(
    GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey, GVariant * inMaybeVariant,
    TypedObjectPropertyChangedLockedMethod inTypedObjectPropertyChangedLockedMethod) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inMaybeVariant != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inTypedObjectPropertyChangedLockedMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    UnboxedVariant value(inMaybeVariant);
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);

    retval = (this->*inTypedObjectPropertyChangedLockedMethod)(inProxy, inPath, inType, inKey, value.get());
    SuccessOrExit(retval);

exit:
    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleServiceConnectComplete(GDBusProxy * inService,
                                                                                    const GError * inError) noexcept
{

    std::unique_lock<std::mutex> lock(mConnManMutex);
    CharSpan debug_text;
    const char * path = nullptr;
    GVariant * props  = nullptr;
    const char * name = nullptr;
    const char * type = nullptr;
    CHIP_ERROR status;

    VerifyOrReturn(inService != nullptr);

    status = GetObjectPathFromProxyLocked(inService, path);
    SuccessOrExit(status);

    status = GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path, props);
    SuccessOrExit(status);

    status = GetObjectTypeFromPropertiesLocked(props, type);
    SuccessOrExit(status);

    status = GetObjectNameFromPropertiesLocked(props, name);
    if (ChipError::IsSuccess(status))
    {
        debug_text = CharSpan::fromCharString(name);
    }

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s service %s connect done",
                    type != nullptr ? type : "<null>", path);

    if (inError == nullptr)
    {
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, debug_text]() {
            mConnectivityManagerImpl->OnConnectResult(NetworkCommissioning::Status::kSuccess, debug_text, 0);
        });
    }
    else
    {
        WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();
        constexpr int reason               = 0;

        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to connect %s service: %s", type,
                     inError->message);

        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, debug_text, reason]() {
            mConnectivityManagerImpl->OnConnectResult(NetworkCommissioning::Status::kUnknownError, debug_text, reason);
        });

        if (delegate != nullptr)
        {
            constexpr uint8_t associationFailureCause   = static_cast<uint8_t>(AssociationFailureCauseEnum::kUnknown);
            constexpr uint16_t associationFailureStatus = WLAN_STATUS_UNSPECIFIED_FAILURE;

            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
                [delegate, associationFailureCause, associationFailureStatus]() {
                    delegate->OnAssociationFailureDetected(associationFailureCause, associationFailureStatus);
                });
        }
    }

exit:
    // Reset the SSID and scan count.

    mWiFiConnectScanCount = 0;
    mWiFiConnectSsid.reset();

    // Clear the pending service.

    mConnManAgentServer.mPendingService.reset();

    if (mConnManAgentServer.mRegistered)
    {
        // Unregister ourselves as the manager agent.

        status = ManagerUnregisterAgentLocked(lock, MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH);
        if (!ChipError::IsSuccess(status))
        {
            ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to unregister agent:%" CHIP_ERROR_FORMAT,
                         status.Format());
        }
    }
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::HandleServiceConnectRequestLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                    ConnManService * inService) noexcept
{
    CharSpan debug_text;
    const char * path = nullptr;
    const char * name = nullptr;
    GVariant * props  = nullptr;
    CHIP_ERROR status;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inService != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    // Attempt to get the service path, properties, and name to set
    // the "debug text".

    status = GetObjectPathFromProxyLocked(G_DBUS_PROXY(inService), path);
    if (ChipError::IsSuccess(status))
    {
        status = GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path, props);
        if (ChipError::IsSuccess(status))
        {
            status = GetObjectNameFromPropertiesLocked(props, name);
            if (ChipError::IsSuccess(status))
            {
                debug_text = CharSpan::fromCharString(name);
            }
        }
    }

    // Register ourselves as the manager agent.

    retval = ManagerRegisterAgentLocked(inOutLock, MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH);
    SuccessOrExit(retval);

    // Cache the pending service we are connecting.

    mConnManAgentServer.mPendingService.reset(g_object_ref(inService));

    // Connect to the specified network service.

    retval = ServiceConnectLocked(inOutLock, inService);
    SuccessOrExit(retval);

exit:
    if (!ChipError::IsSuccess(retval))
    {
        if (mConnManAgentServer.mRegistered)
        {
            status = ManagerUnregisterAgentLocked(inOutLock, MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH);
            if (!ChipError::IsSuccess(status))
            {
                ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to unregister agent:%" CHIP_ERROR_FORMAT,
                             status.Format());
            }
        }

        mConnectivityManagerImpl->OnConnectResult(Status::kUnknownError, debug_text, retval.GetValue());
    }

    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertiesChangedLocked(GDBusProxy * inProxy,
                                                                                                  const char * inPath,
                                                                                                  const char * inType,
                                                                                                  GVariant * inProperties) noexcept
{
    static const char * const kDescription = "service";

    return HandleObjectPropertiesChangedLocked(
        kDescription, inProxy, inPath, inType, inProperties,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedAnyLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChanged(ConnManService * inService, const char * inKey,
                                                                                    GVariant * inValue) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const char * path = nullptr;
    const char * type = nullptr;
    CHIP_ERROR status;

    VerifyOrReturn(inService != nullptr);
    VerifyOrReturn(inKey != nullptr);
    VerifyOrReturn(inValue != nullptr);

    status = GetObjectPathFromProxyLocked(G_DBUS_PROXY(inService), path);
    ReturnOnFailure(status);

    // First, merge the changed property back into the local cache
    // such that any subsequent gets in reaction to the changed
    // property correctly reflect the current, rather than previous,
    // state.

    status = MergeObjectPropertiesLocked("service (property changed)", mConnManClient.mServices.get(), path, inKey, inValue);
    ReturnOnFailure(status);

    status = GetObjectTypeFromPathLocked(mConnManClient.mServices.get(), path, type);
    ReturnOnFailure(status);

    status = HandleServicePropertyChangedAnyLocked(G_DBUS_PROXY(inService), path, type, inKey, inValue);
    ReturnOnFailure(status);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedLocked(
    GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey, GVariant * inValue) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inProxy != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inType != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inKey != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inValue != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s service \"%s\" property changed", inType, inKey);

    if (strcmp(inKey, kConnManServicePropertyEthernetKey) == 0)
    {
        GAutoPtr<char> interface;

        retval = GetServiceInterfaceFromEthernetLocked(inValue, interface);
        SuccessOrExit(retval);

        retval = MaybeSetInterfaceNameLocked(inType, interface.get());
        SuccessOrExit(retval);
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedAnyLocked(
    GDBusProxy * inService, const char * inPath, const char * inType, const char * inKey, GVariant * inMaybeVariant) noexcept
{
    return HandleObjectPropertyChangedAnyLocked(
        G_DBUS_PROXY(inService), inPath, inType, inKey, inMaybeVariant,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedLocked);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertiesChangedLocked(
    GDBusProxy * inProxy, const char * inPath, const char * inType, GVariant * inProperties) noexcept
{
    static const char * const kDescription = "technology";

    return HandleObjectPropertiesChangedLocked(
        kDescription, inProxy, inPath, inType, inProperties,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedAnyLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChanged(ConnManTechnology * inTechnology,
                                                                                       const char * inKey,
                                                                                       GVariant * inValue) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const char * path = nullptr;
    const char * type = nullptr;
    CHIP_ERROR status;

    VerifyOrReturn(inTechnology != nullptr);
    VerifyOrReturn(inKey != nullptr);
    VerifyOrReturn(inValue != nullptr);

    status = GetObjectPathFromProxyLocked(G_DBUS_PROXY(inTechnology), path);
    ReturnOnFailure(status);

    // First, merge the changed property back into the local cache
    // such that any subsequent gets in reaction to the changed
    // property correctly reflect the current, rather than previous,
    // state.

    status = MergeObjectPropertiesLocked("technology (property changed)", mConnManClient.mTechnologies.get(), path, inKey, inValue);
    ReturnOnFailure(status);

    status = GetObjectTypeFromPathLocked(mConnManClient.mTechnologies.get(), path, type);
    ReturnOnFailure(status);

    status = HandleTechnologyPropertyChangedAnyLocked(G_DBUS_PROXY(inTechnology), path, type, inKey, inValue);
    ReturnOnFailure(status);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedLocked(
    GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey, GVariant * inValue) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inProxy != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inType != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inKey != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inValue != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s technology \"%s\" property changed", inType, inKey);

    if (strcmp(inKey, kConnManTechnologyPropertyConnectedKey) == 0)
    {
        VerifyOrExit(g_variant_is_of_type(inValue, G_VARIANT_TYPE_BOOLEAN), retval = CHIP_ERROR_WRONG_KEY_TYPE);

        const bool connected = g_variant_get_boolean(inValue);

        if (strcmp(inType, kConnManTechnologyPropertyTypeWiFiValue) == 0)
        {
            if (mWiFiStationConnected != connected)
            {
                ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Wi-Fi station connected: %u -> %u",
                                mWiFiStationConnected, connected);

                mWiFiStationConnected = connected;
            }
        }
    }
    else if (strcmp(inKey, kConnManTechnologyPropertyPoweredKey) == 0)
    {
        VerifyOrExit(g_variant_is_of_type(inValue, G_VARIANT_TYPE_BOOLEAN), retval = CHIP_ERROR_WRONG_KEY_TYPE);

        const bool powered = g_variant_get_boolean(inValue);

        ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s %s=%u", inType, kConnManTechnologyPropertyPoweredKey,
                      powered);

        if (strcmp(inType, kConnManTechnologyPropertyTypeWiFiValue) == 0)
        {
            const auto newMode =
                (powered ? ConnectivityManager::kWiFiStationMode_Enabled : ConnectivityManager::kWiFiStationMode_Disabled);

            if (mWiFiStationMode != newMode)
            {
                ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Wi-Fi station mode: %s -> %s",
                                ConnectivityManager::WiFiStationModeToStr(mWiFiStationMode),
                                ConnectivityManager::WiFiStationModeToStr(newMode));

                mWiFiStationMode = newMode;
            }
        }
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedAnyLocked(
    GDBusProxy * inTechnology, const char * inPath, const char * inType, const char * inKey, GVariant * inMaybeVariant) noexcept
{
    return HandleObjectPropertyChangedAnyLocked(
        inTechnology, inPath, inType, inKey, inMaybeVariant,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyScanComplete(GDBusProxy * inTechnology,
                                                                                    const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    const char * type = nullptr;
    CHIP_ERROR status;

    VerifyOrReturn(inTechnology != nullptr);

    status = GetObjectTypeFromProxyLocked(G_DBUS_PROXY(inTechnology), mConnManClient.mTechnologies.get(), type);
    SuccessOrExit(status);

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s technology scan done", type);

    if (inError == nullptr)
    {
        if (strcmp(type, kConnManTechnologyPropertyTypeWiFiValue) == 0)
        {
            status = HandleWiFiScanCompleteLocked(lock, CONN_MAN_TECHNOLOGY(inTechnology));
            SuccessOrExit(status);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to scan %s technology: %s", type,
                     inError->message);
    }

exit:
    return;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiPendingConnectLocked(
    std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology, size_t & inOutScanCount,
    Internal::WiFiSSIDFixedBuffer & inOutSsid) noexcept
{
    const CharSpan debug_text(reinterpret_cast<const char *>(inOutSsid.data()), inOutSsid.size());
    ConnManService * service;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inTechnology != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    service =
        GetWiFiServiceProxyFromSsidLocked(inOutSsid.span(), mConnManClient.mServiceProxies.get(), mConnManClient.mServices.get());
    if (service != nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting to '%.*s' after %zu scan(s)...",
                        inOutSsid.size(), inOutSsid.data(), inOutScanCount);

        retval = HandleServiceConnectRequestLocked(inOutLock, service);
        SuccessOrExit(retval);
    }
    else
    {
        if (inOutScanCount < kWiFiConnectScanLimit)
        {
            ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "could not connect to '%.*s' after %zu scan(s)",
                         inOutSsid.size(), inOutSsid.data(), mWiFiConnectScanCount);

            // Reset the SSID and scan count.

            inOutScanCount = 0;
            inOutSsid.reset();

            // Set the return status to POSIX:ENOENT and asychronously
            // delegate Status::kNetworkNotFound with the Wi-Fi SSID
            // as the "debug text".

            retval = ChipError(ChipError::Range::kPOSIX, ENOENT);
            SuccessOrExitAction(retval, mConnectivityManagerImpl->OnConnectResult(Status::kNetworkNotFound, debug_text, 0));
        }
        else
        {
            retval = HandleWiFiScanRetryLocked(inOutLock, inTechnology, "count not connect to", inOutScanCount, inOutSsid);
            SuccessOrExit(retval);
        }
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiPendingScanLocked(
    std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology, size_t & inOutScanCount,
    Internal::WiFiSSIDFixedBuffer & inOutSsid) noexcept
{
    const CharSpan debug_text(reinterpret_cast<const char *>(inOutSsid.data()), inOutSsid.size());
    GAutoPtr<GVariant> properties;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inTechnology != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    // If no matching Wi-Fi network service is among the network
    // services, post a success with an empty/null result.

    retval = GetWiFiServicePropertiesFromSsidLocked(inOutSsid.span(), mConnManClient.mServices.get(), properties);

    if (ChipError::IsSuccess(retval))
    {
        std::vector<WiFiScanResponse> * services = new (std::nothrow) std::vector<WiFiScanResponse>();
        VerifyOrExit(services != nullptr, retval = CHIP_ERROR_NO_MEMORY);

        WiFiScanResponse response;
        int8_t strength = 0;

        // ConnMan only distinguishes between "psk" and "rsn" security
        // types, lumping all of WPA-, WPA-2-, and WPA-3-Personal
        // under "psk". For now, just use 'kWpa2Personal' as a proxy.
        // In the future, we may be able to get this through nl80211
        // or the Wi-Fi backend.

        response.security.Set(WiFiSecurityBitmap::kWpa2Personal);

        memcpy(response.ssid, inOutSsid.data(), inOutSsid.size());
        response.ssidLen = inOutSsid.size();

        memset(&response.bssid, 0, sizeof(response.bssid));

        // ConnMan does not expose an introspectable channel, band, or
        // frequency for wireless networks. For now, set proxy
        // values. In the future, we may be able to get this through
        // nl80211 or the Wi-Fi backend.

        response.channel  = 0;
        response.wiFiBand = WiFiBandEnum::k5g;

        // By default, ConnMan virtualizes all wireless signal
        // strength in the qualitative range 0-100; indicate and set
        // the value as such. In the future, we may be able to return
        // dBm quantiatively via nl80211 or the Wi-Fi backend.

        retval = GetWiFiServiceStrengthLocked(properties.get(), strength);

        response.signal.type     = WirelessSignalType::kQualitative;
        response.signal.strength = strength;

        services->push_back(response);

        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, services, debug_text]() {
            LinuxScanResponseIterator<WiFiScanResponse> iter(services);

            mConnectivityManagerImpl->OnScanFinished(Status::kSuccess, debug_text, &iter);

            delete services;
        });
    }
    else
    {
        if (inOutScanCount < kWiFiActiveScanLimit)
        {
            ChipLogError(DeviceLayer,
                         CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "no Wi-Fi network found matching SSID '%.*s' after %zu scan(s)",
                         inOutSsid.size(), inOutSsid.data(), inOutScanCount);

            // Reset the SSID and scan count.

            inOutScanCount = 0;
            inOutSsid.reset();

            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
                [this, debug_text]() { mConnectivityManagerImpl->OnScanFinished(Status::kNetworkNotFound, debug_text, nullptr); });
        }
        else
        {
            retval =
                HandleWiFiScanRetryLocked(inOutLock, inTechnology, "no Wi-Fi netork found matching", inOutScanCount, inOutSsid);
            SuccessOrExit(retval);
        }
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiScanCompleteLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                          ConnManTechnology * inTechnology) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inTechnology != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    // A Wi-Fi scan may have been initiated due to an active, directed
    // scan for a specific SSID, from a service connect for a specific
    // SSID, or as just a general passive, background scan. We handle
    // those first two cases independently and in parallel. There is
    // nothing to do for the third case.

    // If there is a pending asynchronous connect that required a
    // scan, handle it.

    if (!mWiFiConnectSsid.empty())
    {
        retval = HandleWiFiPendingConnectLocked(inOutLock, inTechnology, mWiFiConnectScanCount, mWiFiConnectSsid);
        SuccessOrExit(retval);
    }

    // If there is a pending asynchronous active, directed scan,
    // handle it.

    if (!mWiFiActiveScanSsid.empty())
    {
        retval = HandleWiFiPendingScanLocked(inOutLock, inTechnology, mWiFiActiveScanCount, mWiFiActiveScanSsid);
        SuccessOrExit(retval);
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiScanRetryLocked(
    std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology, const char * inReason, size_t & inOutScanCount,
    const Internal::WiFiSSIDFixedBuffer & inSsid) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inReason != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s SSID '%.*s' after %zu scan(s): retrying...", inReason,
                    inSsid.size(), inSsid.data(), inOutScanCount);

    // Increment the scan count.

    inOutScanCount++;

    // Attempt to scan, again, for the desired network service.

    retval = TechnologyScanLocked(inOutLock, inTechnology);
    SuccessOrExit(retval);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerAgentOpLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                  const char * inPath,
                                                                                  ManagerAgentOpFunc inManagerAgentOpFunc,
                                                                                  const char * inAction,
                                                                                  const bool & inRegister) noexcept
{
    GAutoPtr<ConnManManager> manager;
    GAutoPtr<char> message;
    gboolean ok = FALSE;
    CHIP_ERROR status;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inManagerAgentOpFunc != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inAction != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(mConnManClient.mManagerProxy, retval = CHIP_ERROR_UNINITIALIZED);

    VerifyOrExit(mConnManAgentServer.mRegistered != inRegister, retval = CHIP_ERROR_INCORRECT_STATE);

    // Hold a reference to the manager client proxy across the unlock boundary.

    manager.reset(static_cast<ConnManManager *>(g_object_ref(mConnManClient.mManagerProxy.get())));

    // Unlock and synchronously invoke the manager agent operation
    // function on the GLib context.

    status = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<GError> err;
        ok = inManagerAgentOpFunc(manager.get(), inPath, nullptr, &err.GetReceiver());

        if (!ok)
        {
            message.reset(g_strdup((err.get() && err->message) ? err->message : "unknown error"));
        }

        return CHIP_NO_ERROR;
    });
    ReturnErrorOnFailure(status);

    if (!ok)
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "could not %s manager agent %s: %s", inAction, inPath,
                     message.get() ? message.get() : "unknown error");

        ExitNow(retval = CHIP_ERROR_INTERNAL);
    }

    mConnManAgentServer.mRegistered = inRegister;

exit:
    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::MaybeClearInterfaceNameLocked(const char * inPath) noexcept
{
    const char * type = nullptr;
    GAutoPtr<char> interface;
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturn(inPath != nullptr);

    status = GetObjectTypeFromPathLocked(mConnManClient.mServices.get(), inPath, type);
    SuccessOrExit(status);

    status = GetServiceInterfaceFromPathLocked(mConnManClient.mServices.get(), inPath, interface);
    SuccessOrExit(status);

    // If an Ethernet service was removed, clear the Ethernet
    // interface name data member if they match.
    //
    // Otherwise, if a Wi-Fi service was removed, clear the Wi-Fi
    // interface name data member if they match.

    if (strcmp(type, kConnManServicePropertyTypeEthernetValue) == 0)
    {
        _MaybeClearInterfaceName(interface.get(), &mEthernetIfName[0]);
    }
    else if (strcmp(type, kConnManServicePropertyTypeWiFiValue) == 0)
    {
        _MaybeClearInterfaceName(interface.get(), &mWiFiIfName[0]);
    }

exit:
    return;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::MaybeSetInterfaceNameLocked(const char * inType,
                                                                                         const char * inInterface) noexcept
{
    GAutoPtr<char> interface;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inType != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inInterface != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    // If an Ethernet service was removed, clear the Ethernet
    // interface name data member if they match.
    //
    // Otherwise, if a Wi-Fi service was removed, clear the Wi-Fi
    // interface name data member if they match.

    if (strcmp(inType, kConnManServicePropertyTypeEthernetValue) == 0)
    {
        _MaybeSetInterfaceName("Ethernet", inInterface, &mEthernetIfName[0]);
    }
    else if (strcmp(inType, kConnManServicePropertyTypeWiFiValue) == 0)
    {
        _MaybeSetInterfaceName("Wi-Fi", inInterface, &mWiFiIfName[0]);
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::RemoveServiceLocked(const char * inPath) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    retval = RemoveObjectLocked(mConnManClient.mServices.get(), inPath, mConnManClient.mServiceProxies.get());

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::RemoveTechnologyLocked(const char * inPath) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    retval = RemoveObjectLocked(mConnManClient.mTechnologies.get(), inPath, mConnManClient.mTechnologyProxies.get());

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateManagerPropertiesLocked(GVariant * inProperties) noexcept
{
    GHashTable * const table = mConnManClient.mProperties.get();
    CHIP_ERROR retval        = CHIP_NO_ERROR;

    VerifyOrExit(inProperties != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(table, retval = CHIP_ERROR_INCORRECT_STATE);

    g_hash_table_remove(table, CONNMAN_MANAGER_PATH);

    retval = MergeObjectPropertiesLocked("manager properties (update)", table, CONNMAN_MANAGER_PATH, inProperties);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateServicesLocked(GVariant * inServices) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inServices != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mConnManClient.mServices, retval = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(g_variant_is_of_type(inServices, G_VARIANT_TYPE("a(oa{sv})")), retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_remove_all(mConnManClient.mServices.get());

    retval = MergeObjectPropertiesLocked("services (update)", mConnManClient.mServices.get(), inServices);
    SuccessOrExit(retval);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateServiceProxyLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                      const char * inPath,
                                                                                      ConnManService *& outService) noexcept
{
    ConnManService * proxy;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(mConnManClient.mServiceProxies, retval = CHIP_ERROR_INCORRECT_STATE);

    proxy = static_cast<ConnManService *>(g_hash_table_lookup(mConnManClient.mServiceProxies.get(), inPath));
    if (!proxy)
    {
        retval = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
            GAutoPtr<GError> err;
            ConnManService * service = conn_man_service_proxy_new_for_bus_sync(
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kConnManServiceName, inPath, nullptr, &err.GetReceiver());

            if (!service || err)
            {
                ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to create service proxy for %s: %s",
                             inPath, err ? err->message : "unknown");

                return CHIP_ERROR_INTERNAL;
            }
            else
            {
                ServiceRegisterPropertyChangedOnGLib(service);

                proxy = service;

                return CHIP_NO_ERROR;
            }
        });
        SuccessOrExit(retval);

        g_hash_table_insert(mConnManClient.mServiceProxies.get(), g_strdup(inPath), proxy);
    }

    outService = proxy;

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateTechnologiesLocked(GVariant * inTechnologies) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inTechnologies != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mConnManClient.mTechnologies, retval = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(g_variant_is_of_type(inTechnologies, G_VARIANT_TYPE("a(oa{sv})")), retval = CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_remove_all(mConnManClient.mTechnologies.get());

    retval = MergeObjectPropertiesLocked("technologies (update)", mConnManClient.mTechnologies.get(), inTechnologies);
    SuccessOrExit(retval);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateTechnologyProxyLocked(
    std::unique_lock<std::mutex> & inOutLock, const char * inPath, ConnManTechnology *& outTechnology) noexcept
{
    ConnManTechnology * proxy;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(mConnManClient.mTechnologyProxies, retval = CHIP_ERROR_INCORRECT_STATE);

    proxy = static_cast<ConnManTechnology *>(g_hash_table_lookup(mConnManClient.mTechnologyProxies.get(), inPath));
    if (!proxy)
    {
        retval = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
            GAutoPtr<GError> err;
            ConnManTechnology * technology = conn_man_technology_proxy_new_for_bus_sync(
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kConnManServiceName, inPath, nullptr, &err.GetReceiver());

            if (!technology || err)
            {
                ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to create technology proxy for %s: %s",
                             inPath, err ? err->message : "unknown");

                return CHIP_ERROR_INTERNAL;
            }
            else
            {
                TechnologyRegisterPropertyChangedOnGLib(technology);

                proxy = technology;

                return CHIP_NO_ERROR;
            }
        });
        SuccessOrExit(retval);

        g_hash_table_insert(mConnManClient.mTechnologyProxies.get(), g_strdup(inPath), proxy);
    }

    outTechnology = proxy;

exit:
    return retval;
}

// Worker Methods

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerRegisterAgent(const char * inPath) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    return ManagerRegisterAgentLocked(lock, inPath);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerRegisterAgentLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                        const char * inPath) noexcept
{
    GAutoPtr<char> path;
    GAutoPtr<GDBusConnection> connection;
    ConnManAgent * skeleton = nullptr;
    GAutoPtr<GError> exportError;
    CHIP_ERROR status;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(mConnManClient.mManagerProxy, retval = CHIP_ERROR_UNINITIALIZED);

    VerifyOrExit(!mConnManAgentServer.mExported, retval = CHIP_ERROR_ALREADY_INITIALIZED);

    path.reset(g_strdup(inPath));
    VerifyOrExit(path.get() != nullptr, retval = CHIP_ERROR_NO_MEMORY);

    // Hold a reference to the manager client proxy connection across
    // the unlock boundary.

    {
        GDBusConnection * c = g_dbus_proxy_get_connection(G_DBUS_PROXY(mConnManClient.mManagerProxy.get()));
        VerifyOrExit(c != nullptr, retval = CHIP_ERROR_INTERNAL);
        connection.reset(static_cast<GDBusConnection *>(g_object_ref(c)));
    }

    status = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        return InitAgentOnGLib(this, connection.get(), path.get(), skeleton, &exportError.GetReceiver());
    });
    ReturnErrorOnFailure(status);

    if (ChipError::IsSuccess(status) && skeleton == nullptr)
    {
        if (exportError.get() != nullptr)
        {
            ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "could not export agent server skeleton: %s",
                         exportError->message);
        }
        ExitNow(retval = CHIP_ERROR_INTERNAL);
    }

    // Commit exported objects under lock.
    mConnManAgentServer.mConnection.reset(static_cast<GDBusConnection *>(g_object_ref(connection.get())));
    mConnManAgentServer.mSkeleton.reset(skeleton);
    mConnManAgentServer.mExported = true;

    status = ManagerAgentOpLocked(inOutLock, path.get(), conn_man_manager_call_register_agent_sync, "register", true);
    if (!ChipError::IsSuccess(status))
    {
        ShutdownAgentLocked(inOutLock, inPath);

        retval = status;
    }

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerUnregisterAgent(const char * inPath) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    return ManagerUnregisterAgentLocked(lock, inPath);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerUnregisterAgentLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                          const char * inPath) noexcept
{
    GAutoPtr<char> path;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inPath != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    path.reset(g_strdup(inPath));
    VerifyOrExit(path.get() != nullptr, retval = CHIP_ERROR_NO_MEMORY);

    retval = ManagerAgentOpLocked(inOutLock, path.get(), conn_man_manager_call_unregister_agent_sync, "unregister", false);

    ShutdownAgentLocked(inOutLock, inPath);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ServiceConnectLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                  ConnManService * inService) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inService != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    // Initiate the asynchronous D-Bus call on the GLib context while *not* holding the class lock.
    // This is a synchronous cross-thread invoke to *schedule* the asynchronous operation; it does
    // not wait for the D-Bus operation to complete--service connect is inherently asynchronous.

    retval = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<ConnManService> service(static_cast<ConnManService *>(g_object_ref(inService)));
        VerifyOrReturnError(service.get() != nullptr, CHIP_ERROR_NO_MEMORY);

        conn_man_service_call_connect(
            service.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(
                +[](GObject * sourceObject_, GAsyncResult * res_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                    inSelf_->OnServiceConnectReady(sourceObject_, res_);
                }),
            this);

        return CHIP_NO_ERROR;
    });
    SuccessOrExit(retval);

exit:
    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::ServiceRegisterPropertyChangedOnGLib(ConnManService * inService) noexcept
{
    VerifyOrReturn(inService != nullptr);

    g_signal_connect(inService, "property-changed",
                     G_CALLBACK(+[](ConnManService * inService_, const char * inKey_, GVariant * inValue_,
                                    ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                         inSelf_->OnServicePropertyChanged(inService_, inKey_, inValue_);
                     }),
                     this);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::TechnologyRegisterPropertyChangedOnGLib(
    ConnManTechnology * inTechnology) noexcept
{
    VerifyOrReturn(inTechnology != nullptr);

    g_signal_connect(inTechnology, "property-changed",
                     G_CALLBACK(+[](ConnManTechnology * inTechnology_, const char * inKey_, GVariant * inValue_,
                                    ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                         inSelf_->OnTechnologyPropertyChanged(inTechnology_, inKey_, inValue_);
                     }),
                     this);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::TechnologyScanLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                  ConnManTechnology * inTechnology) noexcept
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrExit(inTechnology != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    // Initiate the asynchronous D-Bus call on the GLib context while *not* holding the class lock.
    // This is a synchronous cross-thread invoke to *schedule* the asynchronous operation; it does
    // not wait for the D-Bus operation to complete--technology scan is inherently asynchronous.

    retval = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<ConnManTechnology> technology(static_cast<ConnManTechnology *>(g_object_ref(inTechnology)));
        VerifyOrReturnError(technology.get() != nullptr, CHIP_ERROR_NO_MEMORY);

        conn_man_technology_call_scan(
            technology.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(
                +[](GObject * sourceObject_, GAsyncResult * res_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                    inSelf_->OnTechnologyScanReady(sourceObject_, res_);
                }),
            this);

        return CHIP_NO_ERROR;
    });
    SuccessOrExit(retval);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::TechnologySetPropertyLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                         ConnManTechnology * inTechnology,
                                                                                         const char * inKey,
                                                                                         GVariant * inValue) noexcept
{
    ConnManTechnology * technology = nullptr;
    const char * key               = nullptr;
    GVariant * value               = nullptr;
    CHIP_ERROR retval              = CHIP_NO_ERROR;

    VerifyOrExit(inTechnology != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inKey != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(inValue != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    technology = g_object_ref(inTechnology);

    key = g_strdup(inKey);
    VerifyOrExit(key != nullptr, retval = CHIP_ERROR_NO_MEMORY);

    value = g_variant_ref(inValue);

    retval = UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<ConnManTechnology> retained_technology(technology);
        GAutoPtr<char> retained_key(const_cast<char *>(key));
        GAutoPtr<GVariant> retained_value(value);
        GAutoPtr<GError> err;
        gboolean status;

        status = conn_man_technology_call_set_property_sync(retained_technology.get(), retained_key.get(), retained_value.get(),
                                                            nullptr, &err.GetReceiver());
        if (!status || err)
        {
            ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to set technology %s property %s: %s",
                         g_dbus_proxy_get_object_path(G_DBUS_PROXY(retained_technology.get())), retained_key.get(),
                         err ? err->message : "unknown");

            return CHIP_ERROR_INTERNAL;
        }
        else
        {
            return CHIP_NO_ERROR;
        }
    });
    SuccessOrExit(retval);

exit:
    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::TechnologySetPoweredLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                        ConnManTechnology * inTechnology,
                                                                                        const bool & inPowered) noexcept
{
    GAutoPtr<GVariant> value;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrExit(inTechnology != nullptr, retval = CHIP_ERROR_INVALID_ARGUMENT);

    value.reset(g_variant_new_boolean(inPowered));
    VerifyOrExit(value.get() != nullptr, retval = CHIP_ERROR_NO_MEMORY);

    retval = TechnologySetPropertyLocked(inOutLock, inTechnology, kConnManTechnologyPropertyPoweredKey, value.get());
    SuccessOrExit(retval);

exit:
    return retval;
}

// D-Bus / glib Asynchronous Completion Methods

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerReady(GObject * inObject, GAsyncResult * inResult)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManManager * manager;
    GAutoPtr<GError> err;

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting to manager");

    VerifyOrReturn(inObject != nullptr);
    VerifyOrReturn(inResult != nullptr);

    // When creating D-Bus manager object, the thread default context
    // must be initialized. Otherwise, all D-Bus signals will be
    // delivered to the GLib global default main context.

    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    manager = conn_man_manager_proxy_new_for_bus_finish(inResult, &err.GetReceiver());

    if (manager && err.get() == nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connected to manager");

        mConnManClient.mManagerProxy.reset(manager);

        // Establish manager signal handlers.

        // Property Changed

        g_signal_connect(mConnManClient.mManagerProxy.get(), "property-changed",
                         G_CALLBACK(+[](ConnManManager * inManager_, const char * inKey, GVariant * inValue,
                                        ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                             inSelf_->OnManagerPropertyChanged(inManager_, inKey, inValue);
                         }),
                         this);

        // Technology Added

        g_signal_connect(mConnManClient.mManagerProxy.get(), "technology-added",
                         G_CALLBACK(+[](ConnManManager * inManager_, const char * inPath, GVariant * inProperties,
                                        ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                             inSelf_->OnManagerTechnologyAdded(inManager_, inPath, inProperties);
                         }),
                         this);

        // Technology Removed

        g_signal_connect(mConnManClient.mManagerProxy.get(), "technology-removed",
                         G_CALLBACK(+[](ConnManManager * inManager_, const char * inPath,
                                        ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                             inSelf_->OnManagerTechnologyRemoved(inManager_, inPath);
                         }),
                         this);

        // Services Changed

        g_signal_connect(
            mConnManClient.mManagerProxy.get(), "services-changed",
            G_CALLBACK(+[](ConnManManager * inManager_, GVariant * inServicesChanged, const char * const * inServicesRemoved,
                           ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                inSelf_->OnManagerServicesChanged(inManager_, inServicesChanged, inServicesRemoved);
            }),
            this);

        // Prime the manager properties, services, and technologies.

        mConnManClient.mProperties.reset(
            g_hash_table_new_full(g_str_hash, g_str_equal, g_free, reinterpret_cast<GDestroyNotify>(g_variant_unref)));

        conn_man_manager_call_get_properties(
            mConnManClient.mManagerProxy.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(
                +[](GObject * sourceObject_, GAsyncResult * res_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                    inSelf_->OnManagerGetPropertiesReady(sourceObject_, res_);
                }),
            this);

        mConnManClient.mServiceProxies.reset(g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref));
        mConnManClient.mServices.reset(
            g_hash_table_new_full(g_str_hash, g_str_equal, g_free, reinterpret_cast<GDestroyNotify>(g_variant_unref)));

        conn_man_manager_call_get_services(
            mConnManClient.mManagerProxy.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(
                +[](GObject * sourceObject_, GAsyncResult * res_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                    inSelf_->OnManagerGetServicesReady(sourceObject_, res_);
                }),
            this);

        mConnManClient.mTechnologyProxies.reset(g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref));
        mConnManClient.mTechnologies.reset(
            g_hash_table_new_full(g_str_hash, g_str_equal, g_free, reinterpret_cast<GDestroyNotify>(g_variant_unref)));

        conn_man_manager_call_get_technologies(
            mConnManClient.mManagerProxy.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(
                +[](GObject * sourceObject_, GAsyncResult * res_, ConnectivityManagerImpl_NetworkManagementConnMan * inSelf_) {
                    inSelf_->OnManagerGetTechnologiesReady(sourceObject_, res_);
                }),
            this);
    }
    else
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to create manager: %s",
                        err ? err->message : "unknown error");
    }
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerGetObjectsReady(
    GObject * inObject, GAsyncResult * inResult, ManagerGetObjectsFinishFunc inManagerGetObjectsFinishFunc,
    HandleManagerGetObjectsMethod inHandleManagerGetObjectsMethod) noexcept
{
    GAutoPtr<GVariant> objects_pointer;
    GAutoPtr<GError> err_pointer;
    GObject * object   = nullptr;
    GVariant * objects = nullptr;
    GError * err       = nullptr;

    VerifyOrReturn(inObject != nullptr);
    VerifyOrReturn(inResult != nullptr);
    VerifyOrReturn(inManagerGetObjectsFinishFunc != nullptr);
    VerifyOrReturn(inHandleManagerGetObjectsMethod != nullptr);

    // Complete the asynchronous objects "big get" with the lock and
    // then release it.

    {
        std::lock_guard<std::mutex> lock(mConnManMutex);

        inManagerGetObjectsFinishFunc(CONN_MAN_MANAGER(inObject), &objects_pointer.GetReceiver(), inResult,
                                      &err_pointer.GetReceiver());
    }

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.
    //
    // IMPORTANT: GAutoPtr is not safely copyable to another thread /
    //            deferred context by capture.  We explicitly retain
    //            (ref/copy) the underlying GLib objects and then
    //            release them in the lambda.

    object = g_object_ref(inObject);

    if (objects_pointer.get() != nullptr)
    {
        // Retained: released in lambda

        objects = g_variant_ref(objects_pointer.get());
    }

    if (err_pointer.get() != nullptr)
    {
        // Deep Copy: released in lambda

        err = g_error_copy(err_pointer.get());
    }

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
        [this, object, objects, err, inHandleManagerGetObjectsMethod]() -> void {
            GAutoPtr<GObject> retained_object(object);
            GAutoPtr<GVariant> retained_objects(objects);
            GAutoPtr<GError> retained_err(err);

            (this->*inHandleManagerGetObjectsMethod)(CONN_MAN_MANAGER(retained_object.get()), retained_objects.get(),
                                                     retained_err.get());
        });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerGetPropertiesReady(GObject * inObject, GAsyncResult * inResult)
{
    OnManagerGetObjectsReady(inObject, inResult, conn_man_manager_call_get_properties_finish,
                             &ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetProperties);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerGetServicesReady(GObject * inObject, GAsyncResult * inResult)
{
    OnManagerGetObjectsReady(inObject, inResult, conn_man_manager_call_get_services_finish,
                             &ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetServices);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerGetTechnologiesReady(GObject * inObject, GAsyncResult * inResult)
{
    OnManagerGetObjectsReady(inObject, inResult, conn_man_manager_call_get_technologies_finish,
                             &ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetTechnologies);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnObjectActionReady(
    GObject * inObject, GAsyncResult * inResult, ObjectActionFinishFunc inObjectActionFinishFunc,
    HandleObjectActionCompleteMethod inHandleObjectActionCompleteMethod) noexcept
{
    GAutoPtr<GError> err_pointer;
    GObject * object = nullptr;
    GError * err     = nullptr;

    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    VerifyOrReturn(inObject != nullptr);
    VerifyOrReturn(inResult != nullptr);
    VerifyOrReturn(inObjectActionFinishFunc != nullptr);
    VerifyOrReturn(inHandleObjectActionCompleteMethod != nullptr);

    // Complete the asynchronous objects "big get" with the lock and
    // then release it.

    {
        std::lock_guard<std::mutex> lock(mConnManMutex);

        inObjectActionFinishFunc(inObject, inResult, &err_pointer.GetReceiver());
    }

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.
    //
    // IMPORTANT: GAutoPtr is not safely copyable to another thread /
    //            deferred context by capture.  We explicitly retain
    //            (ref/copy) the underlying GLib object and then
    //            release it in the lambda.

    // Retained: released in lambda

    object = g_object_ref(inObject);

    if (err_pointer.get() != nullptr)
    {
        // Deep Copy: released in lambda

        err = g_error_copy(err_pointer.get());
    }

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
        [this, object, err, inHandleObjectActionCompleteMethod]() -> void {
            GAutoPtr<GObject> retained_object(object);
            GAutoPtr<GError> retained_err(err);

            (this->*inHandleObjectActionCompleteMethod)(G_DBUS_PROXY(retained_object.get()), retained_err.get());
        });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnServiceConnectReady(GObject * inObject, GAsyncResult * inResult)
{
    OnObjectActionReady(inObject, inResult, reinterpret_cast<ObjectActionFinishFunc>(conn_man_service_call_connect_finish),
                        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleServiceConnectComplete);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnTechnologyScanReady(GObject * inObject, GAsyncResult * inResult)
{
    OnObjectActionReady(inObject, inResult, reinterpret_cast<ObjectActionFinishFunc>(conn_man_technology_call_scan_finish),
                        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyScanComplete);
}

// D-Bus / glib Signal Callback Methods

gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentCancel(ConnManAgent * inAgent,
                                                                         GDBusMethodInvocation * inInvocation) noexcept
{
    const gboolean retval = TRUE;

    nlEXPECT(inAgent != nullptr, exit);
    nlEXPECT(inInvocation != nullptr, exit);

    conn_man_agent_complete_cancel(inAgent, inInvocation);

exit:
    return retval;
}

gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentRelease(ConnManAgent * inAgent,
                                                                          GDBusMethodInvocation * inInvocation) noexcept
{
    const gboolean retval = TRUE;

    nlEXPECT(inAgent != nullptr, exit);
    nlEXPECT(inInvocation != nullptr, exit);

    conn_man_agent_complete_release(inAgent, inInvocation);

exit:
    return retval;
}

gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentRequestInput(ConnManAgent * inAgent,
                                                                               GDBusMethodInvocation * inInvocation,
                                                                               const gchar * inPath,
                                                                               GVariant * inProperties) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    const GQuark domain = G_IO_ERROR;
    Optional<gint> code;
    const gchar * message     = nullptr;
    const char * pending_path = nullptr;
    bool paths_match;
    bool want_passphrase = false;
    GVariantIter iter;
    const char * key = nullptr;
    GVariant * boxed = nullptr;
    GVariantBuilder builder;
    GVariant * reply;
    const gboolean retval = TRUE;

    VerifyOrExit(inAgent != nullptr, code.SetValue(G_IO_ERROR_INVALID_ARGUMENT); message = "inAgent == nullptr");
    VerifyOrExit(inInvocation != nullptr, code.SetValue(G_IO_ERROR_INVALID_ARGUMENT); message = "inInvocation == nullptr");
    VerifyOrExit(inPath != nullptr, code.SetValue(G_IO_ERROR_INVALID_ARGUMENT); message = "inPath == nullptr");
    VerifyOrExit(inProperties != nullptr, code.SetValue(G_IO_ERROR_INVALID_ARGUMENT); message = "inProperties == nullptr");

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "agent service %s request input", inPath);

    // First, ensure there is a pending service we are currently connecting.

    VerifyOrExit(mConnManAgentServer.mPendingService, code.SetValue(G_IO_ERROR_FAILED);
                 message = "No pending service for RequestInput");

    // Second, ensure the pending service matches the one requesting input.

    pending_path = g_dbus_proxy_get_object_path(G_DBUS_PROXY(mConnManAgentServer.mPendingService.get()));
    VerifyOrExit(pending_path != nullptr, code.SetValue(G_IO_ERROR_FAILED); message = "No path for pending service");

    paths_match = (strcmp(pending_path, inPath) == 0);
    VerifyOrExit(paths_match, code.SetValue(G_IO_ERROR_FAILED); message = "Input requested for unexpected service");

    // Third, validate the request type.

    VerifyOrExit(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), code.SetValue(G_IO_ERROR_INVALID_ARGUMENT);
                 message = "properties are not 'a{sv}' type");

    // Fourth, iterate over the requested property keys and decide
    // what we can satisfy relative to what is requested.

    g_variant_iter_init(&iter, inProperties);
    while (g_variant_iter_next(&iter, "{&s@v}", &key, &boxed))
    {
        UnboxedVariant variant(boxed);
        g_variant_unref(boxed);
        boxed = nullptr;

        if (!variant || !g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_VARDICT))
            continue;

        if (strcmp(key, kConnManServiceAgentPropertyPassphraseKey) == 0)
        {
            ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: wants %s", __func__, key);

            // Optional: check "Type" == "passphrase" and/or Requirement.
            want_passphrase = true;
        }
    }

    // Fifth and finally, build the reply with only the requested keys.

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    if (want_passphrase)
    {
        VerifyOrExit(!mWiFiConnectPassphrase.empty(), code.SetValue(G_IO_ERROR_FAILED);
                     message = "Passphrase requested but not available");

        char passphrase[Internal::kMaxWiFiKeyLength + 1];
        const size_t n = std::min<size_t>(mWiFiConnectPassphrase.size(), Internal::kMaxWiFiKeyLength);
        memcpy(passphrase, mWiFiConnectPassphrase.data(), n);
        passphrase[n] = '\0';

        g_variant_builder_add(&builder, "{sv}", kConnManServiceAgentPropertyPassphraseKey, g_variant_new_string(passphrase));
    }

    reply = g_variant_builder_end(&builder);
    VerifyOrExit(reply != nullptr, code.SetValue(G_IO_ERROR_FAILED); message = "Could not finish building reply");

    conn_man_agent_complete_request_input(inAgent, inInvocation, reply);

exit:
    if (inInvocation != nullptr && code.HasValue())
    {
        if (message == nullptr)
            message = "Internal error";

        g_dbus_method_invocation_return_error(inInvocation, domain, code.Value(), "%s", message);
    }

    if (!mWiFiConnectPassphrase.empty())
    {
        Crypto::DRBG_get_bytes(mWiFiConnectPassphrase.data(), mWiFiConnectPassphrase.capacity());

        mWiFiConnectPassphrase.clear();
    }

    return retval;
}

gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentReportError(ConnManAgent * inAgent,
                                                                              GDBusMethodInvocation * inInvocation,
                                                                              const gchar * inPath, const gchar * inError) noexcept
{
    const gboolean retval = TRUE;

    nlEXPECT(inAgent != nullptr, exit);
    nlEXPECT(inInvocation != nullptr, exit);

    ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "service %s had error %s", inPath ? inPath : "(null)",
                 inError ? inError : "(null)");

    conn_man_agent_complete_report_error(inAgent, inInvocation);

exit:
    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerPropertyChanged(ConnManManager * inManager, const char * inKey,
                                                                                GVariant * inValue)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManManager * manager = nullptr;
    const char * key         = nullptr;
    GVariant * value         = nullptr;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inKey != nullptr);
    VerifyOrReturn(inValue != nullptr);

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.

    manager = g_object_ref(inManager);

    key = g_strdup(inKey);
    VerifyOrReturn(key != nullptr);

    value = g_variant_ref(inValue);

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, manager, key, value]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<char> retained_key(const_cast<char *>(key));
        GAutoPtr<GVariant> retained_value(value);

        HandleManagerPropertyChanged(retained_manager.get(), retained_key.get(), retained_value.get());
    });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerServicesChanged(ConnManManager * inManager,
                                                                                GVariant * inServicesChanged,
                                                                                const char * const * inServicesRemoved)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManManager * manager = nullptr;
    GVariant * changed       = nullptr;
    gchar ** removed         = nullptr;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inServicesChanged != nullptr);

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.

    manager = g_object_ref(inManager);
    changed = g_variant_ref(inServicesChanged);

    if (inServicesRemoved != nullptr)
    {
        removed = g_strdupv(const_cast<gchar **>(inServicesRemoved));
        VerifyOrReturn(removed != nullptr);
    }

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, manager, changed, removed]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<GVariant> retained_changed(changed);
        GAutoPtr<gchar *> retained_removed(removed);

        HandleManagerServicesChanged(retained_manager.get(), retained_changed.get(), retained_removed.get());
    });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerTechnologyAdded(ConnManManager * inManager, const char * inPath,
                                                                                GVariant * inProperties)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManManager * manager = nullptr;
    char * path              = nullptr;
    GVariant * properties    = nullptr;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inPath != nullptr);
    VerifyOrReturn(inProperties != nullptr);

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.

    manager = g_object_ref(inManager);

    path = g_strdup(inPath);
    VerifyOrReturn(path != nullptr);

    properties = g_variant_ref(inProperties);

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, manager, path, properties]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<char> retained_path(const_cast<char *>(path));
        GAutoPtr<GVariant> retained_properties(properties);

        HandleManagerTechnologyAdded(retained_manager.get(), retained_path.get(), retained_properties.get());
    });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnManagerTechnologyRemoved(ConnManManager * inManager, const char * inPath)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManManager * manager = nullptr;
    const char * path        = nullptr;

    VerifyOrReturn(inManager != nullptr);
    VerifyOrReturn(inPath != nullptr);

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.

    manager = g_object_ref(inManager);

    path = g_strdup(inPath);
    VerifyOrReturn(path != nullptr);

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, manager, path]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<char> retained_path(const_cast<char *>(path));

        HandleManagerTechnologyRemoved(retained_manager.get(), retained_path.get());
    });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnServicePropertyChanged(ConnManService * inService, const char * inKey,
                                                                                GVariant * inValue)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManService * service = nullptr;
    const char * key         = nullptr;
    GVariant * value         = nullptr;

    VerifyOrReturn(inService != nullptr);
    VerifyOrReturn(inKey != nullptr);
    VerifyOrReturn(inValue != nullptr);

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.

    service = g_object_ref(inService);

    key = g_strdup(inKey);
    VerifyOrReturn(key != nullptr);

    value = g_variant_ref(inValue);

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, service, key, value]() -> void {
        GAutoPtr<ConnManService> retained_service(service);
        GAutoPtr<char> retained_key(const_cast<char *>(key));
        GAutoPtr<GVariant> retained_value(value);

        HandleServicePropertyChanged(retained_service.get(), retained_key.get(), retained_value.get());
    });
}

void ConnectivityManagerImpl_NetworkManagementConnMan::OnTechnologyPropertyChanged(ConnManTechnology * inTechnology,
                                                                                   const char * inKey, GVariant * inValue)
{
    std::lock_guard<std::mutex> lock(mConnManMutex);
    ConnManTechnology * technology = nullptr;
    const char * key               = nullptr;
    GVariant * value               = nullptr;

    VerifyOrReturn(inTechnology != nullptr);
    VerifyOrReturn(inKey != nullptr);
    VerifyOrReturn(inValue != nullptr);

    // Defer the actual work to the main thread, outside the lock, via
    // a lambda. The lock will be reacquired in the method called by
    // the lambda.

    technology = g_object_ref(inTechnology);

    key = g_strdup(inKey);
    VerifyOrReturn(key != nullptr);

    value = g_variant_ref(inValue);

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, technology, key, value]() -> void {
        GAutoPtr<ConnManTechnology> retained_technology(technology);
        GAutoPtr<char> retained_key(const_cast<char *>(key));
        GAutoPtr<GVariant> retained_value(value);

        HandleTechnologyPropertyChanged(retained_technology.get(), retained_key.get(), retained_value.get());
    });
}
#endif // CHIP_LINUX_NETWORK_MANAGER_CONNMAN

} // namespace DeviceLayer
} // namespace chip
