# CHIP NXP Unit Tests Application

## How to build NXP matter unit tests application

1. Make sure to follow shared build instructions from
   [MATTER NXP Examples Guide for FreeRTOS platforms](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md#set-up-the-build-environment)

2. Run west build command:
   `west build -d <folder> -b <board> src/test_driver/nxp/`

## How to update the list of executed tests

To modify the list of executed tests, please update the test_components.txt file
with adding/removing test component library names.

Available Matter test component libraries:

```
libAppDataModelTests
libAppClusterBuildingBlockTests
libIMInterfaceTests
libICDServerTests
libChipCryptoTests
libInetLayerTests
libAddressResolveTests
libASN1Tests
libCoreTests
libFormatTests
libMessagingTestHelpers
libBDXTests
libInteractionModelTests
libSecureChannelTests
libUserDirectedCommissioningTests
libRetransmitTests
libTransportLayerTests
libMinimalMdnsCoreTests
libMinimalMdnsRespondersTests
libMinimalMdnstests
libMdnsTests
libMinimalMdnsRecordsTests
libCodegenDataModelProviderTests
libSetupPayloadTests
libRawTransportTests
libAppTests
libCertTestVectors
libSupportTests
libPlatformTests
libBleLayerTests
libControllerDataModelTests
libControllerTests
libTestShell
```
