# Integration Test utilities

There are several test utilities that can be used to simulate or force behavior
on devices for the purposes of testing.

When using any of these utilities it is important to inject the errors at the
point where they are running through the MOST code that they can.

If the cluster uses the
[ClusterLogic](../cluster_and_device_type_dev/unit_testing_clusters.md) pattern,
this means injecting errors as close as possible to the driver layer, rather
than catching errors in the server.

## TestEventTriggers

TestEventTriggers are used to test interactions on the DUT that are difficult to
perform during certification testing (ex. triggering a smoke alarm)

**These should be used sparingly!**

TestEventTriggers are started though a command in the General Diagnostics
cluster. The command takes a “test key” and a “trigger code” to request that a
device to perform a specific action. Currently most devices use a default key,
but it can be overridden by a specific device if required.

**TestEventTriggers need to be turned off outside of certification tests**

To use these triggers:

-   Derive from
    [TestEventTriggerHandler](https://github.com/project-chip/connectedhomeip/blob/master/src/app/TestEventTriggerDelegate.h)
-   Implement HandleEventTrigger function
-   Register with TestEventTriggerDelegate::AddHandler

Please see
[EnergyEvseTestEventTriggerHandler](https://github.com/project-chip/connectedhomeip/blob/master/src/app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h)
for a good example.

## NamedPipes

NamedPipes are used to trigger actions on Linux applications. These can be used
in the CI, and are normally used to simulate manual actions for CI integration.
Any required manual action in a test (ex. push a button) should have a
corresponding NamedPipe action to allow the test to run in the CI.

In python tests, the app-pid required to access the named pipe can be passed in
as a flag (--app-pid).

NamedPipes are implemented in
[NamedPipeCommands.h](https://github.com/project-chip/connectedhomeip/blob/master/examples/platform/linux/NamedPipeCommands.h)

To use NamedPipes

-   Derive from NamedPipeCommandDelegate
-   Implement the OnEventCommandReceived(const char \* json) function
-   Instantiate and start a NamedPipeCommands object to receive commands and
    pass in the NamedPipeCommandDelegate and a file path base name
-   (while running) Write to the file (baseName_pid) to trigger the actions

For a good example, see Air Quality:

-   [Delegate](https://github.com/project-chip/connectedhomeip/blob/master/examples/air-quality-sensor-app/linux/AirQualitySensorAppAttrUpdateDelegate.cpp)
-   [main](https://github.com/project-chip/connectedhomeip/blob/master/examples/air-quality-sensor-app/linux/main.cpp)
-   [README](https://github.com/project-chip/connectedhomeip/blob/master/examples/air-quality-sensor-app/linux/README.md)

[RVC Clean Mode](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/TC_RVCCLEANM_2_1.py)
gives an example of how to use named pipes in testing.

## Fault Injection

Fault injection is used to inject conditional code paths at runtime, e.g.
errors. This is very useful for things like client testing, to check error
handling for paths that are difficult to hit under normal operating conditions.

The fault injection framework we are currently using is nlFaultInjection.The
framework uses a macro for injecting the code paths, and the macro is set to a
no-op if the option is turned off at compile time. The build option to turn on
fault inject is `chip_with_nlfaultinjection`.

Fault injection has been plumbed out through a manufacturer-specific
[fault injection cluster](#fault-injection-cluster) that is available in the
SDK. This allows fault injection to be turned on and off using standard cluster
mechanisms during testing. For certification, operating these using a secondary,
non-DUT controller is recommended. For a good example of this, please see
[TC-IDM-1.3](https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#tc-idm-1-3-batched-commands-invoke-request-action-from-dut-to-th-dut_client).

The nlFaultInjection allows the application to define multiple managers. In the
SDK, we have managers for System, inet and CHIP. CHIP should be used for
anything above the system layer (basically all new cluster development). The
CHIP fault manager is available at
[lib/support/CHIPFaultInjection.h](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/CHIPFaultInjection.h).

To add new fault injection code paths:

-   Add new IDs (aFaultID) to the enum in
    [CHIPFaultInjection](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/CHIPFaultInjection.h)
-   add CHIP_FAULT_INJECT(aFaultID, aStatements) at the point where the fault
    injection should occur

### Fault Injection example

```
CHIP_ERROR CASEServer::OnMessageReceived(Messaging::ExchangeContext * ec,
   const PayloadHeader & payloadHeader,
                                        System::PacketBufferHandle && payload)
{
   MATTER_TRACE_SCOPE("OnMessageReceived", "CASEServer");

   bool busy = GetSession().GetState() != CASESession::State::kInitialized;
   CHIP_FAULT_INJECT(FaultInjection::kFault_CASEServerBusy, busy = true);
   if (busy)
   {
…
```

### Fault Injection cluster

The Fault injection cluster is a manufacturer-specific cluster, available in the
SDK (0xFFF1FC06).

-   [server](https://github.com/project-chip/connectedhomeip/blob/master/src/app/clusters/fault-injection-server/fault-injection-server.cpp)
-   [xml](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap-templates/zcl/data-model/chip/fault-injection-cluster.xml)

Example apps can be compiled with this cluster for client-side certification and
integration tests.

To use this cluster to turn on a fault, use the FailAtFault command:

-   Type: FaultType - use FaultType::kChipFault (0x03)
-   Id: int32u - match the ID you set up for your fault
-   NumCallsToSkip: int32u - number of times to run normally
-   NumCallsToFail: int32u - number of times to hit the fault injection
    condition after NumCallsToSkip
-   TakeMutex: bool - controls access to the fault injection manager for
    multi-threaded systems. False is fine.
