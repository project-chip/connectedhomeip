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
#include <array>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include <errno.h>

#include <lib/support/CodeUtils.h>
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

// clang-format off
#define CONNMAN_SERVICE                                "net.connman"

#define CONNMAN_AGENT_INTERFACE                        CONNMAN_SERVICE ".Agent"

#define CONNMAN_MANAGER_INTERFACE                      CONNMAN_SERVICE ".Manager"
#define CONNMAN_MANAGER_PATH                           "/"
#define CONNMAN_MANAGER_OBJECT_PATH_STEM               CONNMAN_MANAGER_PATH "net/connman"
#define CONNMAN_MANAGER_SERVICE_PATH_STEM              CONNMAN_MANAGER_OBJECT_PATH_STEM "/service"
#define CONNMAN_MANAGER_TECHNOLOGY_PATH_STEM           CONNMAN_MANAGER_OBJECT_PATH_STEM "/technology"

#define CONNMAN_SERVICE_INTERFACE                      CONNMAN_SERVICE ".Service"
#define CONNMAN_TECHNOLOGY_INTERFACE                   CONNMAN_SERVICE ".Technology"

// Note that dashes ("-") are not allowed in D-Bus
// paths.

#define MATTER_PATH                                    "/org/csa/matter"
#define MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH  MATTER_PATH "/ConnectivityManager"

#define kConnManObjectPropertyNameKey                  "Name"

#define kConnManObjectPropertyStateKey                 "State"

#define kConnManObjectPropertyStateIdleValue           "idle"
#define kConnManObjectPropertyStateOfflineValue        "offline"
#define kConnManObjectPropertyStateOnlineValue         "online"
#define kConnManObjectPropertyStateReadyValue          "ready"

#define kConnManObjectPropertyTypeKey                  "Type"

#define kConnManObjectPropertyTypeEthernetValue        "ethernet"
#define kConnManObjectPropertyTypeWiFiValue            "wifi"

#define CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX        "connman: "
// clang-format on

namespace chip {
namespace DeviceLayer {

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

    GVariant * get() const noexcept;
    explicit operator bool() const noexcept;

private:
    GAutoPtr<GVariant> mOwned;   // owns unboxed value if we had to unbox
    GVariant * mValue = nullptr; // points to either inMaybeVariant or mOwned.get()
};

struct NetworkServiceTypeDescriptor
{
    const char * mType;
    const char * mDescription;
};

struct ClientErrorTableEntry
{
    const char * mName;
    int mErrno;
};

using ServiceError = Platform::Linux::Detail::ConnManServiceError;

struct ServiceErrorTableEntry
{
    const char * mName;
    ServiceError mError;
};

// Global Variables

// clang-format off
static constexpr char kConnManServiceName[]                                       = CONNMAN_SERVICE;
static constexpr char kConnManManagerInterface[]                                  = CONNMAN_MANAGER_INTERFACE;
static constexpr char kConnManManagerPath[]                                       = CONNMAN_MANAGER_PATH;

// Service Agent Object Property Keys and Values

static constexpr char kConnManServiceAgentPropertyAlternatesKey[]                 = "Alternates";
static constexpr char kConnManServiceAgentPropertyIdentityKey[]                   = "Identity";
static constexpr char kConnManServiceAgentPropertyNameKey[]                       = "Name";
static constexpr char kConnManServiceAgentPropertyPassphraseKey[]                 = "Passphrase";
static constexpr char kConnManServiceAgentPropertyPasswordKey[]                   = "Password";
static constexpr char kConnManServiceAgentPropertyPreviousPassphraseKey[]         = "PreviousPassphrase";

static constexpr char kConnManServiceAgentPropertyRequirementKey[] = "Requirement";

static constexpr char kConnManServiceAgentPropertyRequirementAlternateValue[]     = "alternate";
static constexpr char kConnManServiceAgentPropertyRequirementInformationalValue[] = "informational";
static constexpr char kConnManServiceAgentPropertyRequirementMandatoryValue[]     = "mandatory";
static constexpr char kConnManServiceAgentPropertyRequirementOptionalValue[]      = "optional";

static constexpr char kConnManServiceAgentPropertySSIDKey[]                       = "SSID";

static constexpr char kConnManServiceAgentPropertyTypeKey[]                       = "Type";

static constexpr char kConnManServiceAgentPropertyTypePassphraseValue[]           = "passphrase";
static constexpr char kConnManServiceAgentPropertyTypePSKValue[]                  = "psk";
static constexpr char kConnManServiceAgentPropertyTypeResponseValue[]             = "response";
static constexpr char kConnManServiceAgentPropertyTypeSSIDValue[]                 = "ssid";
static constexpr char kConnManServiceAgentPropertyTypeStringValue[]               = "string";
static constexpr char kConnManServiceAgentPropertyTypeWEPValue[]                  = "wep";
static constexpr char kConnManServiceAgentPropertyTypeWPSPINValue[]               = "wpspin";

static constexpr char kConnManServiceAgentPropertyUsernameKey[]                   = "Username";
static constexpr char kConnManServiceAgentPropertyValueKey[]                      = "Value";
static constexpr char kConnManServiceAgentPropertyWPSKey[]                        = "WPS";

// Manager Object Property Keys and Values

static constexpr char kConnManManagerPropertyStateKey[]                           = kConnManObjectPropertyStateKey;

static constexpr char kConnManManagerPropertyStateIdleValue[]                     = kConnManObjectPropertyStateIdleValue;
static constexpr char kConnManManagerPropertyStateOfflineValue[]                  = kConnManObjectPropertyStateOfflineValue;
static constexpr char kConnManManagerPropertyStateOnlineValue[]                   = kConnManObjectPropertyStateOnlineValue;
static constexpr char kConnManManagerPropertyStateReadyValue[]                    = kConnManObjectPropertyStateReadyValue;
static constexpr char kConnManManagerPropertyStateUnavailableValue[]              = "unavailable";

// Service Object Property Keys and Values

static constexpr char kConnManServicePropertyAutoConnectKey[]                     = "AutoConnect";
static constexpr char kConnManServicePropertyDomainsKey[]                         = "Domains";
static constexpr char kConnManServicePropertyDomainsConfigurationKey[]            = "Domains.Configuration";

static constexpr char kConnManServicePropertyErrorKey[]                           = "Error";

static constexpr char kConnManServicePropertyErrorAuthFailedValue[]               = "auth-failed";
static constexpr char kConnManServicePropertyErrorBlockedValue[]                  = "blocked";
static constexpr char kConnManServicePropertyErrorConnectFailedValue[]            = "connect-failed";
static constexpr char kConnManServicePropertyErrorDHCPFailedValue[]               = "dhcp-failed";
static constexpr char kConnManServicePropertyErrorInvalidKeyValue[]               = "invalid-key";
static constexpr char kConnManServicePropertyErrorLoginFailedValue[]              = "login-failed";
static constexpr char kConnManServicePropertyErrorOnlineCheckFailedValue[]        = "online-check-failed";
static constexpr char kConnManServicePropertyErrorOutOfRangeValue[]               = "out-of-range";
static constexpr char kConnManServicePropertyErrorPINMissingValue[]               = "pin-missing";

static constexpr char kConnManServicePropertyEthernetKey[]                        = "Ethernet";

static constexpr char kConnManServicePropertyEthernetAddressKey[]                 = "Address";
static constexpr char kConnManServicePropertyEthernetInterfaceKey[]               = "Interface";
static constexpr char kConnManServicePropertyEthernetMTUKey[]                     = "MTU";
static constexpr char kConnManServicePropertyEthernetMethodKey[]                  = "Method";

static constexpr char kConnManServicePropertyFavoriteKey[]                        = "Favorite";
static constexpr char kConnManServicePropertyIPv4Key[]                            = "IPv4";
static constexpr char kConnManServicePropertyIPv4ConfigurationKey[]               = "IPv4.Configuration";
static constexpr char kConnManServicePropertyIPv6Key[]                            = "IPv6";
static constexpr char kConnManServicePropertyIPv6ConfigurationKey[]               = "IPv6.Configuration";

static constexpr char kConnManServicePropertyIPAddressKey[]                       = "Address";

static constexpr char kConnManServicePropertymDNSKey[]                            = "mDNS";
static constexpr char kConnManServicePropertymDNSConfigurationKey[]               = "mDNS.Configuration";
static constexpr char kConnManServicePropertyNameKey[]                            = kConnManObjectPropertyNameKey;
static constexpr char kConnManServicePropertyNameserversKey[]                     = "Nameservers";
static constexpr char kConnManServicePropertyNameserversConfigurationKey[]        = "Nameservers.Configuration";
static constexpr char kConnManServicePropertyProviderKey[]                        = "Provider";
static constexpr char kConnManServicePropertyProxyConfigurationKey[]              = "Proxy.Configuration";
static constexpr char kConnManServicePropertyProxyKey[]                           = "Proxy";

static constexpr char kConnManServicePropertyRoamingKey[]                         = "Roaming";

static constexpr char kConnManServicePropertySecurityKey[]                        = "Security";

static constexpr char kConnManServicePropertySecurity8021XValue[]                 = "ieee8021x";
static constexpr char kConnManServicePropertySecurityNoneValue[]                  = "none";
static constexpr char kConnManServicePropertySecurityPSKValue[]                   = "psk";
static constexpr char kConnManServicePropertySecurityRSNValue[]                   = "rsn";
static constexpr char kConnManServicePropertySecurityWEPValue[]                   = "wep";
static constexpr char kConnManServicePropertySecurityWPAValue[]                   = "wpa";
static constexpr char kConnManServicePropertySecurityWPSAdvertisingValue[]        = "wps_advertising";
static constexpr char kConnManServicePropertySecurityWPSValue[]                   = "wps";

static constexpr char kConnManServicePropertyStateKey[]                           = "State";

static constexpr char kConnManServicePropertyStateAssociationValue[]              = "association";
static constexpr char kConnManServicePropertyStateConfigurationValue[]            = "configuration";
static constexpr char kConnManServicePropertyStateDisconnectValue[]               = "disconnect";
static constexpr char kConnManServicePropertyStateFailureValue[]                  = "failure";
static constexpr char kConnManServicePropertyStateIdleValue[]                     = kConnManObjectPropertyStateIdleValue;
static constexpr char kConnManServicePropertyStateOfflineValue[]                  = kConnManObjectPropertyStateOfflineValue;
static constexpr char kConnManServicePropertyStateOnlineValue[]                   = kConnManObjectPropertyStateOnlineValue;
static constexpr char kConnManServicePropertyStateReadyValue[]                    = kConnManObjectPropertyStateReadyValue;

static constexpr char kConnManServicePropertyStrengthKey[]                        = "Strength";
static constexpr char kConnManServicePropertyTimeserversKey[]                     = "Timeservers";
static constexpr char kConnManServicePropertyTimeserversConfigurationKey[]        = "Timeservers.Configuration";

static constexpr char kConnManServicePropertyTypeKey[]                            = kConnManObjectPropertyTypeKey;

static constexpr char kConnManServicePropertyTypeEthernetValue[]                  = kConnManObjectPropertyTypeEthernetValue;
static constexpr char kConnManServicePropertyTypeWiFiValue[]                      = kConnManObjectPropertyTypeWiFiValue;

// Technology Object Property Keys and Values

static constexpr char kConnManTechnologyPropertyConnectedKey[]                    = "Connected";
static constexpr char kConnManTechnologyPropertyNameKey[]                         = kConnManObjectPropertyNameKey;
static constexpr char kConnManTechnologyPropertyPoweredKey[]                      = "Powered";
static constexpr char kConnManTechnologyPropertyTypeKey[]                         = kConnManObjectPropertyTypeKey;

static constexpr char kConnManTechnologyPropertyTypeEthernetValue[]               = kConnManObjectPropertyTypeEthernetValue;
static constexpr char kConnManTechnologyPropertyTypeWiFiValue[]                   = kConnManObjectPropertyTypeWiFiValue;

// Network Name and Passphrase Validate Check Failure Reason Strings

static constexpr char kNetworkNameIsTooShort[]                                    = "name is too short";
static constexpr char kNetworkNameIsTooLong[]                                     = "name is too long";
static constexpr char kNetworkPassphraseIsTooShort[]                              = "passphrase is too short";
static constexpr char kNetworkPassphraseIsTooLong[]                               = "passphrase is too long";
static constexpr char kNetworkPassphraseHasInvalidCharacters[]                    = "passphrase contains an invalid character";
// clang-format on

/**
 *  The maximum number of "active" (connman does not actually do
 *  active, directed scans) scans for a particular SSID before
 *  failing.
 *
 *  'chip-tool' and most app-based commissioners will timeout with
 *  anything longer than this.
 */
static constexpr size_t kWiFiActiveScanLimit = 2;

/**
 *  The maximum number of scans for connecting to a particular SSID
 *  before failing.
 *
 *  'chip-tool' and most app-based commissioners will timeout with
 *  anything longer than this.
 */
static constexpr size_t kWiFiConnectScanLimit = 2;

/**
 *  Trailing-edge debounce interval applied between an observed change
 *  in a tracked network service's IPv4/IPv6 configuration and the
 *  posting of a kInternetConnectivityChange device event, so that
 *  connman property storms at interface bring-up coalesce into a
 *  single event.
 */
static constexpr System::Clock::Milliseconds32 kNetworkServiceConnectivityDebounce = System::Clock::Milliseconds32(1000);

static constexpr std::array sNetworkServiceTypeDescriptors = {
    NetworkServiceTypeDescriptor
#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    { kConnManObjectPropertyTypeEthernetValue, "Ethernet" },
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    NetworkServiceTypeDescriptor{ kConnManObjectPropertyTypeWiFiValue, "Wi-Fi" },
#endif
};

/**
 *  Maps connman's client D-Bus error name space
 *  (`net.connman.Error.*` and `net.connman.Agent.Error.*`) onto POSIX
 *  errno values, carried in CHIP_ERROR's kPOSIX range.
 *
 *  These are the *synchronous* errors returned from method calls on
 *  the connman manager, service, and technology interfaces --
 *  distinct from the closed vocabulary of the service "Error"
 *  *property*, which describes an asynchronous connection failure and
 *  is a separate concern.
 *
 *  @sa MapClientError
 */
// clang-format off
static constexpr std::array<ClientErrorTableEntry, 25> kClientErrorTable =
{{
    { "net.connman.Error.AlreadyConnected",    EISCONN      },
    { "net.connman.Error.AlreadyDisabled",     EALREADY     },
    { "net.connman.Error.AlreadyEnabled",      EALREADY     },
    { "net.connman.Error.AlreadyExists",       EEXIST       },
    { "net.connman.Error.Canceled",            ECANCELED    },
    { "net.connman.Error.InProgress",          EINPROGRESS  },
    { "net.connman.Error.InvalidArguments",    EINVAL       },
    { "net.connman.Error.InvalidProperty",     EINVAL       },
    { "net.connman.Error.InvalidService",      EINVAL       },
    { "net.connman.Error.NoCarrier",           ENOLINK      },
    { "net.connman.Error.NotConnected",        ENOTCONN     },
    { "net.connman.Error.NotFound",            ENOENT       },
    { "net.connman.Error.NotImplemented",      ENOSYS       },
    { "net.connman.Error.NotRegistered",       ESRCH        },
    { "net.connman.Error.NotSupported",        EOPNOTSUPP   },
    { "net.connman.Error.NotUnique",           ENOTUNIQ     },
    { "net.connman.Error.OperationAborted",    ECONNABORTED },
    { "net.connman.Error.OperationCanceled",   ECANCELED    },
    { "net.connman.Error.OperationTimeout",    ETIMEDOUT    },
    { "net.connman.Error.PassphraseRequired",  ENOKEY       },
    { "net.connman.Error.PermissionDenied",    EACCES       },
    { "net.connman.Agent.Error.Canceled",      ECANCELED    },
    { "net.connman.Agent.Error.Rejected",      ECONNREFUSED },
    { "net.connman.Agent.Error.Retry",         EAGAIN       }

    // net.connman.Error.Failed is deliberately absent: it is
    // connman's catch-all and carries no information beyond the caller's
    // default.
}};
// clang-format on

/**
 *  connman surfaces neither the IEEE 802.11 status code nor the
 *  disconnect reason code from wpa_supplicant anywhere on D-Bus (it
 *  collapses both into the closed "Error" vocabulary above). The
 *  wpa_supplicant-backed peer implementation reads them directly from
 *  the supplicant D-Bus interface; we have no equivalent here. Rather
 *  than fabricate a specific code we do not have, report the generic
 *  one and let the `AssociationFailureCauseEnum` carry what signal we
 *  do possess.
 */
static constexpr uint16_t kWlanStatusSuccess            = 0;
static constexpr uint16_t kWlanStatusUnspecifiedFailure = 1;

// clang-format off
static constexpr ServiceError kServiceErrorNone =
{
    Status::kSuccess,
    AssociationFailureCauseEnum::kUnknown,
    kWlanStatusSuccess,
    false,
    false
};

static constexpr ServiceError kServiceErrorUnknown =
{
    Status::kUnknownError,
    AssociationFailureCauseEnum::kUnknown,
    kWlanStatusUnspecifiedFailure,
    true,
    true
};

static constexpr std::array<ServiceErrorTableEntry, 9> kServiceErrorTable =
{{
    /**
     *  The WPA 4-way handshake rejected the pre-shared key: the
     *  passphrase is wrong. This is the canonical Wi-Fi commissioning
     *  failure and the one a commissioner most needs rendered
     *  faithfully, since it is the only one an installer can act on
     *  directly. Association succeeded; authentication did not.
     *  Terminal, and a true association failure.
     */
    { "invalid-key", {
            Status::kAuthFailure,
            AssociationFailureCauseEnum::kAuthenticationFailed,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    },

    /**
     *  IEEE 802.11 authentication failed ahead of, or independently
     *  of, the 4-way handshake. connman does not distinguish this
     *  from "invalid-key" in any way Matter can exploit, so it maps
     *  identically; the distinction survives only in the log.
     *  Terminal, and a true association failure.
     */
    { "auth-failed", {
            Status::kAuthFailure,
            AssociationFailureCauseEnum::kAuthenticationFailed,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    },

    /**
     *  Credential rejection above the link layer: EAP for
     *  WPA-Enterprise, or PPP for cellular. Not reachable for the
     *  WPA2-PSK networks this implementation presently commissions,
     *  but it is a credential failure in every sense that matters to
     *  the commissioner, and it becomes live the moment
     *  `ConnectWiFiNetworkWithPDCAsync` is implemented. Terminal, and
     *  a true association failure.
     */
    { "login-failed", {
            Status::kAuthFailure,
            AssociationFailureCauseEnum::kAuthenticationFailed,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    },

    /**
     *  connman's generic link-establishment failure: the association
     *  itself did not complete. The credential was never exercised,
     *  so reporting an authentication failure would be a lie; the
     *  cause is almost always RF (the AP moved out of range mid-
     *  connect, refused the association, or is at capacity). Terminal,
     *  and a true association failure, but of the association rather
     *  than the authentication.
     */
    { "connect-failed", {
            Status::kOtherConnectionFailure,
            AssociationFailureCauseEnum::kAssociationFailed,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    },

    /**
     *  The AP refused the station outright, typically by MAC
     *  filtering or an access control list. Indistinguishable from
     *  "connect-failed" from the station's perspective, and mapped
     *  the same way: the association was refused, not the
     *  credential. Terminal, and a true association failure.
     */
    { "blocked", {
            Status::kOtherConnectionFailure,
            AssociationFailureCauseEnum::kAssociationFailed,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    },

    /**
     *  The network service vanished between the scan that resolved it
     *  and the connect that targeted it. Semantically this is
     *  "the SSID is not there", which is precisely
     *  `kNetworkNotFound` / `kSsidNotFound` -- and, unlike a generic
     *  connection failure, it is actionable for an installer, who can
     *  move the controller closer to the access point. Terminal, and
     *  a true association failure.
     */
    { "out-of-range", {
            Status::kNetworkNotFound,
            AssociationFailureCauseEnum::kSsidNotFound,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    },

    /**
     *  DHCP (or DHCPv6) did not yield an address. By the time connman
     *  runs its DHCP client, association *and* the 4-way handshake
     *  have both demonstrably succeeded: L2 is up and the credential
     *  was accepted. The failure is strictly at L3 -- an exhausted
     *  pool, an absent or unreachable server, or a lease timeout --
     *  which is exactly `kIPBindFailed`.
     *
     *  Deliberately **not** an association failure. The
     *  `AssociationFailure` event is diagnostic telemetry a fabric
     *  administrator reads to separate "wrong password" from "bad RF";
     *  raising one here, where the handshake demonstrably succeeded,
     *  would corrupt that stream and actively mislead. `OnConnectResult`
     *  alone tells the true story. Terminal, but not an association
     *  failure.
     */
    { "dhcp-failed", {
            Status::kIPBindFailed,
            AssociationFailureCauseEnum::kUnknown,
            kWlanStatusSuccess,
            false,
            true
        }
    },

    /**
     *  connman's Internet reachability probe (WISPr / `online-check`)
     *  failed. The service has nonetheless reached at least "ready":
     *  IPv4 and/or IPv6 is configured, which is precisely and
     *  entirely what Matter's `ConnectNetwork` contract requires.
     *
     *  connman's notion of "online" is strictly stronger than
     *  Matter's, and the gap is not hypothetical: a captive portal,
     *  an egress-filtered enterprise network, or an air-gapped
     *  installation will all fail this probe on a network that is
     *  perfectly commissionable. Letting it veto a connect would hard-
     *  fail commissioning on networks that work -- and the commissioner
     *  proves end-to-end reachability itself, via operational discovery
     *  and CASE, so the device has no business second-guessing it.
     *
     *  Hence: neither terminal nor an association failure. Should this
     *  arrive as a *failure* state at all -- it should not, given the
     *  service has already reached "ready" -- the state handler
     *  tolerates it and leaves the connect in flight rather than
     *  fabricating a terminal error.
     */
    { "online-check-failed", {
            Status::kSuccess,
            AssociationFailureCauseEnum::kUnknown,
            kWlanStatusSuccess,
            false,
            false
        }
    },

    /**
     *  A cellular SIM PIN is required and was not supplied.
     *  Unreachable for Wi-Fi, and mapped only so that the table
     *  covers connman's error vocabulary exhaustively (see connman
     *  `service.c:error2string()`); an unrecognized value would
     *  otherwise fall through to `kServiceErrorUnknown` and log.
     *  Treated as a credential failure, which is what it
     *  is. Terminal, and nominally an association failure (though on
     *  Wi-Fi, where the association-started latch will never have
     *  been set, the event is suppressed regardless).
     */
    { "pin-missing", {
            Status::kAuthFailure,
            AssociationFailureCauseEnum::kAuthenticationFailed,
            kWlanStatusUnspecifiedFailure,
            true,
            true
        }
    }
}};
// clang-format on

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

GVariant * UnboxedVariant::get() const noexcept
{
    return mValue;
}

UnboxedVariant::operator bool() const noexcept
{
    return mValue != nullptr;
}

static CHIP_ERROR RemoveObjectLocked(GHashTable * inObjectTable, const char * inObjectPath, GHashTable * inObjectProxies) noexcept
{
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectProxies != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_remove(inObjectTable, inObjectPath);
    g_hash_table_remove(inObjectProxies, inObjectPath);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetObjectPropertiesFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath,
                                                    GVariant *& outProperties) noexcept
{
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GVariant * props = static_cast<GVariant *>(g_hash_table_lookup(inObjectTable, inObjectPath));
    VerifyOrReturnError(props != nullptr, ChipError(ChipError::Range::kPOSIX, ENOENT));

    outProperties = props;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetObjectByteValueFromPropertiesLocked(GVariant * inProperties, const char * inKey, uint8_t & outValue) noexcept
{
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_WRONG_KEY_TYPE);

    GAutoPtr<GVariant> boxed(g_variant_lookup_value(inProperties, inKey, nullptr));
    VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    UnboxedVariant value(boxed.get());
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(g_variant_is_of_type(value.get(), G_VARIANT_TYPE_BYTE), CHIP_ERROR_WRONG_KEY_TYPE);

    outValue = g_variant_get_byte(value.get());

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetObjectStringValueFromPropertiesLocked(GVariant * inProperties, const char * inKey,
                                                           const char *& outValue) noexcept
{
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_WRONG_KEY_TYPE);

    GAutoPtr<GVariant> boxed(g_variant_lookup_value(inProperties, inKey, nullptr));
    VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    UnboxedVariant value(boxed.get());
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(g_variant_is_of_type(value.get(), G_VARIANT_TYPE_STRING), CHIP_ERROR_WRONG_KEY_TYPE);

    outValue = g_variant_get_string(value.get(), nullptr);

    return CHIP_NO_ERROR;
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
    VerifyOrReturnError(inObjectPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GVariant * props = static_cast<GVariant *>(g_hash_table_lookup(inObjectTable, inObjectPath));
    VerifyOrReturnError(props != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    ReturnErrorOnFailure(GetObjectStringValueFromPropertiesLocked(props, inKey, outValue));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetObjectTypeFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath, const char *& outType) noexcept
{
    return GetObjectStringValueFromPathLocked(inObjectTable, inObjectPath, kConnManObjectPropertyTypeKey, outType);
}

static CHIP_ERROR GetObjectPathFromProxyLocked(GDBusProxy * inProxy, const char *& outPath) noexcept
{
    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const char * path = g_dbus_proxy_get_object_path(inProxy);
    VerifyOrReturnError(path != nullptr, CHIP_ERROR_INTERNAL);

    outPath = path;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetObjectTypeFromProxyLocked(GDBusProxy * inProxy, GHashTable * inObjectTable, const char *& outType) noexcept
{
    const char * path;

    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(GetObjectPathFromProxyLocked(inProxy, path));

    ReturnErrorOnFailure(GetObjectTypeFromPathLocked(inObjectTable, path, outType));

    return CHIP_NO_ERROR;
}

static bool HasTechnologyLocked(GHashTable * inTechnologies, const char * inType) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;

    VerifyOrReturnValue(inTechnologies != nullptr, false);
    VerifyOrReturnValue(inType != nullptr, false);

    g_hash_table_iter_init(&iter, inTechnologies);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        GVariant * const props = static_cast<GVariant *>(value);
        if (props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;

        if ((GetObjectTypeFromPropertiesLocked(props, type) == CHIP_NO_ERROR) && (type != nullptr) && strcmp(type, inType) == 0)
        {
            return true;
        }
    }

    return false;
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
    VerifyOrReturnError(inDelta != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inDelta, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_INVALID_ARGUMENT);

    // If there is no base variant, then just take a reference to the
    // delta variant. Otherwise, proceed merging base and delta.

    if (inBase == nullptr)
    {
        outResult.reset(g_variant_ref(inDelta));
    }
    else
    {
        GVariantIter delta_iter;
        const char * delta_key = nullptr;
        GVariant * delta_value = nullptr;
        GVariantIter base_iter;
        const char * base_key = nullptr;
        GVariant * base_value = nullptr;
        GVariantBuilder builder;

        VerifyOrReturnError(g_variant_is_of_type(inBase, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_INVALID_ARGUMENT);

        // GLib does not have a container with set semantics, so we use a
        // hash table to uniquely contain all of the keys in the delta
        // variant.

        GAutoPtr<GHashTable> delta_keys(g_hash_table_new_full(g_str_hash, g_str_equal, g_free, nullptr));
        VerifyOrReturnError(delta_keys.get() != nullptr, CHIP_ERROR_INTERNAL);

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

    return CHIP_NO_ERROR;
}

static CHIP_ERROR MergeObjectPropertiesLocked(const char * inDescription, GHashTable * inObjectTable, const char * inObjectPath,
                                              GVariant * inObjectProperties) noexcept
{
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(g_variant_is_of_type(inObjectProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_INVALID_ARGUMENT);

    // If the specified object properties are empty, then there is
    // nothing to merge; otherwise, merge the properties key/value
    // pairs.

    LogProperties(inDescription, inObjectPath, inObjectProperties);

    if (g_variant_n_children(inObjectProperties) == 0)
    {
        return CHIP_NO_ERROR;
    }

    GVariant * current = nullptr;
    ReturnErrorOnFailure(GetObjectPropertiesFromPathLocked(inObjectTable, inObjectPath, current)
                             .NoErrorIf(ChipError(ChipError::Range::kPOSIX, ENOENT)));

    GAutoPtr<GVariant> merged;
    ReturnErrorOnFailure(Merge(current, inObjectProperties, merged));

    g_hash_table_replace(inObjectTable, g_strdup(inObjectPath), merged.release());

    return CHIP_NO_ERROR;
}

static CHIP_ERROR MergeObjectPropertiesLocked(const char * inDescription, GHashTable * inObjectTable, const char * inObjectPath,
                                              const char * inKey, GVariant * inValue) noexcept
{
    GVariantBuilder builder;

    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inValue != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    g_variant_builder_add(&builder, "{sv}", inKey, inValue);

    GAutoPtr<GVariant> property(g_variant_builder_end(&builder));

    ReturnErrorOnFailure(MergeObjectPropertiesLocked(inDescription, inObjectTable, inObjectPath, property.get()));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR MergeObjectPropertiesLocked(const char * inDescription, GHashTable * inObjectTable, GVariant * inObjects) noexcept
{
    GVariantIter iter;
    const char * path = nullptr;
    GVariant * props  = nullptr;

    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjects != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inObjects, G_VARIANT_TYPE("a(oa{sv})")), CHIP_ERROR_INVALID_ARGUMENT);

    g_variant_iter_init(&iter, inObjects);

    while (g_variant_iter_next(&iter, "(&o@a{sv})", &path, &props))
    {
        ReturnErrorOnFailure(MergeObjectPropertiesLocked(inDescription, inObjectTable, path, props));

        props = nullptr;
    }

    return CHIP_NO_ERROR;
}

static bool ByteSpanEqualsString(const ByteSpan & inSpan, const char * inString) noexcept
{
    VerifyOrReturnError(inString != nullptr, false);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: comparing '%.*s' v. '%s'...", __func__,
                  static_cast<int>(inSpan.size()), inSpan.data(), inString);

    const size_t length = strlen(inString);

    return ((inSpan.size() == length) && (memcmp(inSpan.data(), inString, length) == 0));
}

static ConnManService * GetWiFiServiceProxyFromSsidLocked(const ByteSpan & inSsid, GHashTable * inProxies,
                                                          GHashTable * inServices) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;

    VerifyOrReturnValue(inSsid.size() > 0, nullptr);
    VerifyOrReturnValue(inProxies != nullptr, nullptr);
    VerifyOrReturnValue(inServices != nullptr, nullptr);

    g_hash_table_iter_init(&iter, inServices);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const path = static_cast<const char *>(key);
        GVariant * const props  = static_cast<GVariant *>(value);
        if ((path == nullptr) || (props == nullptr) || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;
        if ((GetObjectTypeFromPropertiesLocked(props, type) != CHIP_NO_ERROR) || (type == nullptr) ||
            strcmp(type, kConnManServicePropertyTypeWiFiValue) != 0)
        {
            continue;
        }

        const char * name = nullptr;
        if ((GetObjectNameFromPropertiesLocked(props, name) == CHIP_NO_ERROR) && (name != nullptr) &&
            ByteSpanEqualsString(inSsid, name))
        {
            return static_cast<ConnManService *>(g_hash_table_lookup(inProxies, path));
        }
    }

    return nullptr;
}

static ConnManTechnology * GetTechnologyProxyFromTypeLocked(const char * inType, GHashTable * inProxies,
                                                            GHashTable * inTechnologies) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;

    VerifyOrReturnValue(inType != nullptr, nullptr);
    VerifyOrReturnValue(inProxies != nullptr, nullptr);
    VerifyOrReturnValue(inTechnologies != nullptr, nullptr);

    g_hash_table_iter_init(&iter, inTechnologies);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const path = static_cast<const char *>(key);
        GVariant * const props  = static_cast<GVariant *>(value);
        if ((path == nullptr) || (props == nullptr) || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;
        if ((GetObjectTypeFromPropertiesLocked(props, type) == CHIP_NO_ERROR) && (type != nullptr) && strcmp(type, inType) == 0)
        {
            return static_cast<ConnManTechnology *>(g_hash_table_lookup(inProxies, path));
        }
    }

    return nullptr;
}

static CHIP_ERROR GetServiceInterfaceFromEthernetLocked(GVariant * inEthernet, GAutoPtr<gchar> & outInterface) noexcept
{
    const char * interface;

    VerifyOrReturnError(inEthernet != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(g_variant_is_of_type(inEthernet, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_WRONG_KEY_TYPE);

    const gboolean status = g_variant_lookup(inEthernet, kConnManServicePropertyEthernetInterfaceKey, "&s", &interface);
    VerifyOrReturnError(status && interface, CHIP_ERROR_KEY_NOT_FOUND);

    outInterface.reset(g_strdup(interface));
    VerifyOrReturnError(outInterface.get() != nullptr, CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetServiceInterfaceFromPropertiesLocked(GVariant * inProperties, GAutoPtr<gchar> & outInterface) noexcept
{
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_WRONG_KEY_TYPE);

    GAutoPtr<GVariant> variant(g_variant_lookup_value(inProperties, kConnManServicePropertyEthernetKey, nullptr));
    VerifyOrReturnError(variant.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    ReturnErrorOnFailure(GetServiceInterfaceFromEthernetLocked(variant.get(), outInterface));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetServiceInterfaceFromPathLocked(GHashTable * inObjectTable, const char * inObjectPath,
                                                    GAutoPtr<gchar> & outInterface) noexcept
{
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GVariant * const props = static_cast<GVariant *>(g_hash_table_lookup(inObjectTable, inObjectPath));
    VerifyOrReturnError(props != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    ReturnErrorOnFailure(GetServiceInterfaceFromPropertiesLocked(props, outInterface));

    return CHIP_NO_ERROR;
}

static int GetServiceStateScore(const char * inState) noexcept
{
    VerifyOrReturnValue(inState != nullptr, 0);

    if (strcmp(inState, kConnManServicePropertyStateOnlineValue) == 0)
    {
        return 7;
    }
    else if (strcmp(inState, kConnManServicePropertyStateReadyValue) == 0)
    {
        return 6;
    }
    else if (strcmp(inState, kConnManServicePropertyStateConfigurationValue) == 0)
    {
        return 5;
    }
    else if (strcmp(inState, kConnManServicePropertyStateAssociationValue) == 0)
    {
        return 4;
    }
    else if (strcmp(inState, kConnManServicePropertyStateIdleValue) == 0)
    {
        return 3;
    }
    else if (strcmp(inState, kConnManServicePropertyStateDisconnectValue) == 0)
    {
        return 2;
    }
    else if (strcmp(inState, kConnManServicePropertyStateOfflineValue) == 0)
    {
        return 1;
    }

    return 0;
}

static CHIP_ERROR GetBestServicePathForTypeLocked(const char * inType, GHashTable * inServices, GAutoPtr<gchar> & outPath) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    int best_score = -1;
    GAutoPtr<gchar> best_path;

    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inServices != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_iter_init(&iter, inServices);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const current_path = static_cast<const char *>(key);
        GVariant * const props          = static_cast<GVariant *>(value);
        if (props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;
        if (GetObjectTypeFromPropertiesLocked(props, type) != CHIP_NO_ERROR || type == nullptr || strcmp(type, inType) != 0)
        {
            continue;
        }

        const char * state = nullptr;
        if (GetObjectStateFromPropertiesLocked(props, state) != CHIP_NO_ERROR)
        {
            continue;
        }

        const int current_score = GetServiceStateScore(state);
        if (current_score > best_score)
        {
            best_score = current_score;
            best_path.reset(g_strdup(current_path));
        }
    }

    VerifyOrReturnError(best_path.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    outPath.reset(best_path.release());

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetBestServiceInterfaceForTypeLocked(const char * inType, GHashTable * inServices,
                                                       GAutoPtr<gchar> & outInterface) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;
    int best_score = -1;
    GAutoPtr<gchar> best_interface;

    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inServices != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_iter_init(&iter, inServices);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        GVariant * const props = static_cast<GVariant *>(value);
        if (props == nullptr || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;
        if (GetObjectTypeFromPropertiesLocked(props, type) != CHIP_NO_ERROR || type == nullptr || strcmp(type, inType) != 0)
        {
            continue;
        }

        // At this point of execution, retrieval of the state property
        // may fail; proceed with scoring anyway. A null state will be
        // factored into scoring accordingly.

        const char * state = nullptr;
        RETURN_SAFELY_IGNORED GetObjectStringValueFromPropertiesLocked(props, kConnManServicePropertyStateKey, state);

        const int current_score = GetServiceStateScore(state);

        GAutoPtr<gchar> current_interface;
        if (GetServiceInterfaceFromPropertiesLocked(props, current_interface) != CHIP_NO_ERROR ||
            current_interface.get() == nullptr)
        {
            continue;
        }

        if (current_score > best_score)
        {
            best_score = current_score;
            best_interface.reset(current_interface.release());
        }
    }

    VerifyOrReturnError(best_interface.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    outInterface.reset(best_interface.release());

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetWiFiServicePropertiesFromSsidLocked(const ByteSpan & inSsid, GHashTable * inObjectTable,
                                                         GAutoPtr<GVariant> & outProperties) noexcept
{
    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;

    VerifyOrReturnError(inSsid.size() > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    outProperties.reset();

    g_hash_table_iter_init(&iter, inObjectTable);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char * const path = static_cast<const char *>(key);
        GVariant * const props  = static_cast<GVariant *>(value);
        if ((path == nullptr) || (props == nullptr) || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;
        if ((GetObjectTypeFromPropertiesLocked(props, type) != CHIP_NO_ERROR) || (type == nullptr) ||
            (strcmp(type, kConnManServicePropertyTypeWiFiValue) != 0))
        {
            continue;
        }

        // ConnMan's "Name" for Wi-Fi services is the SSID string.
        const char * name = nullptr;
        if ((GetObjectNameFromPropertiesLocked(props, name) == CHIP_NO_ERROR) && (name != nullptr) &&
            ByteSpanEqualsString(inSsid, name))
        {
            outProperties.reset(g_variant_ref(props));
            VerifyOrReturnError(outProperties.get() != nullptr, CHIP_ERROR_NO_MEMORY);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

static CHIP_ERROR GetWiFiServiceSecurityLocked(GVariant * inProperties, chip::BitFlags<WiFiSecurityBitmap> & outSecurity) noexcept
{
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GAutoPtr<GVariant> boxed(g_variant_lookup_value(inProperties, kConnManServicePropertySecurityKey, nullptr));
    VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    UnboxedVariant value(boxed.get());
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(g_variant_is_of_type(value.get(), G_VARIANT_TYPE_STRING_ARRAY), CHIP_ERROR_WRONG_KEY_TYPE);

    chip::BitFlags<WiFiSecurityBitmap> security;
    GVariantIter it;
    const char * s = nullptr;

    g_variant_iter_init(&it, value.get());

    while (g_variant_iter_next(&it, "&s", &s))
    {
        if (s == nullptr)
        {
            continue;
        }

        // Attempt to map connman's notion of Wi-Fi security as best
        // we are able.
        //
        // At this time, IEEE 802.1x and WPS are intentionally
        // unmapped.

        if (strcmp(s, kConnManServicePropertySecurityNoneValue) == 0)
        {
            security.Set(WiFiSecurityBitmap::kUnencrypted);
        }
        else if (strcmp(s, kConnManServicePropertySecurityWEPValue) == 0)
        {
            security.Set(WiFiSecurityBitmap::kWep);
        }
        else if ((strcmp(s, kConnManServicePropertySecurityPSKValue) == 0) ||
                 (strcmp(s, kConnManServicePropertySecurityRSNValue) == 0) ||
                 (strcmp(s, kConnManServicePropertySecurityWPAValue) == 0))
        {
            security.Set(WiFiSecurityBitmap::kWpa2Personal);
        }
    }

    outSecurity = security;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetWiFiServiceStrengthLocked(GVariant * inProperties, int8_t & outStrength) noexcept
{
    uint8_t value;

    ReturnErrorOnFailure(GetObjectByteValueFromPropertiesLocked(inProperties, kConnManServicePropertyStrengthKey, value));

    outStrength = std::clamp(static_cast<int8_t>(value), static_cast<int8_t>(0), static_cast<int8_t>(100));

    return CHIP_NO_ERROR;
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
 *  @retval  -EINVAL        If @a inNetworkName is invalid.
 *
 */
static CHIP_ERROR ValidateWiFiServiceSsid(const ByteSpan & inSsid, const char *& outReason) noexcept
{
    static constexpr size_t kMinLength = 1;
    static constexpr size_t kMaxLength = 32;

    const size_t lLength = inSsid.size();
    VerifyOrReturnError(lLength >= kMinLength, CHIP_ERROR_INVALID_ARGUMENT, outReason = kNetworkNameIsTooShort);
    VerifyOrReturnError(lLength <= kMaxLength, CHIP_ERROR_INVALID_ARGUMENT, outReason = kNetworkNameIsTooLong);

    return CHIP_NO_ERROR;
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

    // Do a quick character range sanity check.

    const size_t lLength = inPassphrase.size();
    VerifyOrReturnError(lLength >= kMinLength, CHIP_ERROR_INVALID_ARGUMENT, outReason = kNetworkPassphraseIsTooShort);
    VerifyOrReturnError(lLength <= kMaxLength, CHIP_ERROR_INVALID_ARGUMENT, outReason = kNetworkPassphraseIsTooLong);

    return CHIP_NO_ERROR;
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
    VerifyOrReturnError(inContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    inContext->mStatus = inContext->mCallable();

    return CHIP_NO_ERROR;
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

static CHIP_ERROR MapClientError(const GError * inError, CHIP_ERROR inDefaultError) noexcept
{
    VerifyOrReturnValue(inError != nullptr, inDefaultError);

    // Only a *remote* GError carries a D-Bus error name. A local
    // GLib/GIO failure (transport teardown, timeout, cancellation)
    // does not, and must not be run through the connman table.
    //
    // NOTE: this depends on the GError not having been passed through
    // g_dbus_error_strip_remote_error(), which destroys the name.

    VerifyOrReturnValue(g_dbus_error_is_remote_error(inError), inDefaultError);

    GAutoPtr<char> name(g_dbus_error_get_remote_error(inError));
    VerifyOrReturnValue(name, inDefaultError);

    for (const auto & entry : kClientErrorTable)
    {
        if (strcmp(entry.mName, name.get()) == 0)
        {
            return ChipError(ChipError::Range::kPOSIX, entry.mErrno);
        }
    }

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "unrecognized client error '%s'", name.get());

    return inDefaultError;
}

static CHIP_ERROR MapClientError(const GError * inError) noexcept
{
    return MapClientError(inError, CHIP_ERROR_INTERNAL);
}

static ServiceError MapServiceError(const char * inServiceError) noexcept
{
    // A cleared or absent "Error" is not a failure; callers gate on
    // mIsTerminal, so return the benign mapping rather than "unknown".

    VerifyOrReturnValue(inServiceError != nullptr, kServiceErrorNone);

    for (const auto & entry : kServiceErrorTable)
    {
        if (strcmp(entry.mName, inServiceError) == 0)
        {
            return entry.mError;
        }
    }

    ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "unrecognized service error '%s'; reporting as unknown",
                 inServiceError);

    return kServiceErrorUnknown;
}

} // namespace

// Matter Linux Connectivity Manager Implementation for connman

// Family Connectivity Aggregate

ConnectivityManagerImpl_NetworkManagementConnMan::FamilyConnectivityAggregate::FamilyConnectivityAggregate() noexcept :
    mReportable(false), mAnyEstablished(false), mAddress(Inet::IPAddress::Any)
{
    return;
}

// Network Service Family Connectivity State

ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceFamilyConnectivityState::
    NetworkServiceFamilyConnectivityState() noexcept :
    mConnectivity(NetworkServiceConnectivity::kUnknown),
    mAddress(Inet::IPAddress::Any)
{
    return;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceFamilyConnectivityState::operator==(
    const NetworkServiceFamilyConnectivityState & inOther) const noexcept
{
    return ((mConnectivity == inOther.mConnectivity) && (mAddress == inOther.mAddress));
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceFamilyConnectivityState::operator!=(
    const NetworkServiceFamilyConnectivityState & inOther) const noexcept
{
    return !(*this == inOther);
}

// Network Service Family Connectivity State

bool ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceConnectivityState::operator==(
    const NetworkServiceConnectivityState & inOther) const noexcept
{
    return ((mIPv4 == inOther.mIPv4) && (mIPv6 == inOther.mIPv6));
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceConnectivityState::operator!=(
    const NetworkServiceConnectivityState & inOther) const noexcept
{
    return !(*this == inOther);
}

// Network Service State

ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceState::NetworkServiceState() noexcept :
    mType(nullptr), mDescription(nullptr), mIfName{}, mPendingConnectivity(), mReportedConnectivity()
{
    return;
}

// Wi-Fi Scan State

ConnectivityManagerImpl_NetworkManagementConnMan::WiFiScanState::WiFiScanState() noexcept : mSsid(), mCount(0)
{
    return;
}

// Introspection

bool ConnectivityManagerImpl_NetworkManagementConnMan::WiFiScanState::IsActive() const noexcept
{
    return (mCount != 0);
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::WiFiScanState::IsDirected() const noexcept
{
    return !mSsid.empty();
}

// Mutation

void ConnectivityManagerImpl_NetworkManagementConnMan::WiFiScanState::Reset() noexcept
{
    mSsid.reset();

    mCount = 0;
}

// Initialization

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::Init(ConnectivityManagerImpl & inConnectivityManagerImpl)
{
    // Initialize the base network management class.

    ReturnErrorOnFailure(NetworkManagementBasis::Init());

    mConnectivityManagerImpl = &inConnectivityManagerImpl;

    static_assert(sNetworkServiceTypeDescriptors.size() ==
                      ConnectivityManagerImpl_NetworkManagementConnMan::kNetworkServiceStateCount,
                  "network service descriptor/state table size mismatch");

    for (size_t i = 0; i < kNetworkServiceStateCount; i++)
    {
        mNetworkServiceStates[i]              = NetworkServiceState{};
        mNetworkServiceStates[i].mType        = sNetworkServiceTypeDescriptors[i].mType;
        mNetworkServiceStates[i].mDescription = sNetworkServiceTypeDescriptors[i].mDescription;
    }

    mWiFiActiveScanState.Reset();
    mWiFiClientConnectPassphrase.reset();
    mWiFiClusterConnectAssociationStarted = false;
    mWiFiClusterConnectPending            = false;
    mWiFiClusterConnectScanState.Reset();
    mWiFiClusterConnectServiceError.reset();
    mWiFiStationConnected         = false;
    mWiFiStationMode              = ConnectivityManager::kWiFiStationMode_NotSupported;
    mWiFiStationReconnectInterval = {};

    return CHIP_NO_ERROR;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::InitAgentOnGLib(ConnectivityManagerImpl_NetworkManagementConnMan * inSelf,
                                                                  GDBusConnection * inConnection, const char * inPath,
                                                                  ConnManAgent *& outSkeleton, GError ** outError) noexcept
{
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    VerifyOrReturnError(inSelf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inConnection != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GAutoPtr<ConnManAgent> skeleton(conn_man_agent_skeleton_new());
    VerifyOrReturnError(skeleton.get() != nullptr, CHIP_ERROR_INTERNAL);

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

    GAutoPtr<GError> err;
    const gboolean exported =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton.get()), inConnection, inPath, &err.GetReceiver());
    if (!exported || err.get() != nullptr)
    {
        if (outError != nullptr)
        {
            *outError = err.get() ? g_error_copy(err.get()) : nullptr;
        }

        return CHIP_ERROR_INTERNAL;
    }

    outSkeleton = CONN_MAN_AGENT(g_object_ref(skeleton.get()));

    return CHIP_NO_ERROR;
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
        LogErrorOnFailure(UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
            ShutdownAgentOnGLib(this, connection.get(), inPath, skeleton.get());

            return CHIP_NO_ERROR;
        }));
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
    GAutoPtr<gchar> path;
    GVariant * props  = nullptr;
    const char * name = nullptr;

    // Iterate over all Wi-Fi networks and get the best connected
    // network. From that network, copy its SSID.

    ReturnErrorOnFailure(
        GetBestServicePathForTypeLocked(kConnManServicePropertyTypeWiFiValue, mConnManClient.mServices.get(), path));

    ReturnErrorOnFailure(GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path.get(), props));

    ReturnErrorOnFailure(GetObjectNameFromPropertiesLocked(props, name));

    const size_t length = std::min(sizeof(outNetwork.networkID), strlen(name));

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Current connected network: %.*s",
                    static_cast<int>(length), name);

    memcpy(outNetwork.networkID, name, length);
    outNetwork.networkIDLen = length;

    return CHIP_NO_ERROR;
}

// Event Handling

void ConnectivityManagerImpl_NetworkManagementConnMan::OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) {}

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
// Ethernet Control Plane Management

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetEthernetIfName()
{
    return GetInterfaceName(kConnManObjectPropertyTypeEthernetValue);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::UpdateEthernetNetworkingStatus()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // TBD
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
// Wi-Fi Control Plane Management

// Observation

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiIfName()
{
    return GetInterfaceName(kConnManObjectPropertyTypeWiFiValue);
}

// Control

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::StartNetworkManagementOnGLib()
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

void ConnectivityManagerImpl_NetworkManagementConnMan::StartNonConcurrentWiFiManagement()
{
    StartWiFiManagement();

    for (size_t cnt = 0; cnt < WIFI_START_CHECK_ATTEMPTS; cnt++)
    {
        if (IsWiFiManagementStarted())
        {
            LogErrorOnFailure(DeviceControlServer::DeviceControlSvr().PostOperationalNetworkStartedEvent());

            ChipLogProgress(DeviceLayer, "Non-concurrent mode Wi-Fi Management Started.");

            return;
        }

        usleep(WIFI_START_CHECK_TIME_USEC);
    }

    ChipLogError(Ble, "Non-concurrent mode Wi-Fi Management taking too long to start.");
}

void ConnectivityManagerImpl_NetworkManagementConnMan::StartWiFiManagement()
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

// Introspection

bool ConnectivityManagerImpl_NetworkManagementConnMan::HasWiFiClusterConnectPendingLocked() const noexcept
{
    return mWiFiClusterConnectPending;
}

/**
 *  @brief
 *    Determine whether a per-family connectivity transition warrants
 *    a device event.
 *
 *  A family transition is reportable on a connectivity state change
 *  or, while established, on an address change (the latter because
 *  any one-shot consumer state (for example, published DNS-SD
 *  address records) derived from the previous address is now
 *  stale). The kUnknown -> kLost transition is deliberately absorbed
 *  without an event: reporting "lost" connectivity that was never
 *  established is startup noise.
 *
 */
bool ConnectivityManagerImpl_NetworkManagementConnMan::IsFamilyConnectivityChangeReportable(
    const NetworkServiceFamilyConnectivityState & inPending, const NetworkServiceFamilyConnectivityState & inReported) noexcept
{
    const bool absorbed     = ((inReported.mConnectivity == NetworkServiceConnectivity::kUnknown) &&
                           (inPending.mConnectivity == NetworkServiceConnectivity::kLost));
    const bool stateChanged = (inPending.mConnectivity != inReported.mConnectivity);
    const bool addressChanged =
        ((inPending.mConnectivity == NetworkServiceConnectivity::kEstablished) && (inPending.mAddress != inReported.mAddress));

    return ((stateChanged && !absorbed) || addressChanged);
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiPendingConnectServicePathLocked(const char * inPath) const noexcept
{
    VerifyOrReturnValue(inPath != nullptr, false);
    VerifyOrReturnValue(mConnManAgentServer.mPendingService, false);

    const char * const pending = g_dbus_proxy_get_object_path(G_DBUS_PROXY(mConnManAgentServer.mPendingService.get()));
    VerifyOrReturnValue(pending != nullptr, false);

    return (strcmp(pending, inPath) == 0);
}

// Wi-Fi Station Control Plane Management

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiManagementStarted()
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    return !!mConnManClient.mManagerProxy;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationApplicationControlled()
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    return (mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled);
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationConnected()
{
    // Get the Wi-Fi technology and return the value of the Connected property.
    //
    // We should possibly also consider the State property of the
    // currently-connected Wi-Fi network service.

    return mWiFiStationConnected;
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationEnabled()
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    return (mWiFiStationMode == ConnectivityManager::kWiFiStationMode_Enabled);
}

bool ConnectivityManagerImpl_NetworkManagementConnMan::IsWiFiStationProvisioned()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // TBD

    return false;
}

// Observation

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiBssId(MutableByteSpan & outBssId)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Use the nl80211 API to retrieve this, similar to 'iw dev wlan0
    // link', where "wlan0" is the network interface name of the
    // currently-connected Wi-Fi network service. Failing that, the
    // Wi-Fi backend could be used.

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiSecurityType(
    app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType)
{
    // ConnMan only distinguishes between "psk" and "rsn" security
    // types, lumping all of WPA-, WPA-2-, and WPA-3-Personal under
    // "psk". For now, just use 'kUnspecified' for now. We may be able
    // to get this through nl80211 or the Wi-Fi backend in the future.

    outSecurityType = SecurityTypeEnum::kUnspecified;

    return CHIP_NO_ERROR;
}

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiStationMode()
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    return mWiFiStationMode;
}

System::Clock::Timeout ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiStationReconnectInterval()
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiVersion(
    app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

// Mutation

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiStationMode(const ConnectivityManager::WiFiStationMode & inWiFiStationMode)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    VerifyOrReturnError(inWiFiStationMode != ConnectivityManager::kWiFiStationMode_NotSupported, CHIP_ERROR_INVALID_ARGUMENT);

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

            ConnManTechnology * const technology =
                GetTechnologyProxyFromTypeLocked(kConnManTechnologyPropertyTypeWiFiValue, mConnManClient.mTechnologyProxies.get(),
                                                 mConnManClient.mTechnologies.get());
            VerifyOrReturnError(technology != nullptr, ChipError(ChipError::Range::kPOSIX, ENOENT));

            ReturnErrorOnFailure(TechnologySetPoweredLocked(lock, technology, powered));
            break;
        }

        default:
            break;
        }

        mWiFiStationMode = inWiFiStationMode;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval)
{
    mWiFiStationReconnectInterval = inInterval;

    return CHIP_NO_ERROR;
}

// Worker

void ConnectivityManagerImpl_NetworkManagementConnMan::ClearWiFiStationProvision()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // TBD but probably remove all favorited networks.
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::CommitConfig()
{
    // As presently implemented, there is nothing to do for this with
    // connman. On successful connection to a Wi-Fi (or other) network
    // services, the necessary credentials are serialized to
    // non-volatile storage.

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ConnectWiFiNetworkAsync(
    ByteSpan inSsid, ByteSpan inCredentials, NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    const char * reason = nullptr;

    // Ensure that we have an initialized Connectivity Manager
    // implementation pointer.

    VerifyOrReturnError(mConnectivityManagerImpl != nullptr, CHIP_ERROR_UNINITIALIZED);

    // Verify that there is not another Wi-Fi station connection in-flight.

    VerifyOrReturnError(!mConnectivityManagerImpl->IsWiFiStationConnecting(), CHIP_ERROR_BUSY);

    // Sanity check the SSID.

    ReturnErrorAndLogOnFailure(ValidateWiFiServiceSsid(inSsid, reason), DeviceLayer, "invalid Wi-Fi SSID '%.*s': %s",
                               static_cast<int>(inSsid.size()), inSsid.data(), reason);

    // Sanity check the credentials.

    ReturnErrorAndLogOnFailure(ValidateWiFiServicePassphrase(inCredentials, reason), DeviceLayer, "invalid Wi-Fi passphrase: %s",
                               reason);

    // Set the connection completion callback.

    mConnectivityManagerImpl->SetOneShotConnectCallback(inConnectCallback);

    // Mark the connect in flight. This is the sole gate on the
    // service state machine driving the Matter completion, and it is
    // retired by ConcludeWiFiClusterConnectLocked.

    mWiFiClusterConnectPending = true;

    // Unconditionally cache the provided SSID.

    mWiFiClusterConnectScanState.mSsid = inSsid;

    // Unconditionally cache the provided credentials.

    mWiFiClientConnectPassphrase = inCredentials;

    // On any synchronous failure below, `Service.Connect()` was never
    // issued: connman will never request the credential, and
    // HandleServiceConnectComplete will never run. Retire the connect
    // *without* dispatching a Matter completion, since
    // LinuxWiFiDriver::ConnectNetwork completes the command from the
    // CHIP_ERROR we return, and drop the one-shot callback that will
    // now never fire. Disarmed on success.

    auto connectGuard = ScopeExit([&]() {
        ScrubWiFiClientConnectPassphraseLocked();

        mConnectivityManagerImpl->SetOneShotConnectCallback(nullptr);

        LogErrorOnFailure(ConcludeWiFiClusterConnectLocked());
    });

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

    ConnManService * const service =
        GetWiFiServiceProxyFromSsidLocked(inSsid, mConnManClient.mServiceProxies.get(), mConnManClient.mServices.get());
    if (service != nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting to '%.*s'...",
                        static_cast<int>(inSsid.size()), inSsid.data());

        ReturnErrorOnFailure(HandleServiceConnectRequestLocked(lock, service));
    }
    else
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "scanning for '%.*s'...",
                        static_cast<int>(inSsid.size()), inSsid.data());

        ConnManTechnology * const technology = GetTechnologyProxyFromTypeLocked(
            kConnManTechnologyPropertyTypeWiFiValue, mConnManClient.mTechnologyProxies.get(), mConnManClient.mTechnologies.get());
        VerifyOrReturnError(technology != nullptr, ChipError(ChipError::Range::kPOSIX, ENOENT));

        // Initialize the connect scan count.

        mWiFiClusterConnectScanState.mCount = 1;

        // Attempt to scan for the desired network service.

        ReturnErrorOnFailure(TechnologyScanLocked(lock, technology));
    }

    // Success; disarm the scoped clean-up.

    connectGuard.release();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ConnectWiFiNetworkWithPDCAsync(
    ByteSpan inSsid, ByteSpan inNetworkIdentity, ByteSpan inClientIdentity, const Crypto::P256Keypair & inClientIdentityKeypair,
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback)
{
    // ConnMan fully supports connecting Wi-Fi stations and access
    // points using certificate-based security; this implementation
    // would simply have to cache the input parameters to satisfy the
    // requisite agent input request from those cached input
    // parameters.

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::StartWiFiScan(ByteSpan inSsid,
                                                                NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback)
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    // Ensure that we have an initialized Connectivity Manager
    // implementation pointer.

    VerifyOrReturnError(mConnectivityManagerImpl != nullptr, CHIP_ERROR_UNINITIALIZED);

    // If there is another ongoing scan request, reject the new one.

    VerifyOrReturnError(!mConnectivityManagerImpl->IsWiFiStationScanning(), CHIP_ERROR_INCORRECT_STATE);

    // If Wi-Fi is not among the supported technologies, there is
    // nothing further to do.

    const bool has_technology = HasTechnologyLocked(mConnManClient.mTechnologies.get(), kConnManTechnologyPropertyTypeWiFiValue);
    VerifyOrReturnError(has_technology, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // ConnMan does not support active, directed scans. Consequently,
    // all we can reasonably do here is set a cache of the desired
    // SSID, do a Wi-Fi passive, broadcast scan, and then see if the
    // desired SSID is among the known Wi-Fi network services at the
    // completion of the scan.

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "scanning for '%.*s'...", static_cast<int>(inSsid.size()),
                    inSsid.data());

    // Cache the provided SSID.

    mWiFiActiveScanState.mSsid = inSsid;

    // Initialize the "active" scan count.

    mWiFiActiveScanState.mCount = 1;

    mConnectivityManagerImpl->SetOneShotScanCallback(inScanCallback);

    auto scanGuard = ScopeExit([&]() {
        mConnectivityManagerImpl->SetOneShotScanCallback(nullptr);

        mWiFiActiveScanState.Reset();
    });

    ReturnErrorOnFailure(TechnologyScanLocked(lock,
                                              GetTechnologyProxyFromTypeLocked(kConnManTechnologyPropertyTypeWiFiValue,
                                                                               mConnManClient.mTechnologyProxies.get(),
                                                                               mConnManClient.mTechnologies.get())));

    // Success; disarm the scoped clean-up.

    scanGuard.release();

    return CHIP_NO_ERROR;
}

// Wi-Fi Soft Access Point (AP) Control Plane Management

// Observation

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl_NetworkManagementConnMan::GetWiFiApMode()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.

    return ConnectivityManager::kWiFiAPMode_NotSupported;
}

// Mutation

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout)
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}

// Control

void ConnectivityManagerImpl_NetworkManagementConnMan::DemandStartWiFiAp()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}

void ConnectivityManagerImpl_NetworkManagementConnMan::StopOnDemandWiFiAp()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}

void ConnectivityManagerImpl_NetworkManagementConnMan::MaintainOnDemandWiFiAp()
{
    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s", __func__);

    // Were this supported, this would use the ConnMan technology tethering API.
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

// Mutation

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetConnectivityChangeString(ConnectivityChange inChange) noexcept
{
    return ((inChange == kConnectivity_Established) ? "established" : (inChange == kConnectivity_Lost) ? "lost" : "no change");
}

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetNetworkServiceConnectivityString(
    NetworkServiceConnectivity inNetworkServiceConnectivity) noexcept
{
    return ((inNetworkServiceConnectivity == NetworkServiceConnectivity::kEstablished) ? "established"
                : (inNetworkServiceConnectivity == NetworkServiceConnectivity::kLost)  ? "lost"
                                                                                       : "unknown");
}

// Mutation

/**
 *  @brief
 *    Project a per-family connectivity aggregate onto the event's
 *    `ConnectivityChange` field.
 *
 *  Maps the reduced aggregate for one IP family onto the tri-state
 *  verdict carried by `OnInternetConnectivityChange`:
 *  `kConnectivity_NoChange` when nothing is reportable, otherwise
 *  `kConnectivity_Established` or `kConnectivity_Lost` according to
 *  whether the family is established on any tracked service. The
 *  return value lets the caller gate address population and event
 *  emission on the same reportability test.
 *
 *  @param[in]   inAggregate
 *    The per-family aggregate produced by folding every tracked
 *    service's (pending, reported) pair via
 *    `AccumulateFamilyConnectivity`.
 *
 *  @param[out]  outChange
 *    On return, the `ConnectivityChange` verdict for the family:
 *    `kConnectivity_NoChange`, `kConnectivity_Established`, or
 *    `kConnectivity_Lost`.
 *
 *  @returns
 *    True if the family had a reportable change (equivalently,
 *    `outChange` is not `kConnectivity_NoChange`); false otherwise.
 *
 *  @sa AccumulateFamilyConnectivity
 *
 *  @private
 *
 */
bool ConnectivityManagerImpl_NetworkManagementConnMan::SetConnectivityChangeFromAggregate(
    const FamilyConnectivityAggregate & inAggregate, ConnectivityChange & outChange) noexcept
{
    outChange = kConnectivity_NoChange;

    if (inAggregate.mReportable)
    {
        outChange = (inAggregate.mAnyEstablished ? kConnectivity_Established : kConnectivity_Lost);
    }

    return inAggregate.mReportable;
}

// Helper Methods

/**
 *  @brief
 *    Fold one service type's (pending, reported) family pair into the
 *    running per-family aggregate.
 *
 *  The fold step invoked once per tracked service type, per family,
 *  while building the `FamilyConnectivityAggregate` for an
 *  `OnInternetConnectivityChange` event. It raises `mReportable` when
 *  this service's pending-versus-reported transition is surfaceable
 *  (per `IsFamilyConnectivityChangeReportable`), records the family
 *  as established when this service's pending state is established,
 *  and--on the first established entry in table (declaration)
 *  order--captures that service's address as the aggregate's
 *  representative address.
 *
 *  @param[in]      inPending
 *    This service's freshly recomputed family connectivity.
 *
 *  @param[in]      inReported
 *    This service's family connectivity as last reported (the
 *    debounce baseline), against which reportability is judged.
 *
 *  @param[in,out]  inOutAggregate
 *    The per-family aggregate updated in place; must be
 *    default-constructed before the first fold.
 *
 *  @sa SetConnectivityChangeFromAggregate
 *  @sa IsFamilyConnectivityChangeReportable
 *
 *  @private
 *
 */
void ConnectivityManagerImpl_NetworkManagementConnMan::AccumulateFamilyConnectivity(
    const NetworkServiceFamilyConnectivityState & inPending, const NetworkServiceFamilyConnectivityState & inReported,
    FamilyConnectivityAggregate & inOutAggregate) noexcept
{
    if (IsFamilyConnectivityChangeReportable(inPending, inReported))
    {
        inOutAggregate.mReportable = true;
    }

    if (inPending.mConnectivity == NetworkServiceConnectivity::kEstablished)
    {
        // The first-established entry in table (declaration) order
        // supplies the cosmetic single address carried by the event.

        if (!inOutAggregate.mAnyEstablished)
        {
            inOutAggregate.mAddress = inPending.mAddress;
        }

        inOutAggregate.mAnyEstablished = true;
    }
}

/**
 *  @brief
 *    Populate a Matter Wi-Fi scan response from a connman service
 *    property dictionary.
 *
 *  Translates one connman Wi-Fi service's cached properties into a
 *  single `WiFiScanResponse`. The service "Name" is the SSID; a
 *  hidden network surfaces with an empty Name and is rejected here,
 *  which is precisely the non-hidden filter a broadcast scan
 *  requires. Security is mapped from connman's "Security" array via
 *  #GetWiFiServiceSecurityLocked, falling back to `kWpa2Personal`
 *  when connman reports nothing recognizable so that the bitmap is
 *  never empty. BSSID, channel, and band are proxy values: connman
 *  exposes no introspectable equivalents, and signal strength is
 *  reported qualitatively (0-100) rather than in dBm. A failed
 *  strength read is non-fatal and leaves the zero default.
 *
 *  @param[in]   inProperties
 *    A pointer to the connman service property dictionary (a{sv}).
 *
 *  @param[out]  outResponse
 *    On `#CHIP_NO_ERROR`, the populated scan response. Left
 *    unmodified on failure.
 *
 *  @retval  #CHIP_NO_ERROR
 *    If a response was built.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT
 *    If `inProperties` is null, or the service Name is absent, empty
 *    (hidden), or longer than #Internal::kMaxWiFiSSIDLength.
 *
 *  @retval  *
 *    Otherwise, any error returned while reading the service Name.
 *
 *  @sa GetWiFiServiceSecurityLocked
 *  @sa HandleWiFiBroadcastScanCompleteLocked
 *  @sa HandleWiFiPendingScanLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::BuildWiFiScanResponseFromServicePropertiesLocked(
    GVariant * inProperties, WiFiScanResponse & outResponse) noexcept
{
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // ConnMan's "Name" for a Wi-Fi service is its SSID string. A hidden
    // network surfaces with an empty Name; rejecting those here is exactly
    // the "non-hidden" filter the broadcast scan requires.

    const char * name = nullptr;
    ReturnErrorOnFailure(GetObjectNameFromPropertiesLocked(inProperties, name));
    VerifyOrReturnError(name != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const size_t nameLen = strlen(name);
    VerifyOrReturnError(nameLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nameLen <= Internal::kMaxWiFiSSIDLength, CHIP_ERROR_INVALID_ARGUMENT);

    WiFiScanResponse response{};

    // Prefer connman's actual Security array (notably so open
    // networks report kUnencrypted rather than mis-prompting a
    // commissioner for a passphrase).  On absence or an
    // all-unrecognized array, fall back to the prior kWpa2Personal
    // proxy so behavior never regresses to an empty bitmap.

    chip::BitFlags<WiFiSecurityBitmap> security;
    if (GetWiFiServiceSecurityLocked(inProperties, security) != CHIP_NO_ERROR || !security.HasAny())
    {
        security = chip::BitFlags<WiFiSecurityBitmap>();
        security.Set(WiFiSecurityBitmap::kWpa2Personal);
    }

    response.security = security;

    memcpy(response.ssid, name, nameLen);
    response.ssidLen = static_cast<uint8_t>(nameLen);

    memset(&response.bssid, 0, sizeof(response.bssid));

    // ConnMan does not expose an introspectable channel, band, or
    // frequency for wireless networks. For now, set proxy
    // values. In the future, we may be able to get this through
    // nl80211 or the Wi-Fi backend.

    response.channel  = 0;
    response.wiFiBand = WiFiBandEnum::k5g;

    // connman virtualizes wireless signal strength (cellular and
    // Wi-Fi alike) into a qualitative 0-100 range. For Wi-Fi it
    // derives that from the supplicant's dBm in
    // plugins/wifi.c:calculate_strength():
    //
    //     strength = min(120 + dBm, 100)
    //
    // which is lossy only at the top: everything at or above -20 dBm
    // saturates to 100. Nothing on the connman D-Bus surface exposes
    // the original value.
    //
    // The Network Commissioning Cluster's 'ScanNetworksResponse'
    // carries RSSI in dBm, signed (it has no qualitative
    // representation, though the SDK-internal WiFiScanResponse does)
    // so invert the conversion. The result is exact below the
    // saturation point. Alternatively, in the future, the
    // specification and the Network Commissioning Cluster would
    // support and pass through a qualitative wireless signal
    // assessment as the SDK-internal structure allows.
    //
    // Note the clamp in GetWiFiServiceStrengthLocked is load-bearing:
    // calculate_strength() has no *lower* bound and returns unsigned
    // char, so a signal below -120 dBm would wrap. Not reachable from
    // wpa_supplicant in practice, but the getter's clamp keeps the
    // arithmetic here total.
    //
    // Strength is best-effort and cosmetic; a failed read leaves the
    // zero-initialized default. It must not gate the scan result,
    // which has already been assembled and is about to be reported
    // successful.

    int8_t strength = 0;
    RETURN_SAFELY_IGNORED GetWiFiServiceStrengthLocked(inProperties, strength);

    response.signal.type     = WirelessSignalType::kdBm;
    response.signal.strength = static_cast<int8_t>(strength - 120);

    outResponse = response;

    return CHIP_NO_ERROR;
}

/**
 *  @brief
 *    Retire an in-flight Wi-Fi connect and dispatch its Matter
 *    completion, exactly once.
 *
 *  The sole completion for `ConnectWiFiNetworkAsync`. Three steps, in
 *  an order that is not negotiable and is therefore enforced here
 *  rather than at the multiple call sites in which it is used:
 *
 *    1. Snapshot the debug text. `inDebugText` frequently aliases
 *       `mWiFiClusterConnectScanState.mSsid`, which step 2 clears.
 *
 *    2. Retire the connect state, so that any further connman signal for this
 *       service (the `failure` -> `idle` transition, a late `Agent.ReportError`,
 *       the `Service.Connect()` reply itself) finds no pending connect and
 *       drives no second, contradictory result.
 *
 *    3. Dispatch `OnConnectResult`, which may temporarily release the
 *       class lock on its fallback path and therefore must observe
 *       fully-retired state.
 *
 *  Deliberately *not* folded into #DispatchWiFiConnectFinishedLocked: the
 *  synchronous failure paths out of `ConnectWiFiNetworkAsync` must retire the
 *  connect via #ConcludeWiFiClusterConnectLocked *without* completing it,
 *  since `LinuxWiFiDriver::ConnectNetwork` completes those from the returned
 *  CHIP_ERROR.
 *
 *  @sa ConcludeWiFiClusterConnectLocked
 *  @sa DispatchWiFiConnectFinishedLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::CompleteWiFiConnectLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                       Status inStatus,
                                                                                       const CharSpan & inDebugText,
                                                                                       int32_t inReason) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "completing connect: pending=%u status=%u debug='%.*s'",
                  HasWiFiClusterConnectPendingLocked(), static_cast<unsigned>(inStatus), static_cast<int>(inDebugText.size()),
                  inDebugText.data());

    // Exactly-once, structurally. Every completion now funnels through here, so
    // the guard lives here rather than being re-derived by each caller.

    VerifyOrReturnError(HasWiFiClusterConnectPendingLocked(), CHIP_NO_ERROR);

    // 1. Snapshot before retiring: inDebugText may alias mWiFiClusterConnectScanState.mSsid.

    Internal::WiFiSSIDFixedBuffer debug;
    debug = ByteSpan(reinterpret_cast<const uint8_t *>(inDebugText.data()), inDebugText.size());

    // 2. Retire.

    ReturnErrorOnFailure(ConcludeWiFiClusterConnectLocked());

    // 3. Complete.

    return DispatchWiFiConnectFinishedLocked(inOutLock, inStatus,
                                             CharSpan(reinterpret_cast<const char *>(debug.data()), debug.size()), inReason);
}

/**
 *  @brief
 *    Conclude the Matter Network Commissioning Cluster connect: the
 *    state whose lifetime is that of the Network Commissioning
 *    Cluster `ConnectNetwork` command.
 *
 *  The peer of #ShutdownClientConnectSessionLocked. Retires only the
 *  state that the Matter Cluster completion renders meaningless (the
 *  pending gate, the association latch, the latched service error,
 *  and the connect scan state) so that any further connman signal for
 *  this service (the `failure` -> `idle` transition, a late
 *  `Agent.ReportError`, the `Service.Connect()` reply itself) finds
 *  no pending connect and drives no second, contradictory result.
 *
 *  @note
 *    Which to call: *"has the Network Commissioning Cluster been
 *    answered?"* -- if it has, this runs. Contrast
 *    #ShutdownClientConnectSessionLocked, whose test is *"can connman
 *    still call my agent?"*. The agent, pending service proxy, and
 *    credential belong to that longer-lived D-Bus scope and are
 *    deliberately *not* touched here.
 *
 *  @note
 *    Does not clear the one-shot connect callback: that is consumed by
 *    `OnConnectResult`, which #CompleteWiFiConnectLocked dispatches
 *    *after* this returns. Clearing it here would wedge the command
 *    handle.
 *
 *  @retval  #CHIP_NO_ERROR  Always.
 *
 *  @sa CompleteWiFiConnectLocked
 *  @sa ShutdownClientConnectSessionLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ConcludeWiFiClusterConnectLocked() noexcept
{
    mWiFiClusterConnectPending            = false;
    mWiFiClusterConnectAssociationStarted = false;

    mWiFiClusterConnectServiceError.reset();

    mWiFiClusterConnectScanState.Reset();

    return CHIP_NO_ERROR;
}

/**
 *  @brief
 *    Complete a Wi-Fi connect operation exactly once, deferring off
 *    the class lock.
 *
 *  Drives the Connectivity Manager `OnConnectResult` completion for a
 *  connect, preferring to defer it onto a fresh Matter event-loop
 *  iteration (via `ScheduleLambda`) so that it runs after the caller
 *  releases `mConnManMutex` and may therefore re-enter this object
 *  safely. Should that deferral fail to allocate, the completion is
 *  instead run synchronously with the lock temporarily released via
 *  #UnlockAndInvoke, so that the Network Commissioning command is
 *  always completed rather than left wedged. The debug text is copied
 *  by value up front because its backing store (a connman service
 *  "Name") may be mutated or removed before a deferred callback runs.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`; this is verified by assertion. On the
 *    fallback path the lock is released and reacquired around the
 *    synchronous completion.
 *
 *  @param[in,out]  inOutLock
 *    A reference to the mutable, held class lock, which may be
 *    temporarily released on the synchronous fallback path.
 *
 *  @param[in]      inStatus
 *    The Network Commissioning status to report.
 *
 *  @param[in]      inDebugText
 *    The debug text (typically the target SSID) to report; copied by
 *    value and truncated to #Internal::kMaxWiFiSSIDLength.
 *
 *  @param[in]      inReason
 *    The association/connect reason code to report.
 *
 *  @retval  #CHIP_NO_ERROR
 *    Always; the completion is dispatched on either the deferred or
 *    the synchronous fallback path.
 *
 *  @sa DispatchWiFiScanFinishedLocked
 *  @sa HandleServiceConnectComplete
 *  @sa HandleWiFiUnresolvedAfterScanLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::DispatchWiFiConnectFinishedLocked(
    std::unique_lock<std::mutex> & inOutLock, Status inStatus, const CharSpan & inDebugText, int32_t inReason) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    // Deferred execution must capture the debug bytes by value; inDebugText's
    // backing store (a connman service "Name" in the properties hash table)
    // may be mutated or removed before the callback runs.
    std::array<uint8_t, Internal::kMaxWiFiSSIDLength> debugBytes{};
    const size_t debugLen = std::min<size_t>(inDebugText.size(), debugBytes.size());
    if (debugLen > 0)
    {
        memcpy(debugBytes.data(), inDebugText.data(), debugLen);
    }

    // Preferred path: defer onto a fresh Matter event-loop iteration so the
    // caller's scope releases mConnManMutex before OnConnectResult runs.
    const CHIP_ERROR scheduled = DeviceLayer::SystemLayer().ScheduleLambda([this, inStatus, debugBytes, debugLen, inReason]() {
        const CharSpan debug(reinterpret_cast<const char *>(debugBytes.data()), debugLen);

        ChipLogDetail(
            DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "dispatching connect result: pending=%u status=%u debug='%.*s'",
            HasWiFiClusterConnectPendingLocked(), static_cast<unsigned>(inStatus), static_cast<int>(debug.size()), debug.data());

        mConnectivityManagerImpl->OnConnectResult(inStatus, debug, inReason);
    });

    if (scheduled != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(scheduled);

        // Fallback: the deferral could not be allocated. Complete the
        // command synchronously with the lock temporarily released,
        // so OnConnectResult may re-enter safely, rather than leaving
        // the command handle wedged.

        UnlockAndInvoke(inOutLock, [&]() -> CHIP_ERROR {
            const CharSpan debug(reinterpret_cast<const char *>(debugBytes.data()), debugLen);

            mConnectivityManagerImpl->OnConnectResult(inStatus, debug, inReason);

            return CHIP_NO_ERROR;
        });
    }

    return CHIP_NO_ERROR;
}

/**
 *  @brief
 *    Complete a Wi-Fi scan operation exactly once, deferring off the
 *    class lock.
 *
 *  Drives the Connectivity Manager `OnScanFinished` completion for a
 *  scan, preferring to defer it onto a fresh Matter event-loop
 *  iteration (via `ScheduleLambda`) so that it runs after the caller
 *  releases `mConnManMutex` and may therefore re-enter this object
 *  safely. Should that deferral fail to allocate, the completion is
 *  instead run synchronously with the lock temporarily released via
 *  #UnlockAndInvoke, so that the Network Commissioning command is
 *  always completed rather than left wedged. Ownership of
 *  `inResponses` transfers into the deferred task on the preferred
 *  path and is released in place on the fallback path. A null
 *  `inResponses` encodes not-found (a null result iterator); a
 *  non-null but empty vector encodes a successful scan that matched
 *  nothing. The debug text is copied by value because its backing
 *  store may not outlive this frame.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`; this is verified by assertion. On the
 *    fallback path the lock is released and reacquired around the
 *    synchronous completion.
 *
 *  @param[in,out]  inOutLock
 *    A reference to the mutable, held class lock, which may be
 *    temporarily released on the synchronous fallback path.
 *
 *  @param[in]      inStatus
 *    The Network Commissioning status to report.
 *
 *  @param[in]      inDebugText
 *    The debug text (typically the target SSID) to report; copied by
 *    value and truncated to #Internal::kMaxWiFiSSIDLength.
 *
 *  @param[in]      inResponses
 *    The scan results to report, ownership of which is consumed by
 *    this call. Null encodes not-found; non-null-but-empty encodes a
 *    successful empty result.
 *
 *  @retval  #CHIP_NO_ERROR
 *    Always; the completion is dispatched on either the deferred or
 *    the synchronous fallback path.
 *
 *  @sa DispatchWiFiConnectFinishedLocked
 *  @sa HandleWiFiBroadcastScanCompleteLocked
 *  @sa HandleWiFiPendingScanLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::DispatchWiFiScanFinishedLocked(
    std::unique_lock<std::mutex> & inOutLock, Status inStatus, const CharSpan & inDebugText,
    std::unique_ptr<std::vector<WiFiScanResponse>> inResponses) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    // Deferred execution must capture debug bytes by value;
    // inDebugText's backing store may not outlive this frame.

    std::array<uint8_t, Internal::kMaxWiFiSSIDLength> debugBytes{};
    const size_t debugLen = std::min<size_t>(inDebugText.size(), debugBytes.size());
    if (debugLen > 0)
    {
        memcpy(debugBytes.data(), inDebugText.data(), debugLen);
    }

    // A null vector encodes not-found (nullptr iterator); a non-null
    // but empty vector encodes a successful scan that found nothing.

    std::vector<WiFiScanResponse> * const raw = inResponses.get();

    // Preferred path: defer onto a fresh Matter event-loop iteration
    // so the caller's scope releases mConnManMutex before
    // OnScanFinished runs.

    const CHIP_ERROR scheduled = DeviceLayer::SystemLayer().ScheduleLambda([this, inStatus, debugBytes, debugLen, raw]() {
        std::unique_ptr<std::vector<WiFiScanResponse>> owned(raw);
        const CharSpan debug(reinterpret_cast<const char *>(debugBytes.data()), debugLen);
        LinuxScanResponseIterator<WiFiScanResponse> iter(owned.get());

        ChipLogDetail(DeviceLayer,
                      CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "dispatching scan result: status=%u count=%zu debug='%.*s' owned=%u",
                      static_cast<unsigned>(inStatus), (raw != nullptr) ? raw->size() : 0, static_cast<int>(debug.size()),
                      debug.data(), owned != nullptr);

        mConnectivityManagerImpl->OnScanFinished(inStatus, debug, (owned != nullptr) ? &iter : nullptr);
    });

    if (scheduled == CHIP_NO_ERROR)
    {
        RETURN_SAFELY_IGNORED inResponses.release();
    }
    else
    {
        LogErrorOnFailure(scheduled);

        // Fallback: the deferral could not be allocated. Complete the
        // command synchronously with the lock temporarily released, so
        // OnScanFinished may safely re-enter, rather than leaving the
        // command handle wedged. Nothing below aliases inResponses after
        // this returns; it is freed here.

        UnlockAndInvoke(inOutLock, [&]() -> CHIP_ERROR {
            LinuxScanResponseIterator<WiFiScanResponse> iter(inResponses.get());
            const CharSpan debug(reinterpret_cast<const char *>(debugBytes.data()), debugLen);

            mConnectivityManagerImpl->OnScanFinished(inStatus, debug, (inResponses != nullptr) ? &iter : nullptr);

            return CHIP_NO_ERROR;
        });
    }

    return CHIP_NO_ERROR;
}

/**
 *  @brief
 *    Look up the tracked-state entry for a connman service type.
 *
 *  Performs a linear search of the tracked-state table for the entry
 *  whose `mType` matches `inType` by string value. The `Locked`
 *  suffix denotes the precondition that the caller holds
 *  `mConnManMutex`; the returned pointer aliases table storage and is
 *  valid only while that lock is held. Note that an ineligible type
 *  (for example, cellular or VPN, deliberately absent from the table)
 *  is indistinguishable here from an unknown one: both yield
 *  `nullptr`.
 *
 *  @param[in]  inType
 *    The connman service "Type" value to match (for example,
 *    "ethernet"); may be `nullptr`.
 *
 *  @returns
 *    A pointer to the matching `NetworkServiceState`, or `nullptr` if
 *    `inType` is @c `nullptr` or no tracked entry matches.
 *
 *  @private
 *
 */
ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceState *
ConnectivityManagerImpl_NetworkManagementConnMan::FindNetworkServiceStateLocked(const char * inType) noexcept
{
    VerifyOrReturnValue(inType != nullptr, nullptr);

    const auto it =
        std::find_if(mNetworkServiceStates.begin(), mNetworkServiceStates.end(), [inType](const NetworkServiceState & inState) {
            return ((inState.mType != nullptr) && (strcmp(inState.mType, inType) == 0));
        });

    return ((it != mNetworkServiceStates.end()) ? &(*it) : nullptr);
}

const char * ConnectivityManagerImpl_NetworkManagementConnMan::GetInterfaceName(const char * inType) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    NetworkServiceState * const state = FindNetworkServiceStateLocked(inType);
    VerifyOrReturnValue(state != nullptr, nullptr);

    // If we have a valid interface name, return it. Otherwise,
    // perform a lazy update of the interface name based on the best
    // network service of the specified type.

    if (state->mIfName[0] != '\0')
    {
        return &state->mIfName[0];
    }
    else if (mConnManClient.mServices)
    {
        GAutoPtr<gchar> interface;

        CHIP_ERROR status = GetBestServiceInterfaceForTypeLocked(inType, mConnManClient.mServices.get(), interface);
        VerifyOrReturnValue(status == CHIP_NO_ERROR, nullptr);

        if (interface.get() != nullptr)
        {
            _MaybeSetInterfaceName(state->mDescription, interface.get(), &state->mIfName[0]);

            return &state->mIfName[0];
        }
    }

    return nullptr;
}

/**
 *  @brief
 *    Derive one IP address family's connectivity disposition from a
 *    connman service property dictionary.
 *
 *  Established-detection deliberately keys off the presence of a
 *  parseable "Address" member within the family's ("IPv4" / "IPv6")
 *  sub-dictionary rather than off the service "State": connman
 *  reaches "ready" when *either* family completes configuration, so
 *  "State" alone cannot distinguish per-family establishment.
 *
 *  @param[in]   inProperties
 *    A pointer to the service property dictionary (a{sv}).
 *
 *  @param[in]   inFamilyKey
 *    The family sub-dictionary key, one of "IPv4" or "IPv6".
 *
 *  @returns
 *    The derived connectivity disposition.
 *
 */
ConnectivityManagerImpl_NetworkManagementConnMan::NetworkServiceFamilyConnectivityState
ConnectivityManagerImpl_NetworkManagementConnMan::GetServiceFamilyConnectivityStateLocked(GVariant * inProperties,
                                                                                          const char * inFamilyKey) noexcept
{
    NetworkServiceFamilyConnectivityState retval;

    // A missing or malformed dictionary results in state connectivity
    // NetworkServiceConnectivity::kUnknown, from default
    // construction.

    VerifyOrReturnValue(inProperties != nullptr, retval);
    VerifyOrReturnValue(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), retval);

    // A structurally valid dictionary but without a parseable family
    // address results in state connectivity
    // NetworkServiceConnectivity::kLost.

    retval.mConnectivity = NetworkServiceConnectivity::kLost;

    GAutoPtr<GVariant> boxed(g_variant_lookup_value(inProperties, inFamilyKey, nullptr));
    VerifyOrReturnValue(boxed.get() != nullptr, retval);

    UnboxedVariant family(boxed.get());
    VerifyOrReturnValue(family, retval);
    VerifyOrReturnValue(g_variant_is_of_type(family.get(), G_VARIANT_TYPE_VARDICT), retval);

    const char * address = nullptr;
    const gboolean found = g_variant_lookup(family.get(), kConnManServicePropertyIPAddressKey, "&s", &address);
    VerifyOrReturnValue(found && (address != nullptr) && address[0] != '\0', retval);

    // Parse into a local so a partial write from a failed parse can't
    // escape.

    Inet::IPAddress parsed;
    VerifyOrReturnValue(Inet::IPAddress::FromString(address, parsed), retval);

    retval.mConnectivity = NetworkServiceConnectivity::kEstablished;
    retval.mAddress      = parsed;

    return retval;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetProperties(ConnManManager * inManager,
                                                                                  GVariant * inProperties,
                                                                                  const GError * inError) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    VerifyOrReturn(inProperties != nullptr);

    if (inError == nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "got %zu manager property/ies",
                        g_variant_n_children(inProperties));

        ReturnOnFailure(UpdateManagerPropertiesLocked(inProperties));

        ReturnOnFailure(HandleManagerPropertiesChangedLocked(inManager, inProperties));
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to get manager properties: %s",
                     inError ? inError->message : "unknown error");
    }
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetServices(ConnManManager * inManager, GVariant * inServices,
                                                                                const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    VerifyOrReturn(inServices != nullptr);

    if (inError == nullptr)
    {
        GHashTableIter iter;
        gpointer key   = nullptr;
        gpointer value = nullptr;

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "got %zu manager service(s)",
                        g_variant_n_children(inServices));

        ReturnOnFailure(UpdateServicesLocked(inServices));

        g_hash_table_iter_init(&iter, mConnManClient.mServices.get());

        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            const char * const path  = static_cast<const char *>(key);
            GVariant * const props   = static_cast<GVariant *>(value);
            const char * type        = nullptr;
            ConnManService * service = nullptr;

            ReturnOnFailure(UpdateServiceProxyLocked(lock, path, service));

            // Prime service-derived state (for example, interface
            // name) from the initial service enumeration (that is,
            // "big get").

            ReturnOnFailure(GetObjectTypeFromPropertiesLocked(props, type));

            ReturnOnFailure(HandleServicePropertiesChangedLocked(lock, G_DBUS_PROXY(service), path, type, props));
        }
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to get manager services: %s",
                     inError ? inError->message : "unknown error");
    }
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerGetTechnologies(ConnManManager * inManager,
                                                                                    GVariant * inTechnologies,
                                                                                    const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    VerifyOrReturn(inTechnologies != nullptr);

    if (inError == nullptr)
    {
        GHashTableIter iter;
        gpointer key   = nullptr;
        gpointer value = nullptr;

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "got %zu manager technology/ies",
                        g_variant_n_children(inTechnologies));

        ReturnOnFailure(UpdateTechnologiesLocked(inTechnologies));

        g_hash_table_iter_init(&iter, mConnManClient.mTechnologies.get());

        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            const char * const path        = static_cast<const char *>(key);
            GVariant * const props         = static_cast<GVariant *>(value);
            const char * type              = nullptr;
            ConnManTechnology * technology = nullptr;

            ReturnOnFailure(UpdateTechnologyProxyLocked(lock, path, technology));

            ReturnOnFailure(GetObjectTypeFromPropertiesLocked(props, type));

            ReturnOnFailure(HandleTechnologyPropertiesChangedLocked(lock, G_DBUS_PROXY(technology), path, type, props));
        }
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to get manager technologies: %s",
                     inError ? inError->message : "unknown error");
    }
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
    VerifyOrReturnError(inManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inMaybeVariant != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    UnboxedVariant value(inMaybeVariant);
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(HandleManagerPropertyChangedLocked(CONN_MAN_MANAGER(inManager), inKey, value.get()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleManagerPropertyChangedLocked(ConnManManager * inManager,
                                                                                                const char * inKey,
                                                                                                GVariant * inValue) noexcept
{
    VerifyOrReturnError(inManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inValue != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "manager \"%s\" property changed", inKey);

    return CHIP_NO_ERROR;
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
    VerifyOrDie(inOutLock.owns_lock());

    CHIP_ERROR status   = CHIP_NO_ERROR;
    const auto logGuard = ScopeExit([&status]() {
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer,
                         CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to handle changed services:%" CHIP_ERROR_FORMAT,
                         status.Format());
        }
    });

    VerifyOrReturn(inManager != nullptr, status = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturn(inServicesChanged != nullptr, status = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturn(g_variant_is_of_type(inServicesChanged, G_VARIANT_TYPE("a(oa{sv})")), status = CHIP_ERROR_INVALID_ARGUMENT);

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
    VerifyOrReturn(status == CHIP_NO_ERROR);

    GVariantIter iter;
    const char * path = nullptr;
    GVariant * props  = nullptr;
    g_variant_iter_init(&iter, inServicesChanged);
    while (g_variant_iter_next(&iter, "(&o@a{sv})", &path, &props))
    {
        ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: %s has %zu properties", __func__, path,
                      g_variant_n_children(props));

        ConnManService * service = nullptr;
        status                   = UpdateServiceProxyLocked(inOutLock, path, service);
        VerifyOrReturn(status == CHIP_NO_ERROR);

        // Result intentionally ignored; a null `current` is caught by
        // the type lookup below.

        GVariant * current = nullptr;
        RETURN_SAFELY_IGNORED GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path, current);

        // Funnel the changed service properties through the same
        // handler used for per-service property changed signals.

        const char * type = nullptr;
        status            = GetObjectTypeFromPropertiesLocked(current, type);
        VerifyOrReturn(status == CHIP_NO_ERROR);

        status = HandleServicePropertiesChangedLocked(inOutLock, G_DBUS_PROXY(service), path, type, props);
        VerifyOrReturn(status == CHIP_NO_ERROR);
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

        if (removed > 0)
        {
            RETURN_SAFELY_IGNORED UpdateNetworkServiceConnectivityLocked();
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

    status = HandleTechnologyPropertiesChangedLocked(lock, G_DBUS_PROXY(technology), inPath, type, inProperties);
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
    if ((status == CHIP_NO_ERROR))
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
    VerifyOrReturnError(inDescription != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inObjectPropertiesChangedAnyLockedMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_INVALID_ARGUMENT);

    const size_t count = g_variant_n_children(inProperties);
    if (count > 0)
    {
        GVariantIter iter;
        const char * key = nullptr;
        GAutoPtr<GVariant> boxed;

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%zu %s property/ies changed", count, inDescription);

        g_variant_iter_init(&iter, inProperties);

        while (g_variant_iter_next(&iter, "{&s@v}", &key, &boxed.GetReceiver()))
        {
            VerifyOrReturnError(key != nullptr, CHIP_ERROR_INTERNAL);
            VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_INTERNAL);

            ReturnErrorOnFailure((this->*inObjectPropertiesChangedAnyLockedMethod)(inProxy, key, boxed.get()));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleObjectPropertiesChangedLocked(
    std::unique_lock<std::mutex> & inOutLock, const char * inDescription, GDBusProxy * inProxy, const char * inPath,
    const char * inType, GVariant * inProperties,
    TypedObjectPropertiesChangedAnyLockedMethod inTypedObjectPropertiesChangedAnyLockedMethod) noexcept
{
    VerifyOrReturnError(inDescription != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inTypedObjectPropertiesChangedAnyLockedMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_INVALID_ARGUMENT);

    const size_t count = g_variant_n_children(inProperties);
    if (count > 0)
    {
        GVariantIter iter;
        const char * key = nullptr;
        GAutoPtr<GVariant> boxed;

        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%zu %s %s property/ies changed", count, inType,
                        inDescription);

        g_variant_iter_init(&iter, inProperties);

        while (g_variant_iter_next(&iter, "{&s@v}", &key, &boxed.GetReceiver()))
        {
            VerifyOrReturnError(key != nullptr, CHIP_ERROR_INTERNAL);
            VerifyOrReturnError(boxed.get() != nullptr, CHIP_ERROR_INTERNAL);

            ReturnErrorOnFailure(
                (this->*inTypedObjectPropertiesChangedAnyLockedMethod)(inOutLock, inProxy, inPath, inType, key, boxed.get()));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleObjectPropertyChangedAnyLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey,
    GVariant * inMaybeVariant, TypedObjectPropertyChangedLockedMethod inTypedObjectPropertyChangedLockedMethod) noexcept
{
    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inMaybeVariant != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inTypedObjectPropertyChangedLockedMethod != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    UnboxedVariant value(inMaybeVariant);
    VerifyOrReturnError(value, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure((this->*inTypedObjectPropertyChangedLockedMethod)(inOutLock, inProxy, inPath, inType, inKey, value.get()));

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleServiceConnectComplete(GDBusProxy * inService,
                                                                                    const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    VerifyOrReturn(inService != nullptr);

    // connman has replied to Service.Connect() and will request nothing
    // further for it; retire the D-Bus-call-scoped state unconditionally,
    // irrespective of which path drove the Matter completion.

    auto teardown = ScopeExit([&]() { ShutdownClientConnectSessionLocked(lock); });

    const char * path = nullptr;
    ReturnOnFailure(GetObjectPathFromProxyLocked(inService, path));

    GVariant * props = nullptr;
    ReturnOnFailure(GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path, props));

    const char * type = nullptr;
    ReturnOnFailure(GetObjectTypeFromPropertiesLocked(props, type));

    CharSpan debug_text;
    const char * name = nullptr;
    if (GetObjectNameFromPropertiesLocked(props, name) == CHIP_NO_ERROR)
    {
        debug_text = CharSpan::fromCharString(name);
    }

    // If the service state machine already concluded this connect (at "ready"
    // or at "failure") the Matter completion has been dispatched with a
    // *specific* status and the connect state retired. connman emits those
    // PropertyChanged signals ahead of replying to Connect(), so this is the
    // ordinary path. Do not dispatch a second, less specific result -- and do
    // not log a misleading failure for a connect that already succeeded.

    VerifyOrReturn(HasWiFiClusterConnectPendingLocked(),
                   ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s service %s connect already concluded",
                                   type, path));

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s service %s connect done", type, path);

    // Fallback: Connect() completed without the service ever reaching a
    // terminal state. This is a synchronous rejection (invalid arguments,
    // already connected, unsupported) or a D-Bus transport failure: not an
    // association failure, and connman told us nothing more specific.

    if (inError != nullptr)
    {
        ChipLogError(DeviceLayer,
                     CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to connect %s service: %s (%" CHIP_ERROR_FORMAT ")", type,
                     inError->message, MapClientError(inError).Format());
    }

    LogErrorOnFailure(
        CompleteWiFiConnectLocked(lock, (inError == nullptr) ? Status::kSuccess : Status::kUnknownError, debug_text, 0));
}

/**
 *  @brief
 *    Register the connman agent and issue an asynchronous
 *    `Service.Connect()` for a Wi-Fi network service.
 *
 *  @note
 *    Deliberately does *not* dispatch a Matter completion on failure.
 *    Its two callers have opposite completion contracts:
 *    `ConnectWiFiNetworkAsync`'s failure is completed by
 *    `LinuxWiFiDriver::ConnectNetwork` from the returned CHIP_ERROR,
 *    whereas #HandleWiFiPendingConnectLocked is reached from
 *    #HandleTechnologyScanComplete, which only logs, and so must
 *    complete the command itself.
 *
 *  @sa ShutdownClientConnectSessionLocked
 *  @sa HandleWiFiPendingConnectLocked
 *
 *  @private
 *
 */
CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::HandleServiceConnectRequestLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                    ConnManService * inService) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());
    VerifyOrReturnError(inService != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Matter's AddOrUpdateWiFiNetwork/ConnectNetwork is
    // authoritative: the commissioner is the source of truth for the
    // credential. connman consults the agent only when the service
    // has no passphrase -- service_connect() short-circuits on a
    // cached one, and the sticky invalid-key error that would
    // otherwise force a re-prompt is cleared by connman itself on the
    // failure -> idle transition. Unconditionally remove any cached
    // provisioning so that service_connect() is guaranteed to yield
    // -ENOKEY and drive Agent.RequestInput, which is the only
    // sanctioned path by which we may supply the credential.
    //
    // connman returns net.connman.Error.NotSupported (mapped to
    // EOPNOTSUPP) when the service was never provisioned (not
    // favorite, no cached passphrase); which is the state we are
    // trying to reach. Treat it as success.

    ReturnErrorOnFailure(ServiceRemoveLocked(inOutLock, inService).NoErrorIf(ChipError(ChipError::Range::kPOSIX, EOPNOTSUPP)));

    // On any failure below, `Service.Connect()` either was not issued or will
    // not complete; unwind the D-Bus-call-scoped state. Disarmed on success,
    // where HandleServiceConnectComplete assumes that responsibility.

    auto cleanup = ScopeExit([&]() { ShutdownClientConnectSessionLocked(inOutLock); });

    ReturnErrorOnFailure(ManagerRegisterAgentLocked(inOutLock, MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH));

    mConnManAgentServer.mPendingService.reset(g_object_ref(inService));

    ReturnErrorOnFailure(ServiceConnectLocked(inOutLock, inService));

    // Success; disarm the scoped clean-up.

    cleanup.release();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertiesChangedLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType,
    GVariant * inProperties) noexcept
{
    static const char * const kDescription = "service";

    return HandleObjectPropertiesChangedLocked(
        inOutLock, kDescription, inProxy, inPath, inType, inProperties,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedAnyLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChanged(ConnManService * inService, const char * inKey,
                                                                                    GVariant * inValue) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
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

    status = HandleServicePropertyChangedAnyLocked(lock, G_DBUS_PROXY(inService), path, type, inKey, inValue);
    ReturnOnFailure(status);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey,
    GVariant * inValue) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inValue != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s service \"%s\" property changed", inType, inKey);

    if (strcmp(inKey, kConnManServicePropertyEthernetKey) == 0)
    {
        GAutoPtr<gchar> interface;

        ReturnErrorOnFailure(GetServiceInterfaceFromEthernetLocked(inValue, interface));

        ReturnErrorOnFailure(MaybeSetInterfaceNameLocked(inType, interface.get()));
    }
    else if ((FindNetworkServiceStateLocked(inType) != nullptr) &&
             ((strcmp(inKey, kConnManServicePropertyIPv4Key) == 0) || (strcmp(inKey, kConnManServicePropertyIPv6Key) == 0) ||
              (strcmp(inKey, kConnManServicePropertyStateKey) == 0)))
    {
        // Any change to a tracked service type's address
        // configuration (or to its state, which gates best-of-type
        // selection) may alter Internet connectivity; recompute and
        // debounce.

        ReturnErrorOnFailure(UpdateNetworkServiceConnectivityLocked());
    }

    // Independently of, and in addition to, the connectivity
    // recomputation above, "Error" and "State" changes on the service
    // backing an in-flight Network Commissioning connect drive that
    // connect's Matter completion. These are deliberately NOT chained
    // onto the preceding 'else if': the connectivity concern is about
    // *any* tracked service type, whereas this is about *the* pending
    // service, and both must run.

    if (strcmp(inKey, kConnManServicePropertyErrorKey) == 0)
    {
        ReturnErrorOnFailure(HandleWiFiPendingConnectServiceErrorChangedLocked(inPath, inType, inValue));
    }
    else if (strcmp(inKey, kConnManServicePropertyStateKey) == 0)
    {
        ReturnErrorOnFailure(HandleWiFiPendingConnectServiceStateChangedLocked(inOutLock, inPath, inType, inValue));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedAnyLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inService, const char * inPath, const char * inType, const char * inKey,
    GVariant * inMaybeVariant) noexcept
{
    return HandleObjectPropertyChangedAnyLocked(
        inOutLock, G_DBUS_PROXY(inService), inPath, inType, inKey, inMaybeVariant,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleServicePropertyChangedLocked);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertiesChangedLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType,
    GVariant * inProperties) noexcept
{
    static const char * const kDescription = "technology";

    return HandleObjectPropertiesChangedLocked(
        inOutLock, kDescription, inProxy, inPath, inType, inProperties,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedAnyLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChanged(ConnManTechnology * inTechnology,
                                                                                       const char * inKey,
                                                                                       GVariant * inValue) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
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

    status = HandleTechnologyPropertyChangedAnyLocked(lock, G_DBUS_PROXY(inTechnology), path, type, inKey, inValue);
    ReturnOnFailure(status);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey,
    GVariant * inValue) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inProxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inValue != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s technology \"%s\" property changed", inType, inKey);

    if (strcmp(inKey, kConnManTechnologyPropertyConnectedKey) == 0)
    {
        VerifyOrReturnError(g_variant_is_of_type(inValue, G_VARIANT_TYPE_BOOLEAN), CHIP_ERROR_WRONG_KEY_TYPE);

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
        VerifyOrReturnError(g_variant_is_of_type(inValue, G_VARIANT_TYPE_BOOLEAN), CHIP_ERROR_WRONG_KEY_TYPE);

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

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedAnyLocked(
    std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inTechnology, const char * inPath, const char * inType,
    const char * inKey, GVariant * inMaybeVariant) noexcept
{
    return HandleObjectPropertyChangedAnyLocked(
        inOutLock, inTechnology, inPath, inType, inKey, inMaybeVariant,
        &ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyPropertyChangedLocked);
}

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleTechnologyScanComplete(GDBusProxy * inTechnology,
                                                                                    const GError * inError) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);
    const char * type = nullptr;

    VerifyOrReturn(inTechnology != nullptr);

    ReturnOnFailure(GetObjectTypeFromProxyLocked(G_DBUS_PROXY(inTechnology), mConnManClient.mTechnologies.get(), type));

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s technology scan done", type);

    if (inError == nullptr)
    {
        if (strcmp(type, kConnManTechnologyPropertyTypeWiFiValue) == 0)
        {
            ReturnOnFailure(HandleWiFiScanCompleteLocked(lock, CONN_MAN_TECHNOLOGY(inTechnology)));
        }
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to scan %s technology: %s", type,
                     inError->message);
    }
}

/**
 *  @brief
 *    Complete a broadcast (non-directed) Wi-Fi scan from the current
 *    service set.
 *
 *  Reached from #HandleWiFiScanCompleteLocked when the active scan is
 *  live but carries no target SSID. Enumerates every currently-known
 *  connman Wi-Fi service and accumulates one `WiFiScanResponse` per
 *  non-hidden entry via
 *  #BuildWiFiScanResponseFromServicePropertiesLocked.  Unlike the
 *  directed path, an empty result set is reported as success rather
 *  than not-found, and there is no rescan or retry: the scan has
 *  already run, and this reports whatever connman presently
 *  knows. De-duplication is implicit, since connman collapses the
 *  BSSIDs of a shared SSID/security into one service.  The
 *  active-scan state is reset before dispatch to restore the idle
 *  invariant (`mCount == 0`) across the possible lock release inside
 *  #DispatchWiFiScanFinishedLocked.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`; this is verified by assertion.
 *
 *  @param[in,out]  inOutLock
 *    A reference to the mutable, held class lock, passed through to
 *    #DispatchWiFiScanFinishedLocked.
 *
 *  @param[in]      inTechnology
 *    A pointer to the completed connman Wi-Fi technology. Validated
 *    but otherwise unused, since a broadcast scan never rescans.
 *
 *  @retval  #CHIP_NO_ERROR
 *    If the results were assembled and dispatched.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT
 *    If `inTechnology` is null.
 *
 *  @retval  #CHIP_ERROR_NO_MEMORY
 *    If the result vector could not be allocated.
 *
 *  @sa BuildWiFiScanResponseFromServicePropertiesLocked
 *  @sa DispatchWiFiScanFinishedLocked
 *  @sa HandleWiFiScanCompleteLocked
 *
 *  @private
 *
 */
CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiBroadcastScanCompleteLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                        ConnManTechnology * inTechnology) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());
    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Enumerate every currently-known Wi-Fi service and accumulate a
    // scan response for each non-hidden one. A broadcast scan reports
    // whatever connman knows after the just-completed technology
    // scan; unlike the directed path, an empty result set is a
    // success, not a not-found, and there is no rescan or
    // retry. De-duplication is implicit: connman collapses BSSIDs of
    // a common SSID/security into one service, so at most one
    // response per (SSID, security) is produced.

    std::unique_ptr<std::vector<WiFiScanResponse>> responses(new (std::nothrow) std::vector<WiFiScanResponse>());
    VerifyOrReturnError(responses != nullptr, CHIP_ERROR_NO_MEMORY);

    GHashTableIter iter;
    gpointer key   = nullptr;
    gpointer value = nullptr;

    g_hash_table_iter_init(&iter, mConnManClient.mServices.get());
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        GVariant * const props = static_cast<GVariant *>(value);
        if ((props == nullptr) || !g_variant_is_of_type(props, G_VARIANT_TYPE_VARDICT))
        {
            continue;
        }

        const char * type = nullptr;
        if ((GetObjectTypeFromPropertiesLocked(props, type) != CHIP_NO_ERROR) || (type == nullptr) ||
            (strcmp(type, kConnManServicePropertyTypeWiFiValue) != 0))
        {
            continue;
        }

        WiFiScanResponse response;
        if (BuildWiFiScanResponseFromServicePropertiesLocked(props, response) != CHIP_NO_ERROR)
        {
            // Skips hidden (empty-Name), oversized, and malformed services.

            continue;
        }

        responses->push_back(response);
    }

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "broadcast scan found %zu Wi-Fi network(s)",
                    responses->size());

    // Restore the idle invariant before dispatching: a broadcast scan is
    // single-shot, so clear the count (the liveness sentinel). The SSID is
    // already empty.

    mWiFiActiveScanState.Reset();

    return DispatchWiFiScanFinishedLocked(inOutLock, NetworkCommissioning::Status::kSuccess, CharSpan(), std::move(responses));
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiPendingConnectLocked(
    std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology, WiFiScanState & inOutScanState) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ConnManService * const service = GetWiFiServiceProxyFromSsidLocked(
        inOutScanState.mSsid.span(), mConnManClient.mServiceProxies.get(), mConnManClient.mServices.get());
    if (service != nullptr)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting to '%.*s' after %zu scan(s)...",
                        static_cast<int>(inOutScanState.mSsid.size()), inOutScanState.mSsid.data(), inOutScanState.mCount);

        const CHIP_ERROR status = HandleServiceConnectRequestLocked(inOutLock, service);
        if (status == CHIP_NO_ERROR)
        {
            // The scan that this connect was waiting on has served
            // its purpose; clear the liveness sentinel so a
            // subsequent technology scan completion does not re-enter
            // and issue a second Service.Connect() (and a second
            // agent registration) atop the live one. The SSID is
            // retained: it is still the completion's debug text.

            inOutScanState.mCount = 0;
        }
        else
        {
            // This path is reached from HandleTechnologyScanComplete, which
            // merely logs the returned error. No caller will complete the
            // Network Commissioning command, so complete it here or the
            // commissioner waits out the fail-safe.

            const CharSpan debug(reinterpret_cast<const char *>(inOutScanState.mSsid.data()), inOutScanState.mSsid.size());

            LogErrorOnFailure(CompleteWiFiConnectLocked(inOutLock, Status::kUnknownError, debug, status.GetValue()));
        }

        return status;
    }

    return HandleWiFiUnresolvedAfterScanLocked(inOutLock, inTechnology, inOutScanState, kWiFiConnectScanLimit,
                                               "could not connect to", WiFiScanCompletionKind::kConnect);
}

/**
 *  @brief
 *    Latch a connman network service "Error" property change for the
 *    service backing an in-flight Wi-Fi connect.
 *
 *  connman emits `PropertyChanged("Error")` before it emits
 *  `PropertyChanged("State" = "failure")`, and invokes
 *  `Agent.ReportError()` only after both. This handler is therefore
 *  the authoritative, race-free point at which the failure cause
 *  becomes known; the failure state transition merely consumes what
 *  is latched here.
 *
 *  @note
 *    connman signals a *cleared* error as the empty string rather than
 *    by omitting the property; that is handled as a latch reset.
 *
 *  @param[in]  inPath
 *    The service object path whose property changed.
 *
 *  @param[in]  inType
 *    The service type; only Wi-Fi participates.
 *
 *  @param[in]  inValue
 *    The new "Error" property value, of type 's'.
 *
 *  @retval  #CHIP_NO_ERROR
 *    On success, or when the change is not for the pending service.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT
 *    If @a inValue is not a string.
 *
 *  @sa HandleWiFiPendingConnectServiceStateChangedLocked
 *  @sa MapServiceError
 *
 *  @private
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiPendingConnectServiceErrorChangedLocked(
    const char * inPath, const char * inType, GVariant * inValue) noexcept
{
    // If the network service type is not Wi-Fi, ignore it.

    VerifyOrReturnError(strcmp(inType, kConnManServicePropertyTypeWiFiValue) == 0, CHIP_NO_ERROR);

    // If the change is not for a Wi-Fi service we are attempting to
    // connect to, ignore it.

    VerifyOrReturnError(IsWiFiPendingConnectServicePathLocked(inPath), CHIP_NO_ERROR);

    // If the variant is of the wrong type (not 's'), it's an invalid
    // argument.

    VerifyOrReturnError(g_variant_is_of_type(inValue, G_VARIANT_TYPE_STRING), CHIP_ERROR_INVALID_ARGUMENT);

    const char * const error = g_variant_get_string(inValue, nullptr);

    if ((error == nullptr) || (*error == '\0'))
    {
        mWiFiClusterConnectServiceError.reset();
    }
    else
    {
        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting service \"%s\" error is now \"%s\"", inPath,
                     error);

        mWiFiClusterConnectServiceError = MapServiceError(error);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiPendingConnectServiceStateChangedLocked(
    std::unique_lock<std::mutex> & inOutLock, const char * inPath, const char * inType, GVariant * inValue) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    // If the network service type is not Wi-Fi, ignore it.

    VerifyOrReturnError(strcmp(inType, kConnManServicePropertyTypeWiFiValue) == 0, CHIP_NO_ERROR);

    // Ensure that the Wi-Fi network service is one we are actually
    // connecting to. Only the service backing the pending connect
    // drives the completion. connman multiplexes PropertyChanged
    // across every service it knows about; a neighboring AP drifting
    // out of range must not complete our `ConnectWiFiNetwork*Async`
    // command.

    VerifyOrReturnError(IsWiFiPendingConnectServicePathLocked(inPath), CHIP_NO_ERROR);

    // Absent an in-flight connect there is no completion to drive. This is
    // what makes the handler idempotent across the ready -> online
    // transition and immune to the failure -> idle transition that connman
    // induces once the error is retired.

    VerifyOrReturnError(HasWiFiClusterConnectPendingLocked(), CHIP_NO_ERROR);

    // If the variant is of the wrong type (not 's'), it's an invalid
    // argument.

    VerifyOrReturnError(g_variant_is_of_type(inValue, G_VARIANT_TYPE_STRING), CHIP_ERROR_INVALID_ARGUMENT);

    const char * const state = g_variant_get_string(inValue, nullptr);
    VerifyOrReturnError(state != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "connecting service \"%s\" state is now \"%s\"", inPath,
                    state);

    WiFiDiagnosticsDelegate * const delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();

    if (strcmp(state, kConnManServicePropertyStateAssociationValue) == 0)
    {
        mWiFiClusterConnectAssociationStarted = true;
    }
    else if (strcmp(state, kConnManServicePropertyStateConfigurationValue) == 0)
    {
        // Association and the 4-way handshake have both succeeded; L2 is up
        // and DHCP/SLAAC is running. Report the link, but do not conclude:
        // dhcp-failed is still reachable from here.

        if (delegate != nullptr)
        {
            ReturnLogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda(
                [delegate]() { delegate->OnConnectionStatusChanged(static_cast<uint8_t>(ConnectionStatusEnum::kConnected)); }));
        }
    }
    else if ((strcmp(state, kConnManServicePropertyStateReadyValue) == 0) ||
             (strcmp(state, kConnManServicePropertyStateOnlineValue) == 0))
    {
        // "ready" means IPv4 and/or IPv6 is configured, which is precisely
        // Matter's ConnectNetwork contract. Deliberately do NOT hold out for
        // "online": connman's Internet reachability probe is strictly
        // stronger than Matter requires and will not fire on an
        // egress-filtered or captive-portal network that is nonetheless
        // perfectly commissionable. Note also that when a higher-priority
        // service is already the default, this service never transitions
        // ready -> online at all.
        //
        // The HasWiFiClusterConnectPendingLocked() gate above makes
        // the subsequent "online" a no-op on the networks where it
        // does arrive.

        const CharSpan debug(reinterpret_cast<const char *>(mWiFiClusterConnectScanState.mSsid.data()),
                             mWiFiClusterConnectScanState.mSsid.size());
        ReturnErrorOnFailure(CompleteWiFiConnectLocked(inOutLock, Status::kSuccess, debug, 0));

        NotifyWiFiConnectivityChange(kConnectivity_Established);
    }
    else if (strcmp(state, kConnManServicePropertyStateFailureValue) == 0)
    {
        // The cause was latched by
        // `HandleWiFiPendingConnectServiceErrorChangedLocked` from
        // the PropertyChanged("Error") that connman emits ahead of
        // this transition. A failure state with no latched error
        // should not happen; report it honestly rather than silently
        // succeeding.

        const ServiceError mapped = mWiFiClusterConnectServiceError.value_or(kServiceErrorUnknown);

        if (!mapped.mIsTerminal)
        {
            // online-check-failed: the service reached at least "ready", so
            // Matter's contract is already satisfied and this is not a
            // connect failure. It should not arrive as a *failure* state,
            // but tolerate it rather than fabricating a terminal error.

            ChipLogProgress(DeviceLayer,
                            CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "non-terminal service error; ignoring failure state");

            return CHIP_NO_ERROR;
        }

        if (mapped.mIsAssociationFailure && mWiFiClusterConnectAssociationStarted && (delegate != nullptr))
        {
            const uint8_t cause   = static_cast<uint8_t>(mapped.mCause);
            const uint16_t status = mapped.mAssociationStatus;

            ReturnLogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda(
                [delegate, cause, status]() { delegate->OnAssociationFailureDetected(cause, status); }));
        }

        if (delegate != nullptr)
        {
            ReturnLogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda(
                [delegate]() { delegate->OnConnectionStatusChanged(static_cast<uint8_t>(ConnectionStatusEnum::kNotConnected)); }));
        }

        const CharSpan debug(reinterpret_cast<const char *>(mWiFiClusterConnectScanState.mSsid.data()),
                             mWiFiClusterConnectScanState.mSsid.size());
        ReturnErrorOnFailure(CompleteWiFiConnectLocked(inOutLock, mapped.mStatus, debug, 0));

        NotifyWiFiConnectivityChange(kConnectivity_Lost);
    }

    // "idle" and "disconnect" are intentionally unhandled
    // here. connman reaches them both on the way *into* a connect and
    // on the way out of a retired failure; neither is a completion for
    // an in-flight connect, and treating them as one would deliver a
    // second, contradictory result to the Network Commissioning
    // cluster. A connect that stalls short of a completion is the job
    // of the connect timeout, not of this handler.

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiPendingScanLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                         ConnManTechnology * inTechnology,
                                                                                         WiFiScanState & inOutScanState) noexcept
{
    GAutoPtr<GVariant> properties;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Directed scan: resolve the target SSID against the current service set.

    retval = GetWiFiServicePropertiesFromSsidLocked(inOutScanState.mSsid.span(), mConnManClient.mServices.get(), properties);

    ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "directed scan for '%.*s': %s",
                  static_cast<int>(inOutScanState.mSsid.size()), inOutScanState.mSsid.data(),
                  (retval == CHIP_NO_ERROR) ? "resolved" : "unresolved");

    if (retval == CHIP_NO_ERROR)
    {
        std::unique_ptr<std::vector<WiFiScanResponse>> services(new (std::nothrow) std::vector<WiFiScanResponse>());
        VerifyOrReturnError(services != nullptr, CHIP_ERROR_NO_MEMORY);

        WiFiScanResponse response;
        ReturnErrorOnFailure(BuildWiFiScanResponseFromServicePropertiesLocked(properties.get(), response));
        services->push_back(response);

        // Snapshot the SSID for the debug text before the reset; the dispatch
        // helpers copy these bytes by value, so the reset cannot race the
        // deferred callback.

        const Internal::WiFiSSIDFixedBuffer ssid = inOutScanState.mSsid;
        const CharSpan debug(reinterpret_cast<const char *>(ssid.data()), ssid.size());

        // Directed scan resolved; reset the SSID and scan count.
        //
        // Clear the operation's scan state BEFORE dispatching the
        // completion result; the snapshot above keeps the reported SSID
        // stable across the reset.

        inOutScanState.Reset();

        return DispatchWiFiScanFinishedLocked(inOutLock, Status::kSuccess, debug, std::move(services));
    }

    // Not found: hand off to the shared give-up-versus-retry arbiter.

    return HandleWiFiUnresolvedAfterScanLocked(inOutLock, inTechnology, inOutScanState, kWiFiActiveScanLimit,
                                               "no Wi-Fi network found matching", WiFiScanCompletionKind::kScan);
}

/**
 *  @brief
 *    Dispatch a completed Wi-Fi technology scan to its pending
 *    connect and/or scan handlers.
 *
 *  Reached from #HandleTechnologyScanComplete once a connman Wi-Fi
 *  technology scan finishes. A single scan may satisfy an in-flight
 *  connect, an in-flight `ScanNetworks`, or neither (an unsolicited
 *  background scan); the first two are handled independently and in
 *  order, and the third is a no-op. Liveness is keyed off each
 *  operation's nonzero scan count, *not* off SSID emptiness: a
 *  broadcast scan legitimately carries an empty SSID and must still
 *  be completed, or the Network Commissioning command handle wedges.
 *  For a live active scan, SSID emptiness then selects between the
 *  directed (#HandleWiFiPendingScanLocked) and broadcast
 *  (#HandleWiFiBroadcastScanCompleteLocked) completion paths.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`; this is verified by assertion. The lock
 *    is passed through to the pending-operation handlers, which may
 *    temporarily release it.
 *
 *  @param[in,out]  inOutLock
 *    A reference to the mutable, held class lock guarding the scan
 *    state machine.
 *
 *  @param[in]      inTechnology
 *    A pointer to the completed connman Wi-Fi network technology.
 *
 *  @retval  #CHIP_NO_ERROR
 *    If any pending operations were dispatched, or none were pending.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT
 *    If `inTechnology` is null.
 *
 *  @retval  *
 *    Otherwise, any error returned by a pending-operation handler.
 *
 *  @sa HandleWiFiPendingConnectLocked
 *  @sa HandleWiFiPendingScanLocked
 *  @sa HandleWiFiBroadcastScanCompleteLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiScanCompleteLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                          ConnManTechnology * inTechnology) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // A Wi-Fi scan may have been initiated due to an active, directed
    // scan for a specific SSID, from a service connect for a specific
    // SSID, or as just a general passive, broadcast background
    // scan. We handle those first two cases independently and in
    // parallel. There is nothing to do for the third case.
    //
    // A nonzero active-scan count (not the emptiness of
    // mWiFiActiveScanState.mSsid) is the liveness sentinel for an
    // outstanding StartWiFiScan. The SSID's emptiness is instead the
    // (correct) directed-versus-broadcast selector: a broadcast scan
    // legitimately carries an empty SSID and must still be completed,
    // or the Network Commissioning command initiator handle wedges.

    // If there is a pending asynchronous connect that required a
    // scan, handle it.

    if (mWiFiClusterConnectScanState.IsActive())
    {
        ReturnErrorOnFailure(HandleWiFiPendingConnectLocked(inOutLock, inTechnology, mWiFiClusterConnectScanState));
    }

    // If there is a pending asynchronous scan, handle it.

    if (mWiFiActiveScanState.IsActive())
    {
        if (mWiFiActiveScanState.IsDirected())
        {
            ReturnErrorOnFailure(HandleWiFiPendingScanLocked(inOutLock, inTechnology, mWiFiActiveScanState));
        }
        else
        {
            ReturnErrorOnFailure(HandleWiFiBroadcastScanCompleteLocked(inOutLock, inTechnology));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiScanRetryLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                       ConnManTechnology * inTechnology,
                                                                                       const char * inReason,
                                                                                       WiFiScanState & inOutScanState) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inReason != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s SSID '%.*s' after %zu scan(s): retrying...", inReason,
                    static_cast<int>(inOutScanState.mSsid.size()), inOutScanState.mSsid.data(), inOutScanState.mCount);

    // Increment the scan count.

    inOutScanState.mCount++;

    // Attempt to scan, again, for the desired network service.

    ReturnErrorOnFailure(TechnologyScanLocked(inOutLock, inTechnology));

    return CHIP_NO_ERROR;
}

/**
 *  @brief
 *    Arbitrate between retrying a Wi-Fi scan and terminally reporting
 *    an unresolved SSID.
 *
 *  Invoked from a pending connect or active-scan completion once the
 *  target SSID remains absent from the connman service set after a
 *  scan. This is the shared give-up-versus-retry policy for both
 *  callers: if the scan budget is exhausted (`inOutScanState.mCount`
 *  has reached `inScanLimit`), the SSID is snapshotted for the debug
 *  text, the scan state is reset, and the caller-specific completion is
 *  completed exactly once through the corresponding dispatch
 *  primitive; otherwise, another scan is armed via
 *  #HandleWiFiScanRetryLocked and its status is returned.
 *
 *  The completion disposition is selected by an `inKind` tag rather
 *  than a caller-supplied callback so that the give-up bookkeeping
 *  (logging, scan-state reset) and the completion dispatch both live
 *  here once, and so that dispatch routes through
 *  #DispatchWiFiScanFinishedLocked or #DispatchWiFiConnectFinishedLocked,
 *  which guarantee exactly-once completion even under scheduler
 *  allocation failure. The two callers differ only in which delegate
 *  they ultimately drive (`OnScanFinished` versus `OnConnectResult`)
 *  and in the advisory error they propagate upward; the `OnScan`/
 *  `OnConnect` completion is authoritative, and the returned error is
 *  logged, not acted upon, by #HandleTechnologyScanComplete. The SSID
 *  is snapshotted before the reset because the dispatch primitives
 *  copy the debug bytes by value into a deferred task that must not
 *  alias `inOutScanState.mSsid`.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`; this is verified by assertion. The lock
 *    is passed through to #HandleWiFiScanRetryLocked and the dispatch
 *    primitives, which may temporarily release it.
 *
 *  @param[in,out]  inOutLock
 *    A reference to the mutable, held class lock guarding the
 *    critical members touched by the scan state machine.
 *
 *  @param[in]      inTechnology
 *    A pointer to the connman Wi-Fi network technology to rescan on
 *    the non-terminal path.
 *
 *  @param[in,out]  inOutScanState
 *    A reference to the mutable scan operation state (target SSID and
 *    scan count). On the terminal path its SSID is snapshotted and
 *    the state is reset; on the retry path #HandleWiFiScanRetryLocked
 *    increments its count.
 *
 *  @param[in]      inScanLimit
 *    The maximum number of Wi-Fi scans permitted before the operation
 *    is abandoned and its completion is reported.
 *
 *  @param[in]      inReason
 *    A pointer to an immutable null-terminated C string describing
 *    why the SSID is unresolved, used both in the completion error log
 *    and as the retry log reason.
 *
 *  @param[in]      inKind
 *    Selects the completion disposition: `#WiFiScanCompletionKind::kScan`
 *    completes `OnScanFinished`; `#WiFiScanCompletionKind::kConnect`
 *    completes `OnConnectResult`.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT
 *    If `inTechnology` or `inReason` is null.
 *
 *  @retval  #CHIP_ERROR_KEY_NOT_FOUND
 *    If the scan budget is exhausted for `#WiFiScanCompletionKind::kScan`
 *    (advisory; `OnScanFinished` is the authoritative completion).
 *
 *  @retval  #CHIP_NO_ERROR
 *    If a further scan was successfully armed.
 *
 *  @retval  *
 *    Otherwise, a POSIX ENOENT #CHIP_ERROR for an exhausted
 *    `#WiFiScanCompletionKind::kConnect` budget (advisory), or any error
 *    returned by #HandleWiFiScanRetryLocked in arming a further scan.
 *
 *  @sa HandleWiFiScanRetryLocked
 *  @sa HandleWiFiPendingConnectLocked
 *  @sa HandleWiFiPendingScanLocked
 *  @sa DispatchWiFiScanFinishedLocked
 *  @sa DispatchWiFiConnectFinishedLocked
 *
 *  @private
 *
 */
CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::HandleWiFiUnresolvedAfterScanLocked(
    std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology, WiFiScanState & inOutScanState,
    const size_t & inScanLimit, const char * inReason, WiFiScanCompletionKind inKind) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inReason != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (inOutScanState.mCount >= inScanLimit)
    {
        // Snapshot the SSID for the debug text before the reset; the dispatch
        // helpers copy these bytes by value, so the reset cannot race the
        // deferred callback.

        const Internal::WiFiSSIDFixedBuffer ssid = inOutScanState.mSsid;
        const CharSpan debug(reinterpret_cast<const char *>(ssid.data()), ssid.size());

        ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s SSID '%.*s' after %zu scan(s)", inReason,
                     static_cast<int>(ssid.size()), reinterpret_cast<const char *>(ssid.data()), inOutScanState.mCount);

        // Reset the SSID and scan count.
        //
        // Clear the operation's scan state BEFORE dispatching the
        // completion result; the snapshot above keeps the reported SSID
        // stable across the reset.

        inOutScanState.Reset();

        switch (inKind)
        {

        case WiFiScanCompletionKind::kScan:
            ReturnErrorOnFailure(DispatchWiFiScanFinishedLocked(inOutLock, Status::kNetworkNotFound, debug, nullptr));

            return CHIP_ERROR_KEY_NOT_FOUND;

        case WiFiScanCompletionKind::kConnect:
            ReturnErrorOnFailure(CompleteWiFiConnectLocked(inOutLock, Status::kNetworkNotFound, debug, 0));

            return ChipError(ChipError::Range::kPOSIX, ENOENT);
        }

        return CHIP_ERROR_INTERNAL;
    }

    return HandleWiFiScanRetryLocked(inOutLock, inTechnology, inReason, inOutScanState);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerAgentOpLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                  const char * inPath,
                                                                                  ManagerAgentOpFunc inManagerAgentOpFunc,
                                                                                  const char * inAction,
                                                                                  const bool & inRegister) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inManagerAgentOpFunc != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inAction != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(mConnManClient.mManagerProxy, CHIP_ERROR_UNINITIALIZED);

    VerifyOrReturnError(mConnManAgentServer.mRegistered != inRegister, CHIP_ERROR_INCORRECT_STATE);

    // Hold a reference to the manager proxy across the unlock boundary.

    GAutoPtr<ConnManManager> manager(static_cast<ConnManManager *>(g_object_ref(mConnManClient.mManagerProxy.get())));

    // Unlock and synchronously invoke the manager agent operation
    // function on the GLib context.

    ReturnErrorOnFailure(UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<GError> err;
        if (!inManagerAgentOpFunc(manager.get(), inPath, nullptr, &err.GetReceiver()))
        {
            ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "could not %s manager agent %s: %s", inAction, inPath,
                         (err.get() && err->message) ? err->message : "unknown error");

            return CHIP_ERROR_INTERNAL;
        }

        return CHIP_NO_ERROR;
    }));

    mConnManAgentServer.mRegistered = inRegister;

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::MaybeClearInterfaceNameLocked(const char * inPath) noexcept
{
    const char * type = nullptr;
    GAutoPtr<gchar> interface;

    VerifyOrReturn(inPath != nullptr);

    ReturnOnFailure(GetObjectTypeFromPathLocked(mConnManClient.mServices.get(), inPath, type));

    ReturnOnFailure(GetServiceInterfaceFromPathLocked(mConnManClient.mServices.get(), inPath, interface));

    NetworkServiceState * const state = FindNetworkServiceStateLocked(type);
    if (state != nullptr)
    {
        _MaybeClearInterfaceName(interface.get(), &state->mIfName[0]);
    }
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::MaybeSetInterfaceNameLocked(const char * inType,
                                                                                         const char * inInterface) noexcept
{
    VerifyOrReturnError(inType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inInterface != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Untracked Matter network service types (for example, cellular or VPN) are
    // silently ignored.

    NetworkServiceState * const state = FindNetworkServiceStateLocked(inType);
    if (state != nullptr)
    {
        _MaybeSetInterfaceName(state->mDescription, inInterface, &state->mIfName[0]);
    }

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl_NetworkManagementConnMan::NotifyWiFiConnectivityChange(ConnectivityChange inChange) noexcept
{
    const ChipDeviceEvent event{ .Type                   = DeviceEventType::kWiFiConnectivityChange,
                                 .WiFiConnectivityChange = { .Result = inChange } };

    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::RemoveServiceLocked(const char * inPath) noexcept
{
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return RemoveObjectLocked(mConnManClient.mServices.get(), inPath, mConnManClient.mServiceProxies.get());
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::RemoveTechnologyLocked(const char * inPath) noexcept
{
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return RemoveObjectLocked(mConnManClient.mTechnologies.get(), inPath, mConnManClient.mTechnologyProxies.get());
}

/**
 *  @brief
 *    Scrub and clear the cached Wi-Fi connect credential.
 *
 *  The credential's lifetime is that of the D-Bus `Service.Connect()`
 *  call, not that of any single `Agent.RequestInput`: connman may
 *  legitimately request input more than once within one connect
 *  (hidden networks, WPS, and any future `Agent.Error.Retry` reply).
 *  It is therefore scrubbed by whichever of the two scope terminators
 *  runs: #ShutdownClientConnectSessionLocked, when `Connect()` was
 *  issued and has replied; or the abort guard in
 *  `ConnectWiFiNetworkAsync`, when `Connect()` was never issued at
 *  all.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`.
 *
 *  @sa ShutdownClientConnectSessionLocked
 *
 *  @private
 *
 */
void ConnectivityManagerImpl_NetworkManagementConnMan::ScrubWiFiClientConnectPassphraseLocked() noexcept
{
    if (!mWiFiClientConnectPassphrase.empty())
    {
        Crypto::DRBG_get_bytes(mWiFiClientConnectPassphrase.data(), mWiFiClientConnectPassphrase.size());

        mWiFiClientConnectPassphrase.clear();
    }
}

/**
 *  @brief
 *    Shut down the connman connect session: the state whose lifetime
 *    is that of the D-Bus `Service.Connect()` call.
 *
 *  The peer of #ConcludeWiFiClusterConnectLocked. Retires everything
 *  connman may still reach into while a connect request is
 *  outstanding: the registered agent, the pending service proxy
 *  against which agent requests are path-matched, and the cached
 *  credential from which `Agent.RequestInput` is answered.
 *
 *  @note
 *    Which to call: *"can connman still call my agent?"* -- if it can,
 *    the session is live and this must not run. Contrast
 *    #ConcludeWiFiClusterConnectLocked, whose test is *"has the Network
 *    Commissioning cluster been answered?"*. The two scopes do not
 *    nest and do not end together: connman drives the service to
 *    "ready" or "failure" -- concluding the *Matter* connect -- before
 *    it replies to `Connect()`, and may request agent input at any
 *    point until it does.
 *
 *  @note
 *    The `Locked` suffix denotes the precondition that the caller
 *    holds `mConnManMutex`; this is verified by assertion. The lock
 *    may be temporarily released by the agent unregistration, which is
 *    why this takes the lock and #ConcludeWiFiClusterConnectLocked does not.
 *
 *  @param[in,out]  inOutLock
 *    A reference to the mutable, held class lock.
 *
 *  @sa ConcludeWiFiClusterConnectLocked
 *  @sa ScrubWiFiClientConnectPassphraseLocked
 *
 *  @private
 *
 */
void ConnectivityManagerImpl_NetworkManagementConnMan::ShutdownClientConnectSessionLocked(
    std::unique_lock<std::mutex> & inOutLock) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    if (mConnManAgentServer.mRegistered)
    {
        LogErrorOnFailure(ManagerUnregisterAgentLocked(inOutLock, MATTER_CONNECTIVITY_MANGER_CONNMAN_AGENT_PATH));
    }

    mConnManAgentServer.mPendingService.reset();

    ScrubWiFiClientConnectPassphraseLocked();
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateManagerPropertiesLocked(GVariant * inProperties) noexcept
{
    GHashTable * const table = mConnManClient.mProperties.get();

    VerifyOrReturnError(inProperties != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(table, CHIP_ERROR_INCORRECT_STATE);

    g_hash_table_remove(table, CONNMAN_MANAGER_PATH);

    return MergeObjectPropertiesLocked("manager properties (update)", table, CONNMAN_MANAGER_PATH, inProperties);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateNetworkServiceConnectivityLocked() noexcept
{
    bool changed = false;

    // This method, like all *Locked methods reached via
    // ScheduleLambda-marshaled handlers, runs on the Matter thread;
    // the System layer timer APIs below require it.

    VerifyOrDie(PlatformMgr().IsChipStackLockedByCurrentThread());

    // Recompute the pending per-family connectivity state for each
    // tracked network service type from the cached properties of the
    // best service of that type. Absence of any service of the type
    // means both families are lost.

    for (auto & state : mNetworkServiceStates)
    {
        NetworkServiceConnectivityState pending;
        GAutoPtr<gchar> path;
        GVariant * props = nullptr;
        CHIP_ERROR status;

        status = GetBestServicePathForTypeLocked(state.mType, mConnManClient.mServices.get(), path);

        if (status == CHIP_NO_ERROR)
        {
            status = GetObjectPropertiesFromPathLocked(mConnManClient.mServices.get(), path.get(), props);
        }

        if ((status == CHIP_NO_ERROR) && (props != nullptr))
        {
            pending.mIPv4 = GetServiceFamilyConnectivityStateLocked(props, kConnManServicePropertyIPv4Key);
            pending.mIPv6 = GetServiceFamilyConnectivityStateLocked(props, kConnManServicePropertyIPv6Key);
        }
        else
        {
            pending.mIPv4.mConnectivity = NetworkServiceConnectivity::kLost;
            pending.mIPv6.mConnectivity = NetworkServiceConnectivity::kLost;
        }

        if (pending != state.mPendingConnectivity)
        {
            state.mPendingConnectivity = pending;
            changed                    = true;

            ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s connectivity pending: IPv4 %s IPv6 %s",
                          state.mDescription, GetNetworkServiceConnectivityString(pending.mIPv4.mConnectivity),
                          GetNetworkServiceConnectivityString(pending.mIPv6.mConnectivity));
        }
    }

    // If any tracked type's pending state changed, (re)arm the
    // trailing-edge debounce timer. Storms that settle back to the
    // previously-reported state result in a timer expiration that
    // reports nothing.

    if (changed)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleNetworkServiceConnectivityDebounce, this);

        ReturnErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(kNetworkServiceConnectivityDebounce,
                                                                   HandleNetworkServiceConnectivityDebounce, this));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateServicesLocked(GVariant * inServices) noexcept
{
    VerifyOrReturnError(inServices != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mConnManClient.mServices, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(g_variant_is_of_type(inServices, G_VARIANT_TYPE("a(oa{sv})")), CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_remove_all(mConnManClient.mServices.get());

    ReturnErrorOnFailure(MergeObjectPropertiesLocked("services (update)", mConnManClient.mServices.get(), inServices));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateServiceProxyLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                      const char * inPath,
                                                                                      ConnManService *& outService) noexcept
{
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mConnManClient.mServiceProxies, CHIP_ERROR_INCORRECT_STATE);

    ConnManService * proxy = static_cast<ConnManService *>(g_hash_table_lookup(mConnManClient.mServiceProxies.get(), inPath));
    if (proxy == nullptr)
    {
        ReturnErrorOnFailure(UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
            GAutoPtr<GError> err;
            ConnManService * service = conn_man_service_proxy_new_for_bus_sync(
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kConnManServiceName, inPath, nullptr, &err.GetReceiver());

            if (!service || err)
            {
                ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to create service proxy for %s: %s",
                             inPath, err ? err->message : "unknown");

                return CHIP_ERROR_INTERNAL;
            }

            ServiceRegisterPropertyChangedOnGLib(service);

            proxy = service;

            return CHIP_NO_ERROR;
        }));

        g_hash_table_insert(mConnManClient.mServiceProxies.get(), g_strdup(inPath), proxy);
    }

    outService = proxy;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateTechnologiesLocked(GVariant * inTechnologies) noexcept
{
    VerifyOrReturnError(inTechnologies != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mConnManClient.mTechnologies, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(g_variant_is_of_type(inTechnologies, G_VARIANT_TYPE("a(oa{sv})")), CHIP_ERROR_INVALID_ARGUMENT);

    g_hash_table_remove_all(mConnManClient.mTechnologies.get());

    return MergeObjectPropertiesLocked("technologies (update)", mConnManClient.mTechnologies.get(), inTechnologies);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::UpdateTechnologyProxyLocked(
    std::unique_lock<std::mutex> & inOutLock, const char * inPath, ConnManTechnology *& outTechnology) noexcept
{
    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mConnManClient.mTechnologyProxies, CHIP_ERROR_INCORRECT_STATE);

    ConnManTechnology * proxy =
        static_cast<ConnManTechnology *>(g_hash_table_lookup(mConnManClient.mTechnologyProxies.get(), inPath));
    if (proxy == nullptr)
    {
        ReturnErrorOnFailure(UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
            GAutoPtr<GError> err;
            ConnManTechnology * technology = conn_man_technology_proxy_new_for_bus_sync(
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kConnManServiceName, inPath, nullptr, &err.GetReceiver());

            if (!technology || err)
            {
                ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to create technology proxy for %s: %s",
                             inPath, err ? err->message : "unknown");

                return CHIP_ERROR_INTERNAL;
            }

            TechnologyRegisterPropertyChangedOnGLib(technology);

            proxy = technology;

            return CHIP_NO_ERROR;
        }));

        g_hash_table_insert(mConnManClient.mTechnologyProxies.get(), g_strdup(inPath), proxy);
    }

    outTechnology = proxy;

    return CHIP_NO_ERROR;
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
    GAutoPtr<gchar> path;
    GAutoPtr<GDBusConnection> connection;
    ConnManAgent * skeleton = nullptr;
    GAutoPtr<GError> exportError;
    CHIP_ERROR status;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(mConnManClient.mManagerProxy, CHIP_ERROR_UNINITIALIZED);

    VerifyOrReturnError(!mConnManAgentServer.mExported, CHIP_ERROR_ALREADY_INITIALIZED);

    path.reset(g_strdup(inPath));
    VerifyOrReturnError(path.get() != nullptr, CHIP_ERROR_NO_MEMORY);

    // Hold a reference to the manager client proxy connection across
    // the unlock boundary.

    {
        GDBusConnection * c = g_dbus_proxy_get_connection(G_DBUS_PROXY(mConnManClient.mManagerProxy.get()));
        VerifyOrReturnError(c != nullptr, CHIP_ERROR_INTERNAL);
        connection.reset(static_cast<GDBusConnection *>(g_object_ref(c)));
    }

    ReturnErrorOnFailure(UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        return InitAgentOnGLib(this, connection.get(), path.get(), skeleton, &exportError.GetReceiver());
    }));

    if (skeleton == nullptr)
    {
        if (exportError.get() != nullptr)
        {
            ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "could not export agent server skeleton: %s",
                         exportError->message);
        }

        return CHIP_ERROR_INTERNAL;
    }

    // Commit exported objects under lock.

    mConnManAgentServer.mConnection.reset(static_cast<GDBusConnection *>(g_object_ref(connection.get())));
    mConnManAgentServer.mSkeleton.reset(skeleton);
    mConnManAgentServer.mExported = true;

    status = ManagerAgentOpLocked(inOutLock, path.get(), conn_man_manager_call_register_agent_sync, "register", true);
    if ((status != CHIP_NO_ERROR))
    {
        ShutdownAgentLocked(inOutLock, inPath);

        return status;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerUnregisterAgent(const char * inPath) noexcept
{
    std::unique_lock<std::mutex> lock(mConnManMutex);

    return ManagerUnregisterAgentLocked(lock, inPath);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ManagerUnregisterAgentLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                          const char * inPath) noexcept
{
    GAutoPtr<gchar> path;
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    path.reset(g_strdup(inPath));
    VerifyOrReturnError(path.get() != nullptr, CHIP_ERROR_NO_MEMORY);

    retval = ManagerAgentOpLocked(inOutLock, path.get(), conn_man_manager_call_unregister_agent_sync, "unregister", false);

    ShutdownAgentLocked(inOutLock, inPath);

    return retval;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ServiceConnectLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                  ConnManService * inService) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inService != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Initiate the asynchronous D-Bus call on the GLib context while *not* holding the class lock.
    // This is a synchronous cross-thread invoke to *schedule* the asynchronous operation; it does
    // not wait for the D-Bus operation to complete--service connect is inherently asynchronous.

    return UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
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
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::ServiceRemoveLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                 ConnManService * inService) noexcept
{
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inService != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GAutoPtr<ConnManService> service(static_cast<ConnManService *>(g_object_ref(inService)));

    return UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<GError> err;
        gboolean status = conn_man_service_call_remove_sync(service.get(), nullptr, &err.GetReceiver());
        if (!status || err)
        {
            const CHIP_ERROR mapped = MapClientError(err.get());

            ChipLogError(
                DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to remove service %s: %s (%" CHIP_ERROR_FORMAT ")",
                g_dbus_proxy_get_object_path(G_DBUS_PROXY(service.get())), err ? err->message : "unknown", mapped.Format());

            return mapped;
        }

        return CHIP_NO_ERROR;
    });
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
    VerifyOrDie(inOutLock.owns_lock());

    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Initiate the asynchronous D-Bus call on the GLib context while *not* holding the class lock.
    // This is a synchronous cross-thread invoke to *schedule* the asynchronous operation; it does
    // not wait for the D-Bus operation to complete--technology scan is inherently asynchronous.

    return UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
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
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::TechnologySetPropertyLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                         ConnManTechnology * inTechnology,
                                                                                         const char * inKey,
                                                                                         GVariant * inValue) noexcept
{
    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(inValue != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GAutoPtr<ConnManTechnology> technology(static_cast<ConnManTechnology *>(g_object_ref(inTechnology)));

    GAutoPtr<gchar> key(g_strdup(inKey));
    VerifyOrReturnError(key.get() != nullptr, CHIP_ERROR_NO_MEMORY); // technology released here on the OOM path

    GAutoPtr<GVariant> value(g_variant_ref(inValue));

    return UnlockAndInvokeOnGLibContextSync(inOutLock, [&]() -> CHIP_ERROR {
        GAutoPtr<GError> err;
        gboolean status =
            conn_man_technology_call_set_property_sync(technology.get(), key.get(), value.get(), nullptr, &err.GetReceiver());
        if (!status || err)
        {
            const CHIP_ERROR mapped = MapClientError(err.get());

            ChipLogError(DeviceLayer,
                         CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "failed to set technology %s property %s: %s (%" CHIP_ERROR_FORMAT
                                                                 ")",
                         g_dbus_proxy_get_object_path(G_DBUS_PROXY(technology.get())), key.get(), err ? err->message : "unknown",
                         mapped.Format());

            return mapped;
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementConnMan::TechnologySetPoweredLocked(std::unique_lock<std::mutex> & inOutLock,
                                                                                        ConnManTechnology * inTechnology,
                                                                                        const bool & inPowered) noexcept
{
    VerifyOrReturnError(inTechnology != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    GAutoPtr<GVariant> value(g_variant_new_boolean(inPowered));
    VerifyOrReturnError(value.get() != nullptr, CHIP_ERROR_NO_MEMORY);

    return TechnologySetPropertyLocked(inOutLock, inTechnology, kConnManTechnologyPropertyPoweredKey, value.get());
}

// System Layer Timer Completion Methods

void ConnectivityManagerImpl_NetworkManagementConnMan::HandleNetworkServiceConnectivityDebounce(System::Layer * inSystemLayer,
                                                                                                void * inAppState)
{
    auto * lSelf = static_cast<ConnectivityManagerImpl_NetworkManagementConnMan *>(inAppState);

    VerifyOrReturn(lSelf != nullptr);

    lSelf->ReportNetworkServiceConnectivity();
}

void ConnectivityManagerImpl_NetworkManagementConnMan::ReportNetworkServiceConnectivity() noexcept
{
    ChipDeviceEvent event{};
    bool report = false;
    CHIP_ERROR status;

    // Gather and diff under lock; post outside it.

    {
        std::lock_guard<std::mutex> lock(mConnManMutex);

        FamilyConnectivityAggregate v4;
        FamilyConnectivityAggregate v6;

        for (auto & state : mNetworkServiceStates)
        {
            AccumulateFamilyConnectivity(state.mPendingConnectivity.mIPv4, state.mReportedConnectivity.mIPv4, v4);
            AccumulateFamilyConnectivity(state.mPendingConnectivity.mIPv6, state.mReportedConnectivity.mIPv6, v6);

            // Reported state adopts pending state regardless of
            // whether an event posts, so that absorbed transitions
            // establish the baseline.

            state.mReportedConnectivity = state.mPendingConnectivity;
        }

        event.Type = DeviceEventType::kInternetConnectivityChange;

        report |= SetConnectivityChangeFromAggregate(v4, event.InternetConnectivityChange.IPv4);
        report |= SetConnectivityChangeFromAggregate(v6, event.InternetConnectivityChange.IPv6);

        if (v4.mReportable && v4.mAnyEstablished)
        {
            event.InternetConnectivityChange.ipAddress = v4.mAddress;
        }
        else if (v6.mReportable && v6.mAnyEstablished)
        {
            event.InternetConnectivityChange.ipAddress = v6.mAddress;
        }
    }

    if (report)
    {
        ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "Internet connectivity change: IPv4 %s IPv6 %s",
                        GetConnectivityChangeString(event.InternetConnectivityChange.IPv4),
                        GetConnectivityChangeString(event.InternetConnectivityChange.IPv6));

        status = PlatformMgr().PostEvent(&event);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer,
                         CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX
                         "failed to post Internet connectivity change event: %" CHIP_ERROR_FORMAT,
                         status.Format());
        }
    }
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

    LogErrorOnFailure(
        DeviceLayer::SystemLayer().ScheduleLambda([this, object, objects, err, inHandleManagerGetObjectsMethod]() -> void {
            GAutoPtr<GObject> retained_object(object);
            GAutoPtr<GVariant> retained_objects(objects);
            GAutoPtr<GError> retained_err(err);

            (this->*inHandleManagerGetObjectsMethod)(CONN_MAN_MANAGER(retained_object.get()), retained_objects.get(),
                                                     retained_err.get());
        }));
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, object, err, inHandleObjectActionCompleteMethod]() -> void {
        GAutoPtr<GObject> retained_object(object);
        GAutoPtr<GError> retained_err(err);

        (this->*inHandleObjectActionCompleteMethod)(G_DBUS_PROXY(retained_object.get()), retained_err.get());
    }));
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
    VerifyOrReturnValue(inAgent != nullptr, TRUE);
    VerifyOrReturnValue(inInvocation != nullptr, TRUE);

    conn_man_agent_complete_cancel(inAgent, inInvocation);

    // The connman agent contract requires the handler to report the
    // invocation as handled; there is no path on which we decline it.

    return TRUE;
}

gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentRelease(ConnManAgent * inAgent,
                                                                          GDBusMethodInvocation * inInvocation) noexcept
{
    VerifyOrReturnValue(inAgent != nullptr, TRUE);
    VerifyOrReturnValue(inInvocation != nullptr, TRUE);

    conn_man_agent_complete_release(inAgent, inInvocation);

    // The connman agent contract requires the handler to report the
    // invocation as handled; there is no path on which we decline it.

    return TRUE;
}

/**
 *  @brief
 *    Answer connman's `Agent.RequestInput` with the cached Wi-Fi
 *    credential.
 *
 *  @note
 *    **Runs on the GLib thread and takes `mConnManMutex` directly.**
 *    Unlike the connman signal handlers (`OnServicePropertyChanged`
 *    and friends), which marshal to the Matter thread via
 *    `ScheduleLambda`, this *cannot*: it must answer the D-Bus method
 *    invocation synchronously, with the passphrase, before returning.
 *
 *    There is no deferral that preserves that contract. This is the
 *    precedent that also licenses the same treatment in
 *    #OnAgentReportError.
 *
 *  @note
 *    The cached credential is deliberately **not** scrubbed here.
 *    connman may legitimately call `RequestInput` more than once
 *    within a single `Service.Connect()` (hidden networks, WPS, and
 *    any future `Agent.Error.Retry` reply) and scrubbing on the first
 *    would fail the second with a misleading "not available".
 *    #ShutdownClientConnectSessionLocked owns the scrub, at the
 *    close of the D-Bus call that is the credential's true scope.
 *
 *  @sa OnAgentReportError
 *  @sa ShutdownClientConnectSessionLocked
 *
 *  @private
 *
 */
gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentRequestInput(ConnManAgent * inAgent,
                                                                               GDBusMethodInvocation * inInvocation,
                                                                               const gchar * inPath,
                                                                               GVariant * inProperties) noexcept
{
    std::lock_guard<std::mutex> lock(mConnManMutex);

    // The cached credential is deliberately NOT scrubbed here. Its lifetime is
    // the D-Bus Service.Connect() call, not this single request: connman may
    // legitimately call Agent.RequestInput more than once within one connect,
    // and scrubbing here would fail the second with a misleading "not
    // available". ShutdownClientConnectSessionLocked owns the scrub.

    // Answer the invocation with a G_IO_ERROR and yield the TRUE the
    // agent contract mandates. Safe when there is no invocation to
    // answer.

    const auto fail = [&](gint inCode_, const char * inMessage_) -> gboolean {
        if (inInvocation != nullptr)
        {
            g_dbus_method_invocation_return_error(inInvocation, G_IO_ERROR, inCode_, "%s", inMessage_);
        }

        return TRUE;
    };

    // Sanity check the input parameters.

    VerifyOrReturnValue(inAgent != nullptr, fail(G_IO_ERROR_INVALID_ARGUMENT, "inAgent == nullptr"));
    VerifyOrReturnValue(inInvocation != nullptr, TRUE);
    VerifyOrReturnValue(inPath != nullptr, fail(G_IO_ERROR_INVALID_ARGUMENT, "inPath == nullptr"));
    VerifyOrReturnValue(inProperties != nullptr, fail(G_IO_ERROR_INVALID_ARGUMENT, "inProperties == nullptr"));

    ChipLogProgress(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "agent service %s request input", inPath);

    // First, ensure there is a pending service we are currently connecting.

    VerifyOrReturnValue(mConnManAgentServer.mPendingService, fail(G_IO_ERROR_FAILED, "No pending service for RequestInput"));

    // Second, ensure the pending service matches the one requesting input.

    const char * pending_path = g_dbus_proxy_get_object_path(G_DBUS_PROXY(mConnManAgentServer.mPendingService.get()));
    VerifyOrReturnValue(pending_path != nullptr, fail(G_IO_ERROR_FAILED, "No path for pending service"));

    const bool paths_match = (strcmp(pending_path, inPath) == 0);
    VerifyOrReturnValue(paths_match, fail(G_IO_ERROR_FAILED, "Input requested for unexpected service"));

    // Third, validate the request type.

    VerifyOrReturnValue(g_variant_is_of_type(inProperties, G_VARIANT_TYPE_VARDICT),
                        fail(G_IO_ERROR_INVALID_ARGUMENT, "properties are not 'a{sv}' type"));

    bool want_passphrase = false;
    {
        GVariantIter iter;
        const char * key = nullptr;
        GVariant * boxed = nullptr;
        g_variant_iter_init(&iter, inProperties);

        // Fourth, iterate over the requested property keys and decide
        // what we can satisfy relative to what is requested.

        while (g_variant_iter_next(&iter, "{&s@v}", &key, &boxed))
        {
            UnboxedVariant variant(boxed);
            g_variant_unref(boxed);

            if (!variant || !g_variant_is_of_type(variant.get(), G_VARIANT_TYPE_VARDICT))
            {
                continue;
            }

            if (strcmp(key, kConnManServiceAgentPropertyPassphraseKey) == 0)
            {
                ChipLogDetail(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "%s: wants %s", __func__, key);

                // Optional: check "Type" == "passphrase" and/or Requirement.

                want_passphrase = true;
            }
        }
    }

    // Fifth and finally, build the reply with only the requested keys.

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    if (want_passphrase)
    {
        VerifyOrReturnValue(!mWiFiClientConnectPassphrase.empty(),
                            fail(G_IO_ERROR_FAILED, "Passphrase requested but not available"));

        char passphrase[Internal::kMaxWiFiKeyLength + 1];
        const size_t n = std::min<size_t>(mWiFiClientConnectPassphrase.size(), Internal::kMaxWiFiKeyLength);
        memcpy(passphrase, mWiFiClientConnectPassphrase.data(), n);
        passphrase[n] = '\0';

        g_variant_builder_add(&builder, "{sv}", kConnManServiceAgentPropertyPassphraseKey, g_variant_new_string(passphrase));

        // g_variant_new_string copied the passphrase bytes; wipe the stack copy.

        Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(passphrase), sizeof(passphrase));
    }

    conn_man_agent_complete_request_input(inAgent, inInvocation, g_variant_builder_end(&builder));

    // The connman agent contract requires the handler to report the
    // invocation as handled; there is no path on which we decline it.

    return TRUE;
}

/**
 *  @brief
 *    Latch the cause of a connman network service failure reported
 *    via `Agent.ReportError`, as a fallback to the "Error" property.
 *
 *  @note
 *    **Runs on the GLib thread and takes `mConnManMutex` directly,
 *    and must not be marshaled to the Matter thread.** This is subtle
 *    and load-bearing. connman emits, in order:
 *
 *      1. `PropertyChanged("Error")`
 *      2. `PropertyChanged("State" = "failure")`
 *      3. `Agent.ReportError()`
 *
 *    Signals (1) and (2) are *already* deferred onto the Matter
 *    thread by `OnServicePropertyChanged`, and (2) concludes the
 *    Matter connect via #ConcludeWiFiClusterConnectLocked, which clears
 *    the latch and the pending gate. Marshaling this handler as well
 *    would queue it strictly *behind* both (that is, behind the very
 *    conclusion that clears what it is trying to set) so its
 *    `HasWiFiClusterConnectPendingLocked()` guard below would reject it on
 *    every single connect, rendering the fallback dead code. Handled
 *    inline on the GLib thread, it can still win that race and serve
 *    as the fallback it is meant to be.
 *
 *  @note
 *    Secondary latch only;
 *    #HandleWiFiPendingConnectServiceErrorChangedLocked, driven from
 *    signal (1), is the primary and normally wins. Hence the
 *    never-overwrite guard: in the ordinary case this handler arrives
 *    *after* the failure has already been mapped, dispatched, and
 *    concluded, and an unguarded write would arm a stale error for
 *    the *next* connect.
 *
 *  @sa HandleWiFiPendingConnectServiceErrorChangedLocked
 *  @sa OnAgentRequestInput
 *
 *  @private
 *
 */
gboolean ConnectivityManagerImpl_NetworkManagementConnMan::OnAgentReportError(ConnManAgent * inAgent,
                                                                              GDBusMethodInvocation * inInvocation,
                                                                              const gchar * inPath, const gchar * inError) noexcept
{
    VerifyOrReturnValue(inAgent != nullptr, TRUE);
    VerifyOrReturnValue(inInvocation != nullptr, TRUE);

    ChipLogError(DeviceLayer, CONNECTIVITY_MANAGER_CONNMAN_LOG_PREFIX "service %s had error %s", inPath ? inPath : "(null)",
                 inError ? inError : "(null)");

    conn_man_agent_complete_report_error(inAgent, inInvocation);

    // Secondary latch only, and deliberately handled inline on the GLib thread
    // rather than marshaled to the Matter thread: connman emits
    // PropertyChanged("Error") and PropertyChanged("State" = "failure") ahead of
    // this call, and those are *already* deferred via ScheduleLambda. Deferring
    // this as well would queue it strictly behind them -- that is, behind the
    // ConcludeWiFiClusterConnectLocked that the failure transition performs --
    // and every guard below would reject it, making the latch dead code.
    // Handled inline, it can still win that race and serve as the fallback it
    // is meant to be.
    //
    // Never overwrite a latched cause, and never latch for a connect that is no
    // longer in flight or for a service that is not ours: connman calls
    // ReportError *after* the failure transition in the ordinary case, and an
    // unguarded write would arm a stale error for the *next* connect.

    if ((inError != nullptr) && HasWiFiClusterConnectPendingLocked() && IsWiFiPendingConnectServicePathLocked(inPath) &&
        !mWiFiClusterConnectServiceError.has_value())
    {
        mWiFiClusterConnectServiceError = MapServiceError(inError);
    }

    // The connman agent contract requires the handler to report the
    // invocation as handled; there is no path on which we decline it.

    return TRUE;
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, manager, key, value]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<gchar> retained_key(const_cast<gchar *>(key));
        GAutoPtr<GVariant> retained_value(value);

        HandleManagerPropertyChanged(retained_manager.get(), retained_key.get(), retained_value.get());
    }));
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, manager, changed, removed]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<GVariant> retained_changed(changed);
        GAutoPtr<gchar *> retained_removed(removed);

        HandleManagerServicesChanged(retained_manager.get(), retained_changed.get(), retained_removed.get());
    }));
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, manager, path, properties]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<gchar> retained_path(const_cast<gchar *>(path));
        GAutoPtr<GVariant> retained_properties(properties);

        HandleManagerTechnologyAdded(retained_manager.get(), retained_path.get(), retained_properties.get());
    }));
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, manager, path]() -> void {
        GAutoPtr<ConnManManager> retained_manager(manager);
        GAutoPtr<gchar> retained_path(const_cast<gchar *>(path));

        HandleManagerTechnologyRemoved(retained_manager.get(), retained_path.get());
    }));
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, service, key, value]() -> void {
        GAutoPtr<ConnManService> retained_service(service);
        GAutoPtr<gchar> retained_key(const_cast<gchar *>(key));
        GAutoPtr<GVariant> retained_value(value);

        HandleServicePropertyChanged(retained_service.get(), retained_key.get(), retained_value.get());
    }));
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

    LogErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, technology, key, value]() -> void {
        GAutoPtr<ConnManTechnology> retained_technology(technology);
        GAutoPtr<gchar> retained_key(const_cast<gchar *>(key));
        GAutoPtr<GVariant> retained_value(value);

        HandleTechnologyPropertyChanged(retained_technology.get(), retained_key.get(), retained_value.get());
    }));
}

} // namespace DeviceLayer
} // namespace chip
