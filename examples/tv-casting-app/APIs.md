# Matter Casting APIs

Matter Casting consists of three parts:

-   **The mobile app**: For most content providers, this would be your
    consumer-facing mobile app. By making your mobile app a Matter "Casting
    Client", you enable the user to discover casting targets, cast content, and
    control casting sessions. The
    [example Matter tv-casting-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-casting-app)
    for Android / iOS and Linux builds on top of the Matter SDK to demonstrate
    how a TV Casting mobile app works.
-   **The TV content app**: For most content providers, this would be your
    consumer-facing app on a Smart TV. By enhancing your TV app to act as a
    Matter "Content app", you enable Matter Casting Clients to cast content. The
    [example Matter content-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-app/android/App/content-app)
    for Android builds on top of the Matter SDK to demonstrate how a TV Content
    app works.
-   **The TV platform app**: The TV platform app implements the Casting Video  
    Player device type and provides common capabilities around media playback on
    the TV such as play/pause, keypad navigation, input and output control,
    content search, and an implementation of an app platform as described in the
    media chapter of the device library specification. This is generally
    implemented by the TV manufacturer. The
    [example Matter tv-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-app)
    for Android builds on top of the Matter SDK to demonstrate how a TV platform
    app works.

This document describes how enable your Android and iOS apps to act as a Matter
"Casting Client". This documentation is also designed to work with the example
[example Matter tv-casting-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-casting-app)
samples so you can see the experience end to end.

## Introduction

A Casting Client (e.g. a mobile phone app) is expected to be a Matter
Commissionable Node and a `CastingPlayer` (i.e. a TV) is expected to be a Matter
Commissioner. In the context of the
[Matter Video Player architecture](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/media/VideoPlayerArchitecture.adoc),
a `CastingPlayer` would map to
[Casting "Video" Player](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/media/VideoPlayerArchitecture.adoc#1-introduction).
The `CastingPlayer` is expected to be hosting one or more `Endpoints` (some of
which can represent
[Content Apps](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/media/VideoPlayerArchitecture.adoc#1-introduction)
in the Matter Video Player architecture) that support one or more Matter Media
`Clusters`.

The steps to start a casting session are:

1. [Initialize](#initialize-the-casting-client) the `CastingClient` using the
   Matter SDK.
1. [Discover](#discover-casting-players) `CastingPlayer` devices using Matter
   Commissioner discovery.
1. [Connect](#connect-to-a-casting-player) to the `CastingPlayer` to discover
   available endpoints. By connecting, the `CastingClient` will send a User
   Directed Commissioning (UDC) request to the `CastingPlayer` device in order
   to make a Matter commissioning request. The `CastingPlayer` will then obtain
   the appropriate user consent to allow a connection from this `CastingClient`
   and obtain the setup code needed to commission the `CastingClient`. The setup
   code will typically come from a corresponding TV content app or be input by
   the user.
1. [Select](#select-an-endpoint-on-the-casting-player) an available `Endpoint`
   hosted by the `CastingPlayer`.

Next, you're ready to:

1. [Issue commands](#issuing-commands) to the `Endpoint`.
1. [Read](#read-operations) endpoint attributes like playback state.
1. [Subscribe](#subscriptions) to playback events.

## Build and Setup

The Casting Client is expected to consume the Matter TV Casting library built
for its respective platform which implements the APIs described in this
document. Refer to the tv-casting-app READMEs for [Linux](linux/README.md),
Android and [iOS](darwin/TvCasting/README.md) to understand how to build and
consume each platform's specific libraries. The libraries MUST be built with the
client's specific values for `CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID` and
`CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID` updated in the
[CHIPProjectAppConfig.h](tv-casting-common/include/CHIPProjectAppConfig.h) file.

### Initialize the Casting Client

_{Complete Initialization examples: [Linux](linux/simple-app.cpp) |
[Android](android/App/app/src/main/java/com/matter/casting/InitializationExample.java)
| [iOS](darwin/TvCasting/TvCasting/MTRInitializationExample.swift)}_

A Casting Client must first initialize the Matter SDK and define the following
`DataProvider` objects for the the Matter Casting library to use throughout the
client's lifecycle:

1.  **Rotating Device Identifier** - Refer to the Matter specification for
    details on how to generate the
    [Rotating Device Identifier](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/rendezvous/DeviceDiscovery.adoc#245-rotating-device-identifier)).
    Then, instantiate a `DataProvider` object as described below.

    On Linux, define a `RotatingDeviceIdUniqueIdProvider` to provide the Casting
    Client's `RotatingDeviceIdUniqueId`, by implementing a
    `matter:casting::support::MutableByteSpanDataProvider`:

    ```c
    class RotatingDeviceIdUniqueIdProvider : public MutableByteSpanDataProvider {
    private:
        chip::MutableByteSpan rotatingDeviceIdUniqueIdSpan;
        uint8_t rotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];

    public:
        RotatingDeviceIdUniqueIdProvider() {
            // generate a random Unique ID for this example app for demonstration
            for (size_t i = 0; i < sizeof(rotatingDeviceIdUniqueId); i++) {
                rotatingDeviceIdUniqueId[i] = chip::Crypto::GetRandU8();
            }
            rotatingDeviceIdUniqueIdSpan = chip::MutableByteSpan(rotatingDeviceIdUniqueId);
        }

        chip::MutableByteSpan * Get() { return &rotatingDeviceIdUniqueIdSpan; }
    };
    ```

    On Android, define a `rotatingDeviceIdUniqueIdProvider` to provide the
    Casting Client's `RotatingDeviceIdUniqueId`, by implementing a
    `com.matter.casting.support.DataSource`:

    ```java
    private static final DataProvider<byte[]> rotatingDeviceIdUniqueIdProvider =
      new DataProvider<byte[]>() {
        private static final String ROTATING_DEVICE_ID_UNIQUE_ID =
            "EXAMPLE_IDENTIFIER"; // dummy value for demonstration only

        @Override
        public byte[] get() {
          return ROTATING_DEVICE_ID_UNIQUE_ID.getBytes();
        }
      };
    ```

    On iOS, define the
    `func castingAppDidReceiveRequestForRotatingDeviceIdUniqueId` in a class,
    `MTRAppParametersDataSource`, that implements the `MTRDataSource`:

    ```objectivec
    class MTRAppParametersDataSource : NSObject, MTRDataSource
    {
        func castingAppDidReceiveRequestForRotatingDeviceIdUniqueId(_ sender: Any) -> Data {
            // dummy value, with at least 16 bytes (ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength), for demonstration only
            return "0123456789ABCDEF".data(using: .utf8)!
        }
        ...
    }
    ```

2.  **Commissioning Data** - This object contains the passcode, discriminator,
    etc which identify the app and are provided to the `CastingPlayer` during
    the commissioning process. Refer to the Matter specification's
    [Onboarding Payload](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/qr_code/OnboardingPayload.adoc#ref_OnboardingPayload)
    section for details on commissioning data.

    On Linux, define a function `InitCommissionableDataProvider` to initialize
    initialize a `LinuxCommissionableDataProvider` that can provide the required
    values to the `CastingApp`.

    ```c
    CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, LinuxDeviceOptions & options) {
        chip::Optional<uint32_t> setupPasscode;

        if (options.payload.setUpPINCode != 0) {
            setupPasscode.SetValue(options.payload.setUpPINCode);
        } else if (!options.spake2pVerifier.HasValue()) {
            // default to TestOnlyCommissionableDataProvider for demonstration
            uint32_t defaultTestPasscode = 0;
            chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
            VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupPasscode(defaultTestPasscode) == CHIP_NO_ERROR);
            setupPasscode.SetValue(defaultTestPasscode);
            options.payload.setUpPINCode = defaultTestPasscode;
        }
        uint32_t spake2pIterationCount = chip::Crypto::kSpake2p_Min_PBKDF_Iterations;
        if (options.spake2pIterations != 0) {
            spake2pIterationCount = options.spake2pIterations;
        }
        return provider.Init(options.spake2pVerifier, options.spake2pSalt, spake2pIterationCount, setupPasscode,
                         options.payload.discriminator.GetLongValue());
    }
    ```

    On Android, define a `commissioningDataProvider` that can provide the
    required values to the `CastingApp`.

    ```java
    private static final DataProvider<CommissionableData> commissionableDataProvider =
      new DataProvider<CommissionableData>() {
        @Override
        public CommissionableData get() {
          // dummy values for demonstration only
          return new CommissionableData(20202021, 3874);
        }
      };
    ```

    On iOS, add a `func commissioningDataProvider` to the
    `MTRAppParametersDataSource` class defined above, that can provide the
    required values to the `MTRCastingApp`.

    ```objectivec
    func castingAppDidReceiveRequestForCommissionableData(_ sender: Any) -> MTRCommissionableData {
        // dummy values for demonstration only
        return MTRCommissionableData(
            passcode: 20202021,
            discriminator: 3874,
            spake2pIterationCount: 1000,
            spake2pVerifier: nil,
            spake2pSalt: nil)
    }
    ```

3.  **Device Attestation Credentials** - This object contains the
    `DeviceAttestationCertificate`, `ProductAttestationIntermediateCertificate`,
    etc. and implements a way to sign messages when called upon by the Matter TV
    Casting Library as part of the
    [Device Attestation process](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/device_attestation/Device_Attestation_Specification.adoc)
    during commissioning.

    On Linux, implement a define a `dacProvider` to provide the Casting Client's
    Device Attestation Credentials, by implementing a
    `chip::Credentials::DeviceAttestationCredentialsProvider`. For this example,
    we will use the `chip::Credentials::Examples::ExampleDACProvider`

    On Android, define a `dacProvider` to provide the Casting Client's Device
    Attestation Credentials, by implementing a
    `com.matter.casting.support.DACProvider`:

    ```java
    private static final DACProvider dacProvider = new DACProviderStub();
    private final static DataProvider<DeviceAttestationCredentials> dacProvider = new DataProvider<DeviceAttestationCredentials>() {
        private static final String kDevelopmentDAC_Cert_FFF1_8001 = "MIIB5z...<snipped>...CXE1M=";     // dummy values for demonstration only
        private static final String kDevelopmentDAC_PrivateKey_FFF1_8001 = "qrYAror...<snipped>...StE+/8=";
        private static final String KPAI_FFF1_8000_Cert_Array = "MIIByzC...<snipped>...pwP4kQ==";

        @Override
        public DeviceAttestationCredentials get() {
            DeviceAttestationCredentials deviceAttestationCredentials = new DeviceAttestationCredentials() {
            @Override
            public byte[] SignWithDeviceAttestationKey(byte[] message) {
                try {
    	        byte[] privateKeyBytes = Base64.decode(kDevelopmentDAC_PrivateKey_FFF1_8001, Base64.DEFAULT);
    	        AlgorithmParameters algorithmParameters = AlgorithmParameters.getInstance("EC");
    	        algorithmParameters.init(new ECGenParameterSpec("secp256r1"));
    	        ECParameterSpec parameterSpec = algorithmParameters.getParameterSpec(ECParameterSpec.class);
    	        ECPrivateKeySpec ecPrivateKeySpec = new ECPrivateKeySpec(new BigInteger(1, privateKeyBytes), parameterSpec);

    	        KeyFactory keyFactory = KeyFactory.getInstance("EC");
    	        PrivateKey privateKey = keyFactory.generatePrivate(ecPrivateKeySpec);

    	        Signature signature = Signature.getInstance("SHA256withECDSA");
    	        signature.initSign(privateKey);
    	        signature.update(message);

    	        return signature.sign();
                } catch (Exception e) {
    	        return null;
                }
            }
            };

            deviceAttestationCredentials.setDeviceAttestationCert(
            Base64.decode(kDevelopmentDAC_Cert_FFF1_8001, Base64.DEFAULT));
            deviceAttestationCredentials.setProductAttestationIntermediateCert(
            Base64.decode(KPAI_FFF1_8000_Cert_Array, Base64.DEFAULT));
            return deviceAttestationCredentials;
        }
    };
    ```

    On iOS, add functions
    `castingAppDidReceiveRequestForDeviceAttestationCredentials` and
    `didReceiveRequestToSignCertificateRequest` to the
    `MTRAppParametersDataSource` class defined above, that can return
    `MTRDeviceAttestationCredentials` and sign messages for the Casting Client,
    respectively.

    ```objectivec
    // dummy DAC values for demonstration only
    let kDevelopmentDAC_Cert_FFF1_8001: Data = Data(base64Encoded: "MIIB..<snipped>..CXE1M=")!;
    let kDevelopmentDAC_PrivateKey_FFF1_8001: Data = Data(base64Encoded: "qrYA<snipped>tE+/8=")!;
    let kDevelopmentDAC_PublicKey_FFF1_8001: Data = Data(base64Encoded: "BEY6<snipped>I=")!;
    let KPAI_FFF1_8000_Cert_Array: Data = Data(base64Encoded: "MIIB<snipped>4kQ==")!;
    let kCertificationDeclaration: Data = Data(base64Encoded: "MII<snipped>fA==")!;

    func castingAppDidReceiveRequestForDeviceAttestationCredentials(_ sender: Any) -> MTRDeviceAttestationCredentials {
        return MTRDeviceAttestationCredentials(
            certificationDeclaration: kCertificationDeclaration,
            firmwareInformation: Data(),
            deviceAttestationCert: kDevelopmentDAC_Cert_FFF1_8001,
            productAttestationIntermediateCert: KPAI_FFF1_8000_Cert_Array)
    }

    func castingApp(_ sender: Any, didReceiveRequestToSignCertificateRequest csrData: Data) -> Data {
        var privateKey = Data()
        privateKey.append(kDevelopmentDAC_PublicKey_FFF1_8001);
        privateKey.append(kDevelopmentDAC_PrivateKey_FFF1_8001);

        let privateKeyRef: SecKey = SecKeyCreateWithData(privateKey as NSData,
                                    [
                                        kSecAttrKeyType: kSecAttrKeyTypeECSECPrimeRandom,
                                        kSecAttrKeyClass: kSecAttrKeyClassPrivate,
                                        kSecAttrKeySizeInBits: 256
                                    ] as NSDictionary, nil)!

        let _:Unmanaged<SecKey> = Unmanaged<SecKey>.passRetained(privateKeyRef);

        // use SecKey above to sign csrData and return resulting value
    }
    ```

Once you have created the `DataProvider` objects above, you are ready to
initialize the Casting App as described below. Note: When you initialize the
Casting client, make sure your code initializes it only once, before it starts a
Matter casting session.

On Linux, create an `AppParameters` object using the
`RotatingDeviceIdUniqueIdProvider`, `LinuxCommissionableDataProvider`,
`CommonCaseDeviceServerInitParamsProvider`, `ExampleDACProvider` and
`DefaultDACVerifier`, and call `CastingApp::GetInstance()->Initialize` with it.

```c
LinuxCommissionableDataProvider gCommissionableDataProvider;
int main(int argc, char * argv[]) {
    // Create AppParameters that need to be passed to CastingApp.Initialize()
    AppParameters appParameters;
    RotatingDeviceIdUniqueIdProvider rotatingDeviceIdUniqueIdProvider;
    CommonCaseDeviceServerInitParamsProvider serverInitParamsProvider;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = InitCommissionableDataProvider(gCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
    VerifyOrReturnValue(
        err == CHIP_NO_ERROR, 0,
        ChipLogError(AppServer, "Initialization of CommissionableDataProvider failed %" CHIP_ERROR_FORMAT, err.Format()));
    err = appParameters.Create(&rotatingDeviceIdUniqueIdProvider, &gCommissionableDataProvider,
                               chip::Credentials::Examples::GetExampleDACProvider(),
                               GetDefaultDACVerifier(chip::Credentials::GetTestAttestationTrustStore()), &serverInitParamsProvider);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                        ChipLogError(AppServer, "Creation of AppParameters failed %" CHIP_ERROR_FORMAT, err.Format()));

    // Initialize the CastingApp
    err = CastingApp::GetInstance()->Initialize(appParameters);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                        ChipLogError(AppServer, "Initialization of CastingApp failed %" CHIP_ERROR_FORMAT, err.Format()));

    // Initialize Linux KeyValueStoreMgr
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);

    // Start the CastingApp
    err = CastingApp::GetInstance()->Start();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                        ChipLogError(AppServer, "CastingApp::Start failed %" CHIP_ERROR_FORMAT, err.Format()));
    ...
}
```

On Android, create an `AppParameters` object using the
`rotatingDeviceIdUniqueIdProvider`, `commissioningDataProvider`, `dacProvider`
and `DataProvider<ConfigurationManager>`, and call
`CastingApp.getInstance().initialize` with it.

```java
public static MatterError initAndStart(Context applicationContext) {
    // Create an AppParameters object to pass in global casting parameters to the SDK
    final AppParameters appParameters =
        new AppParameters(
            applicationContext,
            new DataProvider<ConfigurationManager>() {
              @Override
              public ConfigurationManager get() {
                return new PreferencesConfigurationManager(
                    applicationContext, "chip.platform.ConfigurationManager");
              }
            },
            rotatingDeviceIdUniqueIdProvider,
            commissionableDataProvider,
            dacProvider);

    // Initialize the SDK using the appParameters and check if it returns successfully
    MatterError err = CastingApp.getInstance().initialize(appParameters);
    if (err.hasError()) {
      Log.e(TAG, "Failed to initialize Matter CastingApp");
      return err;
    }

    // Start the CastingApp
    err = CastingApp.getInstance().start();
    if (err.hasError()) {
      Log.e(TAG, "Failed to start Matter CastingApp");
      return err;
    }
    return err;
}
```

On iOS, call `MTRCastingApp.initialize` with an object of the
`MTRAppParametersDataSource`.

```objectivec
func initialize() -> MatterError {
    if let castingApp = MTRCastingApp.getSharedInstance() {
        return castingApp.initialize(with: MTRAppParametersDataSource())
    } else {
        return MATTER_ERROR_INCORRECT_STATE
    }
}
```

### Discover Casting Players

_{Complete Discovery examples: [Linux](linux/simple-app-helper.cpp)}_

The Casting Client discovers `CastingPlayers` using Matter Commissioner
discovery over DNS-SD by listening for `CastingPlayer` events as they are
discovered, updated, or lost from the network.

On Linux, define a `DiscoveryDelegateImpl` that implements the
`matter::casting::core::DiscoveryDelegate`.

```c
class DiscoveryDelegateImpl : public DiscoveryDelegate {
private:
    int commissionersCount = 0;

public:
    void HandleOnAdded(matter::casting::memory::Strong<CastingPlayer> player) override {
        if (commissionersCount == 0) {
            ChipLogProgress(AppServer, "Select discovered CastingPlayer to request commissioning");
            ChipLogProgress(AppServer, "Example: cast request 0");
        }
        ++commissionersCount;
        ChipLogProgress(AppServer, "Discovered CastingPlayer #%d", commissionersCount);
        player->LogDetail();
    }

    void HandleOnUpdated(matter::casting::memory::Strong<CastingPlayer> player) override {
        ChipLogProgress(AppServer, "Updated CastingPlayer with ID: %s", player->GetId());
    }
};
```

Finally, register these listeners and start discovery.

On Linux, register an instance of the `DiscoveryDelegateImpl` with
`matter::casting::core::CastingPlayerDiscovery` by calling `SetDelegate` on its
singleton instance. Then, call `StartDiscovery` by optionally specifying the
`kTargetPlayerDeviceType` to filter results by.

```c
const uint64_t kTargetPlayerDeviceType = 35; // 35 represents device type of Matter Video Player
...
...
DiscoveryDelegateImpl delegate;
CastingPlayerDiscovery::GetInstance()->SetDelegate(&delegate);
VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                    ChipLogError(AppServer, "CastingPlayerDiscovery::SetDelegate failed %" CHIP_ERROR_FORMAT, err.Format()));

err = CastingPlayerDiscovery::GetInstance()->StartDiscovery(kTargetPlayerDeviceType);
VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                    ChipLogError(AppServer, "CastingPlayerDiscovery::StartDiscovery failed %" CHIP_ERROR_FORMAT, err.Format()));
chip::DeviceLayer::PlatformMgr().RunEventLoop();
...
```

### Connect to a Casting Player

_{Complete Connection examples: [Linux](linux/simple-app-helper.cpp)}_

Each `CastingPlayer` object created during
[Discovery](#discover-casting-players) contains information such as
`deviceName`, `vendorId`, `productId`, etc. which can help the user pick the
right `CastingPlayer`. A Casting Client can attempt to connect to the
`selectedCastingPlayer` using
[Matter User Directed Commissioning (UDC)](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/rendezvous/UserDirectedCommissioning.adoc).
The Matter TV Casting library locally caches information required to reconnect
to a `CastingPlayer`, once the Casting client has been commissioned by it. After
that, the Casting client is able to skip the full UDC process by establishing
CASE with the `CastingPlayer` directly. Once connected, the `CastingPlayer`
object will contain the list of available Endpoints on that `CastingPlayer`.
Optionally, the following arguments may also be passed in. The optional
`commissioningWindowTimeoutSec` indicates how long to keep the commissioning
window open, if commissioning is required. And `DesiredEndpointFilter` specifies
the attributes, such as Vendor ID and Product ID of the `Endpoint`, the Casting
client desires to interact with after connecting. This forces the Matter TV
Casting library to go through the full UDC process in search of the desired
Endpoint, in cases where it is not available in the Casting client's cache.

On Linux, the Casting Client can connect to a `CastingPlayer` by successfully
calling `VerifyOrEstablishConnection` on it.

```c

const uint16_t kDesiredEndpointVendorId = 65521;

void ConnectionHandler(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer)
{
    ChipLogProgress(AppServer, "ConnectionHandler called with %" CHIP_ERROR_FORMAT, err.Format());
}

...
// targetCastingPlayer is a discovered CastingPlayer
matter::casting::core::EndpointFilter desiredEndpointFilter;
desiredEndpointFilter.vendorId = kDesiredEndpointVendorId;
targetCastingPlayer->VerifyOrEstablishConnection(ConnectionHandler,
                                                    matter::casting::core::kCommissioningWindowTimeoutSec,
                                                    desiredEndpointFilter);
...
```

### Select an Endpoint on the Casting Player

## Interacting with a Casting Endpoint

### Issuing Commands

### Read Operations

### Subscriptions
