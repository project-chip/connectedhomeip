# ChipDeviceCtrl.py API

* [chip.ChipDeviceCtrl](#chipChipDeviceCtrl)
  * [RegisterOnActiveCallback](#chipChipDeviceCtrl.RegisterOnActiveCallback)
  * [UnregisterOnActiveCallback](#chipChipDeviceCtrl.UnregisterOnActiveCallback)
  * [WaitForCheckIn](#chipChipDeviceCtrl.WaitForCheckIn)
  * [CallbackContext](#chipChipDeviceCtrl.CallbackContext)
  * [CommissioningContext](#chipChipDeviceCtrl.CommissioningContext)
  * [CommissionableNode](#chipChipDeviceCtrl.CommissionableNode)
    * [Commission](#chipChipDeviceCtrl.CommissionableNode.Commission)
  * [DeviceProxyWrapper](#chipChipDeviceCtrl.DeviceProxyWrapper)
  * [ChipDeviceControllerBase](#chipChipDeviceCtrl.ChipDeviceControllerBase)
    * [Shutdown](#chipChipDeviceCtrl.ChipDeviceControllerBase.Shutdown)
    * [ShutdownAll](#chipChipDeviceCtrl.ChipDeviceControllerBase.ShutdownAll)
    * [CheckIsActive](#chipChipDeviceCtrl.ChipDeviceControllerBase.CheckIsActive)
    * [IsConnected](#chipChipDeviceCtrl.ChipDeviceControllerBase.IsConnected)
    * [ConnectBLE](#chipChipDeviceCtrl.ChipDeviceControllerBase.ConnectBLE)
    * [UnpairDevice](#chipChipDeviceCtrl.ChipDeviceControllerBase.UnpairDevice)
    * [CloseBLEConnection](#chipChipDeviceCtrl.ChipDeviceControllerBase.CloseBLEConnection)
    * [ExpireSessions](#chipChipDeviceCtrl.ChipDeviceControllerBase.ExpireSessions)
    * [MarkSessionDefunct](#chipChipDeviceCtrl.ChipDeviceControllerBase.MarkSessionDefunct)
    * [MarkSessionForEviction](#chipChipDeviceCtrl.ChipDeviceControllerBase.MarkSessionForEviction)
    * [DeleteAllSessionResumptionStorage](#chipChipDeviceCtrl.ChipDeviceControllerBase.DeleteAllSessionResumptionStorage)
    * [GetTestCommissionerUsed](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetTestCommissionerUsed)
    * [SetTestCommissionerSimulateFailureOnStage](#chipChipDeviceCtrl.ChipDeviceControllerBase.SetTestCommissionerSimulateFailureOnStage)
    * [SetTestCommissionerSimulateFailureOnReport](#chipChipDeviceCtrl.ChipDeviceControllerBase.SetTestCommissionerSimulateFailureOnReport)
    * [SetTestCommissionerPrematureCompleteAfter](#chipChipDeviceCtrl.ChipDeviceControllerBase.SetTestCommissionerPrematureCompleteAfter)
    * [CheckTestCommissionerCallbacks](#chipChipDeviceCtrl.ChipDeviceControllerBase.CheckTestCommissionerCallbacks)
    * [CheckStageSuccessful](#chipChipDeviceCtrl.ChipDeviceControllerBase.CheckStageSuccessful)
    * [CheckTestCommissionerPaseConnection](#chipChipDeviceCtrl.ChipDeviceControllerBase.CheckTestCommissionerPaseConnection)
    * [GetAddressAndPort](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetAddressAndPort)
    * [DiscoverCommissionableNodes](#chipChipDeviceCtrl.ChipDeviceControllerBase.DiscoverCommissionableNodes)
    * [GetDiscoveredDevices](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetDiscoveredDevices)
    * [GetIPForDiscoveredDevice](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetIPForDiscoveredDevice)
    * [OpenCommissioningWindow](#chipChipDeviceCtrl.ChipDeviceControllerBase.OpenCommissioningWindow)
    * [GetCompressedFabricId](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetCompressedFabricId)
    * [GetFabricIdInternal](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetFabricIdInternal)
    * [GetFabricIndexInternal](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetFabricIndexInternal)
    * [GetNodeIdInternal](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetNodeIdInternal)
    * [GetRootPublicKeyBytesInternal](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetRootPublicKeyBytesInternal)
    * [GetClusterHandler](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetClusterHandler)
    * [FindOrEstablishPASESession](#chipChipDeviceCtrl.ChipDeviceControllerBase.FindOrEstablishPASESession)
    * [GetConnectedDeviceSync](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetConnectedDeviceSync)
    * [WaitForActive](#chipChipDeviceCtrl.ChipDeviceControllerBase.WaitForActive)
    * [GetConnectedDevice](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetConnectedDevice)
    * [ComputeRoundTripTimeout](#chipChipDeviceCtrl.ChipDeviceControllerBase.ComputeRoundTripTimeout)
    * [GetRemoteSessionParameters](#chipChipDeviceCtrl.ChipDeviceControllerBase.GetRemoteSessionParameters)
    * [TestOnlySendBatchCommands](#chipChipDeviceCtrl.ChipDeviceControllerBase.TestOnlySendBatchCommands)
    * [TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke](#chipChipDeviceCtrl.ChipDeviceControllerBase.TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke)
    * [SendCommand](#chipChipDeviceCtrl.ChipDeviceControllerBase.SendCommand)
    * [SendBatchCommands](#chipChipDeviceCtrl.ChipDeviceControllerBase.SendBatchCommands)
    * [SendGroupCommand](#chipChipDeviceCtrl.ChipDeviceControllerBase.SendGroupCommand)
    * [WriteAttribute](#chipChipDeviceCtrl.ChipDeviceControllerBase.WriteAttribute)
    * [WriteGroupAttribute](#chipChipDeviceCtrl.ChipDeviceControllerBase.WriteGroupAttribute)
    * [TestOnlyPrepareToReceiveBdxData](#chipChipDeviceCtrl.ChipDeviceControllerBase.TestOnlyPrepareToReceiveBdxData)
    * [TestOnlyPrepareToSendBdxData](#chipChipDeviceCtrl.ChipDeviceControllerBase.TestOnlyPrepareToSendBdxData)
    * [Read](#chipChipDeviceCtrl.ChipDeviceControllerBase.Read)
    * [ReadAttribute](#chipChipDeviceCtrl.ChipDeviceControllerBase.ReadAttribute)
    * [ReadEvent](#chipChipDeviceCtrl.ChipDeviceControllerBase.ReadEvent)
    * [SetIpk](#chipChipDeviceCtrl.ChipDeviceControllerBase.SetIpk)
    * [InitGroupTestingData](#chipChipDeviceCtrl.ChipDeviceControllerBase.InitGroupTestingData)
    * [CreateManualCode](#chipChipDeviceCtrl.ChipDeviceControllerBase.CreateManualCode)
  * [ChipDeviceController](#chipChipDeviceCtrl.ChipDeviceController)
    * [Commission](#chipChipDeviceCtrl.ChipDeviceController.Commission)
    * [CommissionThread](#chipChipDeviceCtrl.ChipDeviceController.CommissionThread)
    * [CommissionWiFi](#chipChipDeviceCtrl.ChipDeviceController.CommissionWiFi)
    * [SetWiFiCredentials](#chipChipDeviceCtrl.ChipDeviceController.SetWiFiCredentials)
    * [SetThreadOperationalDataset](#chipChipDeviceCtrl.ChipDeviceController.SetThreadOperationalDataset)
    * [ResetCommissioningParameters](#chipChipDeviceCtrl.ChipDeviceController.ResetCommissioningParameters)
    * [SetTimeZone](#chipChipDeviceCtrl.ChipDeviceController.SetTimeZone)
    * [SetDSTOffset](#chipChipDeviceCtrl.ChipDeviceController.SetDSTOffset)
    * [SetTCAcknowledgements](#chipChipDeviceCtrl.ChipDeviceController.SetTCAcknowledgements)
    * [SetSkipCommissioningComplete](#chipChipDeviceCtrl.ChipDeviceController.SetSkipCommissioningComplete)
    * [SetDefaultNTP](#chipChipDeviceCtrl.ChipDeviceController.SetDefaultNTP)
    * [SetTrustedTimeSource](#chipChipDeviceCtrl.ChipDeviceController.SetTrustedTimeSource)
    * [SetCheckMatchingFabric](#chipChipDeviceCtrl.ChipDeviceController.SetCheckMatchingFabric)
    * [GenerateICDRegistrationParameters](#chipChipDeviceCtrl.ChipDeviceController.GenerateICDRegistrationParameters)
    * [EnableICDRegistration](#chipChipDeviceCtrl.ChipDeviceController.EnableICDRegistration)
    * [DisableICDRegistration](#chipChipDeviceCtrl.ChipDeviceController.DisableICDRegistration)
    * [GetFabricCheckResult](#chipChipDeviceCtrl.ChipDeviceController.GetFabricCheckResult)
    * [CommissionOnNetwork](#chipChipDeviceCtrl.ChipDeviceController.CommissionOnNetwork)
    * [get\_rcac](#chipChipDeviceCtrl.ChipDeviceController.get_rcac)
    * [CommissionWithCode](#chipChipDeviceCtrl.ChipDeviceController.CommissionWithCode)
    * [CommissionIP](#chipChipDeviceCtrl.ChipDeviceController.CommissionIP)
    * [NOCChainCallback](#chipChipDeviceCtrl.ChipDeviceController.NOCChainCallback)
    * [IssueNOCChain](#chipChipDeviceCtrl.ChipDeviceController.IssueNOCChain)
    * [SetDACRevocationSetPath](#chipChipDeviceCtrl.ChipDeviceController.SetDACRevocationSetPath)
  * [BareChipDeviceController](#chipChipDeviceCtrl.BareChipDeviceController)
    * [\_\_init\_\_](#chipChipDeviceCtrl.BareChipDeviceController.__init__)

# chip.ChipDeviceCtrl

Chip Device Controller interface

#### RegisterOnActiveCallback

```python
def RegisterOnActiveCallback(scopedNodeId: ScopedNodeId,
                             callback: typing.Callable[[ScopedNodeId], None])
```

Registers a callback when the device with given (fabric index, node id) becomes active.

Does nothing if the callback is already registered.

#### UnregisterOnActiveCallback

```python
def UnregisterOnActiveCallback(scopedNodeId: ScopedNodeId,
                               callback: typing.Callable[[ScopedNodeId],
                                                         None])
```

Unregisters a callback when the device with given (fabric index, node id) becomes active.

Does nothing if the callback has not been registered.

#### WaitForCheckIn

```python
async def WaitForCheckIn(scopedNodeId: ScopedNodeId, timeoutSeconds: float)
```

Waits for a device becomes active.

**Returns**:

  - A future, completes when the device becomes active.

## CallbackContext

```python
class CallbackContext()
```

A context manager for handling callbacks that are expected to be called exactly once.

The context manager makes sure that no concurrent operations which use the same callback
handlers are executed.

## CommissioningContext

```python
class CommissioningContext(CallbackContext)
```

A context manager for handling commissioning callbacks that are expected to be called exactly once.

This context also resets commissioning related device controller state.

## CommissionableNode

```python
class CommissionableNode(discovery.CommissionableNode)
```

#### Commission

```python
def Commission(nodeId: int, setupPinCode: int) -> int
```

Commission the device using the device controller discovered this device.

nodeId: The nodeId commissioned to the device
setupPinCode: The setup pin code of the device

**Returns**:

  - Effective Node ID of the device (as defined by the assigned NOC)

## DeviceProxyWrapper

```python
class DeviceProxyWrapper()
```

Encapsulates a pointer to OperationalDeviceProxy on the c++ side that needs to be
freed when DeviceProxyWrapper goes out of scope. There is a potential issue where
if this is copied around that a double free will occur, but how this is used today
that is not an issue that needs to be accounted for and it will become very apparent
if that happens.

## ChipDeviceControllerBase

```python
class ChipDeviceControllerBase()
```

#### Shutdown

```python
def Shutdown()
```

Shuts down this controller and reclaims any used resources, including the bound
C++ constructor instance in the SDK.

**Raises**:

- `ChipStackError` - On failure.
  

**Returns**:

  None

#### ShutdownAll

```python
def ShutdownAll()
```

Shut down all active controllers and reclaim any used resources.

#### CheckIsActive

```python
def CheckIsActive()
```

Checks if the device controller is active.

**Raises**:

- `RuntimeError` - On failure.

#### IsConnected

```python
def IsConnected()
```

Checks if the device controller is connected.

**Returns**:

- `bool` - True if is connected, False if not connected.
  

**Raises**:

- `RuntimeError` - If '_isActive' is False (from the call to CheckIsActive).

#### ConnectBLE

```python
async def ConnectBLE(discriminator: int,
                     setupPinCode: int,
                     nodeid: int,
                     isShortDiscriminator: bool = False) -> int
```

Connect to a BLE device via PASE using the given discriminator and setup pin code.

**Returns**:

  Effective Node ID of the device (as defined by the assigned NOC)

#### UnpairDevice

```python
async def UnpairDevice(nodeid: int) -> None
```

Unpairs the device with the specified node ID.

**Returns**:

  None.

#### CloseBLEConnection

```python
def CloseBLEConnection()
```

Closes the BLE connection for the device controller.

**Raises**:

- `ChipStackError` - On failure.

#### ExpireSessions

```python
def ExpireSessions(nodeid)
```

Close all sessions with `nodeid` (if any existed) so that sessions get re-established.

This is needed to properly handle operations that invalidate a node's state, such as
UpdateNOC.

WARNING: ONLY CALL THIS IF YOU UNDERSTAND THE SIDE-EFFECTS

**Raises**:

- `ChipStackError` - On failure.

#### MarkSessionDefunct

```python
def MarkSessionDefunct(nodeid)
```

Marks a previously active session with the specified node as defunct to temporarily prevent it
from being used with new exchanges to send or receive messages. This should be called when there
is suspicion of a loss-of-sync with the session state on the associated peer, such as evidence
of transport failure.

If messages are received thereafter on this session, the session will be put back into the Active state.

This function should only be called on an active session.
This will NOT detach any existing SessionHolders.

**Arguments**:

- `nodeid` _int_ - The node ID of the device whose session should be marked as defunct.
  

**Raises**:

- `RuntimeError` - If the controller is not active.
- `PyChipError` - If the operation fails.

#### MarkSessionForEviction

```python
def MarkSessionForEviction(nodeid)
```

Marks the session with the specified node for eviction. It will first detach all SessionHolders
attached to this session by calling 'OnSessionReleased' on each of them. This will force them
to release their reference to the session. If there are no more references left, the session
will then be de-allocated.

Once marked for eviction, the session SHALL NOT ever become active again.

**Arguments**:

- `nodeid` _int_ - The node ID of the device whose session should be marked for eviction.
  

**Raises**:

- `RuntimeError` - If the controller is not active.
- `PyChipError` - If the operation fails.

#### DeleteAllSessionResumptionStorage

```python
def DeleteAllSessionResumptionStorage()
```

Remove all session resumption information associated with the fabric index of the controller.

**Raises**:

- `RuntimeError` - If the controller is not active.
- `PyChipError` - If the operation fails.

#### GetTestCommissionerUsed

```python
def GetTestCommissionerUsed()
```

Get the status of test commissioner in use.

**Returns**:

- `bool` - True if the test commissioner is in use, False if not.

#### SetTestCommissionerSimulateFailureOnStage

```python
def SetTestCommissionerSimulateFailureOnStage(stage: int)
```

Simulates a failure on a specific stage of the test commissioner.

**Returns**:

- `bool` - True if the failure simulate success, False if not.

#### SetTestCommissionerSimulateFailureOnReport

```python
def SetTestCommissionerSimulateFailureOnReport(stage: int)
```

Simulates a failure on report of the test commissioner.

**Returns**:

- `bool` - True if the failure simulate success, False if not.

#### SetTestCommissionerPrematureCompleteAfter

```python
def SetTestCommissionerPrematureCompleteAfter(stage: int)
```

Premature complete of the test commissioner.

**Returns**:

- `bool` - True if the premature complete success, False if not.

#### CheckTestCommissionerCallbacks

```python
def CheckTestCommissionerCallbacks()
```

Check the test commissioner callbacks.

**Returns**:

- `bool` - True if the test commissioner callbacks success, False if not.

#### CheckStageSuccessful

```python
def CheckStageSuccessful(stage: int)
```

Check the test commissioner stage sucess.

**Returns**:

- `bool` - True if test commissioner stage success, False if not.

#### CheckTestCommissionerPaseConnection

```python
def CheckTestCommissionerPaseConnection(nodeid)
```

Check the test commissioner Pase connection sucess.

**Returns**:

- `bool` - True if test commissioner Pase connection success, False if not.

#### GetAddressAndPort

```python
def GetAddressAndPort(nodeid)
```

Get the address and port.

**Returns**:

- `tuple` - The address and port if no error occurs or None on failure.

#### DiscoverCommissionableNodes

```python
async def DiscoverCommissionableNodes(
    filterType: discovery.FilterType = discovery.FilterType.NONE,
    filter: typing.Any = None,
    stopOnFirst: bool = False,
    timeoutSecond: int = 5
) -> typing.Union[None, CommissionableNode, typing.List[CommissionableNode]]
```

Discover commissionable nodes via DNS-SD with specified filters.
Supported filters are:

discovery.FilterType.NONE
discovery.FilterType.SHORT_DISCRIMINATOR
discovery.FilterType.LONG_DISCRIMINATOR
discovery.FilterType.VENDOR_ID
discovery.FilterType.DEVICE_TYPE
discovery.FilterType.COMMISSIONING_MODE
discovery.FilterType.INSTANCE_NAME
discovery.FilterType.COMMISSIONER
discovery.FilterType.COMPRESSED_FABRIC_ID

This function will always return a list of CommissionableDevice. When stopOnFirst is set,
this function will return when at least one device is discovered or on timeout.

**Returns**:

- `list` - A list of discovered devices.

#### GetDiscoveredDevices

```python
async def GetDiscoveredDevices()
```

Get the discovered devices.

**Returns**:

- `list` - A list of discovered devices.

#### GetIPForDiscoveredDevice

```python
def GetIPForDiscoveredDevice(idx, addrStr, length)
```

Get the IP address for a discovered device.

**Returns**:

- `bool` - True if IP for discovered device success, False if not.

#### OpenCommissioningWindow

```python
async def OpenCommissioningWindow(
        nodeid: int, timeout: int, iteration: int, discriminator: int,
        option: CommissioningWindowPasscode) -> CommissioningParameters
```

Opens a commissioning window on the device with the given nodeid.
nodeid:        Node id of the device
timeout:       Command timeout
iteration:     The PAKE iteration count associated with the PAKE Passcode ID and ephemeral
PAKE passcode verifier to be used for this commissioning. Valid range: 1000 - 100000
Ignored if option == 0
discriminator: The long discriminator for the DNS-SD advertisement. Valid range: 0-4095
Ignored if option == 0
option:        0 = kOriginalSetupCode
1 = kTokenWithRandomPIN

**Returns**:

  CommissioningParameters

#### GetCompressedFabricId

```python
def GetCompressedFabricId()
```

Get compressed fabric Id.

**Returns**:

- `int` - The compressed fabric ID as a 64-bit integer.
  

**Raises**:

- `ChipStackError` - On failure.

#### GetFabricIdInternal

```python
def GetFabricIdInternal() -> int
```

Get the fabric ID from the object. Only used to validate cached value from property.

**Returns**:

- `int` - The raw fabric ID as a 64-bit integer.
  

**Raises**:

- `ChipStackError` - On failure.

#### GetFabricIndexInternal

```python
def GetFabricIndexInternal() -> int
```

Get the fabric index from the object. Only used to validate cached value from property.

**Returns**:

- `int` - fabric index in local fabric table associated with this controller.
  

**Raises**:

- `ChipStackError` - On failure.

#### GetNodeIdInternal

```python
def GetNodeIdInternal() -> int
```

Get the node ID from the object. Only used to validate cached value from property.

**Returns**:

- `int` - The Node ID as a 64 bit integer.
  

**Raises**:

- `ChipStackError` - On failure.

#### GetRootPublicKeyBytesInternal

```python
def GetRootPublicKeyBytesInternal() -> bytes
```

Get the root public key associated with our fabric.

**Returns**:

- `bytes` - The root public key raw bytes in uncompressed point form.
  

**Raises**:

- `ChipStackError` - On failure.

#### GetClusterHandler

```python
def GetClusterHandler()
```

Get cluster handler

**Returns**:

- `ChipClusters` - An instance of the ChipClusters class.

#### FindOrEstablishPASESession

```python
async def FindOrEstablishPASESession(setupCode: str,
                                     nodeid: int,
                                     timeoutMs: typing.Optional[int] = None
                                     ) -> typing.Optional[DeviceProxyWrapper]
```

Returns CommissioneeDeviceProxy if we can find or establish a PASE connection to the specified device

**Returns**:

  DeviceProxyWrapper on success, if not is None.

#### GetConnectedDeviceSync

```python
def GetConnectedDeviceSync(
        nodeid,
        allowPASE=True,
        timeoutMs: typing.Optional[int] = None,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Gets an OperationalDeviceProxy or CommissioneeDeviceProxy for the specified Node.

Arg:
nodeId: Target's Node ID
allowPASE: Get a device proxy of a device being commissioned.
timeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.

**Returns**:

  DeviceProxyWrapper on success.

#### WaitForActive

```python
async def WaitForActive(nodeid,
                        *,
                        timeoutSeconds=30.0,
                        stayActiveDurationMs=30000)
```

Waits a LIT ICD device to become active. Will send a StayActive command to the device on active to allow human operations.

**Arguments**:

- `nodeId` - Node ID of the LID ICD
- `stayActiveDurationMs` - The duration in the StayActive command, in milliseconds
  

**Returns**:

  StayActiveResponse on success

#### GetConnectedDevice

```python
async def GetConnectedDevice(
        nodeid,
        allowPASE: bool = True,
        timeoutMs: typing.Optional[int] = None,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Gets an OperationalDeviceProxy or CommissioneeDeviceProxy for the specified Node.

**Arguments**:

- `nodeId` - Target's Node ID
- `allowPASE` - Get a device proxy of a device being commissioned.
- `timeoutMs` - Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
  

**Returns**:

  DeviceProxyWrapper on success

#### ComputeRoundTripTimeout

```python
def ComputeRoundTripTimeout(nodeid, upperLayerProcessingTimeoutMs: int = 0)
```

Returns a computed timeout value based on the round-trip time it takes for the peer at the other end of the session to
receive a message, process it and send it back. This is computed based on the session type, the type of transport,
sleepy characteristics of the target and a caller-provided value for the time it takes to process a message
at the upper layer on the target For group sessions.

This will result in a session being established if one wasn't already.

**Returns**:

- `int` - The computed timeout value in milliseconds, representing the round-trip time.

#### GetRemoteSessionParameters

```python
def GetRemoteSessionParameters(nodeid) -> typing.Optional[SessionParameters]
```

Returns the SessionParameters of reported by the remote node associated with `nodeid`.
If there is some error in getting SessionParameters None is returned.

This will result in a session being established if one wasn't already established.

**Returns**:

- `SessionParameters` - The session parameters.

#### TestOnlySendBatchCommands

```python
async def TestOnlySendBatchCommands(
        nodeid: int,
        commands: typing.List[ClusterCommand.InvokeRequestInfo],
        timedRequestTimeoutMs: typing.Optional[int] = None,
        interactionTimeoutMs: typing.Optional[int] = None,
        busyWaitMs: typing.Optional[int] = None,
        suppressResponse: typing.Optional[bool] = None,
        remoteMaxPathsPerInvoke: typing.Optional[int] = None,
        suppressTimedRequestMessage: bool = False,
        commandRefsOverride: typing.Optional[typing.List[int]] = None)
```

Please see SendBatchCommands for description.
TestOnly overridable arguments:
remoteMaxPathsPerInvoke: Overrides the number of batch commands we think can be sent to remote node.
suppressTimedRequestMessage: When set to true, we suppress sending Timed Request Message.
commandRefsOverride: List of commandRefs to use for each command with the same index in `commands`.

**Returns**:

  TestOnlyBatchCommandResponse

#### TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke

```python
async def TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(
        nodeid: int,
        endpoint: int,
        payload: ClusterObjects.ClusterCommand,
        responseType=None)
```

Please see SendCommand for description.

**Returns**:

  Command response. The type of the response is defined by the command.
  

**Raises**:

  InteractionModelError on error

#### SendCommand

```python
async def SendCommand(
        nodeid: int,
        endpoint: int,
        payload: ClusterObjects.ClusterCommand,
        responseType=None,
        timedRequestTimeoutMs: typing.Optional[int] = None,
        interactionTimeoutMs: typing.Optional[int] = None,
        busyWaitMs: typing.Optional[int] = None,
        suppressResponse: typing.Optional[bool] = None,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Send a cluster-object encapsulated command to a node and get returned a future that can be awaited upon to receive
the response. If a valid responseType is passed in, that will be used to de-serialize the object. If not,
the type will be automatically deduced from the metadata received over the wire.

timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the
right timeout value based on transport characteristics as well as the responsiveness of the target.

**Returns**:

  command response. The type of the response is defined by the command.
  

**Raises**:

  InteractionModelError on error

#### SendBatchCommands

```python
async def SendBatchCommands(
        nodeid: int,
        commands: typing.List[ClusterCommand.InvokeRequestInfo],
        timedRequestTimeoutMs: typing.Optional[int] = None,
        interactionTimeoutMs: typing.Optional[int] = None,
        busyWaitMs: typing.Optional[int] = None,
        suppressResponse: typing.Optional[bool] = None,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Send a batch of cluster-object encapsulated commands to a node and get returned a future that can be awaited upon to receive
the responses. If a valid responseType is passed in, that will be used to de-serialize the object. If not,
the type will be automatically deduced from the metadata received over the wire.

nodeId: Target's Node ID
commands: A list of InvokeRequestInfo containing the commands to invoke.
timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None' to indicate a non-timed request.
interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the
right timeout value based on transport characteristics as well as the responsiveness of the target.
busyWaitMs: How long to wait in ms after sending command to device before performing any other operations.
suppressResponse: Do not send a response to this action

**Returns**:

  - List of command responses in the same order as what was given in `commands`. The type of the response is defined by the command.
  - A value of `None` indicates success.
  - If only a single command fails, for example with `UNSUPPORTED_COMMAND`, the corresponding index associated with the command will,
  contain `interaction_model.Status.UnsupportedCommand`.
  - If a command is not responded to by server, command will contain `interaction_model.Status.NoCommandResponse`

**Raises**:

  - InteractionModelError if error with sending of InvokeRequestMessage fails as a whole.

#### SendGroupCommand

```python
def SendGroupCommand(groupid: int,
                     payload: ClusterObjects.ClusterCommand,
                     busyWaitMs: typing.Optional[int] = None)
```

Send a group cluster-object encapsulated command to a group_id and get returned a future
that can be awaited upon to get confirmation command was sent.

**Returns**:

- `None` - responses are not sent to group commands.
  

**Raises**:

  InteractionModelError on error.

#### WriteAttribute

```python
async def WriteAttribute(
        nodeid: int,
        attributes: typing.List[typing.Tuple[
            int, ClusterObjects.ClusterAttributeDescriptor]],
        timedRequestTimeoutMs: typing.Optional[int] = None,
        interactionTimeoutMs: typing.Optional[int] = None,
        busyWaitMs: typing.Optional[int] = None,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Write a list of attributes on a target node.

nodeId: Target's Node ID
timedWriteTimeoutMs: Timeout for a timed write request. Omit or set to 'None' to indicate a non-timed request.
attributes: A list of tuples of type (endpoint, cluster-object):
interactionTimeoutMs: Overall timeout for the interaction. Omit or set to 'None' to have the SDK automatically compute the
right timeout value based on transport characteristics as well as the responsiveness of the target.

E.g
(1, Clusters.UnitTesting.Attributes.XYZAttribute('hello')) -- Write 'hello'
to the XYZ attribute on the test cluster to endpoint 1

**Returns**:

  [AttributeStatus] (list - one for each path).
  

**Raises**:

  InteractionModelError on error.

#### WriteGroupAttribute

```python
def WriteGroupAttribute(groupid: int,
                        attributes: typing.List[typing.Tuple[
                            ClusterObjects.ClusterAttributeDescriptor, int]],
                        busyWaitMs: typing.Optional[int] = None)
```

Write a list of attributes on a target group.

groupid: Group ID to send write attribute to.
attributes: A list of tuples of type (cluster-object, data-version). The data-version can be omitted.

E.g
(Clusters.UnitTesting.Attributes.XYZAttribute('hello'), 1) -- Group Write 'hello' with data version 1.

**Returns**:

  list = An empty list
  

**Raises**:

  InteractionModelError on error.

#### TestOnlyPrepareToReceiveBdxData

```python
def TestOnlyPrepareToReceiveBdxData() -> asyncio.Future
```

Sets up the system to expect a node to initiate a BDX transfer. The transfer will send data here.

**Returns**:

  a future that will yield a BdxTransfer with the init message from the transfer.
  

**Raises**:

  InteractionModelError on error.

#### TestOnlyPrepareToSendBdxData

```python
def TestOnlyPrepareToSendBdxData(data: bytes) -> asyncio.Future
```

Sets up the system to expect a node to initiate a BDX transfer. The transfer will send data to the node.

**Returns**:

  A future that will yield a BdxTransfer with the init message from the transfer.
  

**Raises**:

  InteractionModelError on error.

#### Read

```python
async def Read(
        nodeid: int,
        attributes: typing.
    Optional[typing.List[typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[int],  # Endpoint
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster]],
        # Wildcard endpoint, Cluster + Attribute present
        typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Wildcard attribute id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster]],
        # Concrete path
        typing.Tuple[int,
                     typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Directly specified attribute path
        ClusterAttribute.AttributePath]]] = None,
        dataVersionFilters: typing.Optional[typing.List[typing.Tuple[
            int, typing.Type[ClusterObjects.Cluster], int]]] = None,
        events: typing.Optional[typing.List[typing.Union[
            None,  # Empty tuple, all wildcard
            typing.Tuple[str, int],  # all wildcard with urgency set
            typing.Tuple[int, int],  # Endpoint,
            # Wildcard endpoint, Cluster id present
            typing.Tuple[typing.Type[ClusterObjects.Cluster], int],
            # Wildcard endpoint, Cluster + Event present
            typing.Tuple[typing.Type[ClusterObjects.ClusterEvent], int],
            # Wildcard event id
            typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int],
            # Concrete path
            typing.Tuple[int, typing.Type[ClusterObjects.ClusterEvent],
                         int]]]] = None,
        eventNumberFilter: typing.Optional[int] = None,
        returnClusterObject: bool = False,
        reportInterval: typing.Optional[typing.Tuple[int, int]] = None,
        fabricFiltered: bool = True,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Read a list of attributes and/or events from a target node

nodeId: Target's Node ID
attributes: A list of tuples of varying types depending on the type of read being requested:
(endpoint, Clusters.ClusterA.AttributeA):   Endpoint = specific,    Cluster = specific,   Attribute = specific
(endpoint, Clusters.ClusterA):              Endpoint = specific,    Cluster = specific,   Attribute = *
(Clusters.ClusterA.AttributeA):             Endpoint = *,           Cluster = specific,   Attribute = specific
endpoint:                                   Endpoint = specific,    Cluster = *,          Attribute = *
Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Attribute = *
'*' or ():                                  Endpoint = *,           Cluster = *,          Attribute = *

The cluster and attributes specified above are to be selected from the generated cluster objects.

e.g.
ReadAttribute(1, [ 1 ] ) -- case 4 above.
ReadAttribute(1, [ Clusters.BasicInformation ] ) -- case 5 above.
ReadAttribute(1, [ (1, Clusters.BasicInformation.Attributes.Location ] ) -- case 1 above.

An AttributePath can also be specified directly by [chip.cluster.Attribute.AttributePath(...)]

dataVersionFilters: A list of tuples of (endpoint, cluster, data version).

events: A list of tuples of varying types depending on the type of read being requested:
(endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,
Cluster = specific,   Event = specific, Urgent = True/False
(endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,
Cluster = specific,   Event = *, Urgent = True/False
(Clusters.ClusterA.EventA, urgent):                 Endpoint = *,
Cluster = specific,   Event = specific, Urgent = True/False
endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
Clusters.ClusterA:                          Endpoint = *,          Cluster = specific,    Event = *, Urgent = True/False
'*' or ():                                  Endpoint = *,          Cluster = *,          Event = *, Urgent = True/False

eventNumberFilter: Optional minimum event number filter.

returnClusterObject: This returns the data as consolidated cluster objects, with all attributes for a cluster inside
a single cluster-wide cluster object.

reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
When not provided, a read request will be sent.
fabricFiltered: If True (default), the read/subscribe is fabric-filtered and will only see things associated with the fabric
of the reader/subscriber. Relevant for attributes with fabric-scoped data.
keepSubscriptions: Keep existing subscriptions. If set to False, existing subscriptions with this node will get cancelled
and a new one gets setup.
autoResubscribe: Automatically resubscribe to the subscription if subscription is lost. The automatic re-subscription only
applies if the subscription establishes on first try. If the first subscription establishment attempt fails the function
returns right away.

**Returns**:

  - AsyncReadTransaction.ReadResponse. Please see ReadAttribute and ReadEvent for examples of how to access data.
  

**Raises**:

  - InteractionModelError (chip.interaction_model) on error

#### ReadAttribute

```python
async def ReadAttribute(
        nodeid: int,
        attributes: typing.
    Optional[typing.List[typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[int],  # Endpoint
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster]],
        # Wildcard endpoint, Cluster + Attribute present
        typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Wildcard attribute id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster]],
        # Concrete path
        typing.Tuple[int,
                     typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Directly specified attribute path
        ClusterAttribute.AttributePath]]],
        dataVersionFilters: typing.Optional[typing.List[typing.Tuple[
            int, typing.Type[ClusterObjects.Cluster], int]]] = None,
        returnClusterObject: bool = False,
        reportInterval: typing.Optional[typing.Tuple[int, int]] = None,
        fabricFiltered: bool = True,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Read a list of attributes from a target node, this is a wrapper of DeviceController.Read()

nodeId: Target's Node ID
attributes: A list of tuples of varying types depending on the type of read being requested:
(endpoint, Clusters.ClusterA.AttributeA):   Endpoint = specific,    Cluster = specific,   Attribute = specific
(endpoint, Clusters.ClusterA):              Endpoint = specific,    Cluster = specific,   Attribute = *
(Clusters.ClusterA.AttributeA):             Endpoint = *,           Cluster = specific,   Attribute = specific
endpoint:                                   Endpoint = specific,    Cluster = *,          Attribute = *
Clusters.ClusterA:                          Endpoint = *,           Cluster = specific,   Attribute = *
'*' or ():                                  Endpoint = *,           Cluster = *,          Attribute = *

The cluster and attributes specified above are to be selected from the generated cluster objects.

e.g.
ReadAttribute(1, [ 1 ] ) -- case 4 above.
ReadAttribute(1, [ Clusters.BasicInformation ] ) -- case 5 above.
ReadAttribute(1, [ (1, Clusters.BasicInformation.Attributes.Location ] ) -- case 1 above.

An AttributePath can also be specified directly by [chip.cluster.Attribute.AttributePath(...)]

returnClusterObject: This returns the data as consolidated cluster objects, with all attributes for a cluster inside
a single cluster-wide cluster object.

reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
When not provided, a read request will be sent.
fabricFiltered: If True (default), the read/subscribe is fabric-filtered and will only see things associated with the fabric
of the reader/subscriber. Relevant for attributes with fabric-scoped data.
keepSubscriptions: Keep existing subscriptions. If set to False, existing subscriptions with this node will get cancelled
and a new one gets setup.
autoResubscribe: Automatically resubscribe to the subscription if subscription is lost. The automatic re-subscription only
applies if the subscription establishes on first try. If the first subscription establishment attempt fails the function
returns right away.

**Returns**:

  - subscription request: ClusterAttribute.SubscriptionTransaction
  To get notified on attribute change use SetAttributeUpdateCallback on the returned
  SubscriptionTransaction. This is used to set a callback function, which is a callable of
  type Callable[[TypedAttributePath, SubscriptionTransaction], None]
  Get the attribute value from the change path using GetAttribute on the SubscriptionTransaction
  You can await changes in the main loop using a trigger mechanism from the callback.
  ex. queue.SimpleQueue
  
  - read request: AsyncReadTransaction.ReadResponse.attributes.
  This is of type AttributeCache.attributeCache (Attribute.py),
  which is a dict mapping endpoints to a list of Cluster (ClusterObjects.py) classes
  (dict[int, List[Cluster]])
  Access as returned_object[endpoint_id][<Cluster class>][<Attribute class>]
  Ex. To access the OnTime attribute from the OnOff cluster on endpoint 1
  returned_object[1][Clusters.OnOff][Clusters.OnOff.Attributes.OnTime]
  

**Raises**:

  - InteractionModelError (chip.interaction_model) on error

#### ReadEvent

```python
async def ReadEvent(
        nodeid: int,
        events: typing.List[typing.Union[
            None,  # Empty tuple, all wildcard
            typing.Tuple[str, int],  # all wildcard with urgency set
            typing.Tuple[int, int],  # Endpoint,
            # Wildcard endpoint, Cluster id present
            typing.Tuple[typing.Type[ClusterObjects.Cluster], int],
            # Wildcard endpoint, Cluster + Event present
            typing.Tuple[typing.Type[ClusterObjects.ClusterEvent], int],
            # Wildcard event id
            typing.Tuple[int, typing.Type[ClusterObjects.Cluster], int],
            # Concrete path
            typing.Tuple[int, typing.Type[ClusterObjects.ClusterEvent], int]]],
        eventNumberFilter: typing.Optional[int] = None,
        fabricFiltered: bool = True,
        reportInterval: typing.Optional[typing.Tuple[int, int]] = None,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True,
        payloadCapability: int = TransportPayloadCapability.MRP_PAYLOAD)
```

Read a list of events from a target node, this is a wrapper of DeviceController.Read()

nodeId: Target's Node ID
events: A list of tuples of varying types depending on the type of read being requested:
(endpoint, Clusters.ClusterA.EventA, urgent):       Endpoint = specific,
Cluster = specific,   Event = specific, Urgent = True/False
(endpoint, Clusters.ClusterA, urgent):              Endpoint = specific,
Cluster = specific,   Event = *, Urgent = True/False
(Clusters.ClusterA.EventA, urgent):                 Endpoint = *,
Cluster = specific,   Event = specific, Urgent = True/False
endpoint:                                   Endpoint = specific,    Cluster = *,          Event = *, Urgent = True/False
Clusters.ClusterA:                          Endpoint = *,          Cluster = specific,    Event = *, Urgent = True/False
'*' or ():                                  Endpoint = *,          Cluster = *,          Event = *, Urgent = True/False

The cluster and events specified above are to be selected from the generated cluster objects.

e.g.
ReadEvent(1, [ 1 ] ) -- case 4 above.
ReadEvent(1, [ Clusters.BasicInformation ] ) -- case 5 above.
ReadEvent(1, [ (1, Clusters.BasicInformation.Events.Location ] ) -- case 1 above.

eventNumberFilter: Optional minimum event number filter.
reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing subscriptions.
When not provided, a read request will be sent.
keepSubscriptions: Keep existing subscriptions. If set to False, existing subscriptions with this node will get cancelled
and a new one gets setup.
autoResubscribe: Automatically resubscribe to the subscription if subscription is lost. The automatic re-subscription only
applies if the subscription establishes on first try. If the first subscription establishment attempt fails the function
returns right away.

**Returns**:

  - subscription request: ClusterAttribute.SubscriptionTransaction
  To get notified on event subscriptions, use the SetEventUpdateCallback function on the
  returned  SubscriptionTransaction. This is a callable of type
  Callable[[EventReadResult, SubscriptionTransaction], None]
  You can await events using a trigger mechanism in the callback. ex. queue.SimpleQueue
  
  - read request: AsyncReadTransaction.ReadResponse.events.
  This is a List[ClusterEvent].
  

**Raises**:

  - InteractionModelError (chip.interaction_model) on error

#### SetIpk

```python
def SetIpk(ipk: bytes)
```

Sets the Identity Protection Key (IPK) for the device controller.

**Raises**:

- `ChipStackError` - On failure.

#### InitGroupTestingData

```python
def InitGroupTestingData()
```

Populates the Device Controller's GroupDataProvider with known test group info and keys.

**Raises**:

- `ChipStackError` - On failure.

#### CreateManualCode

```python
def CreateManualCode(discriminator: int, passcode: int) -> str
```

Creates a standard flow manual code from the given discriminator and passcode.

**Returns**:

- `str` - The decoded string from the buffer.

**Raises**:

- `MemoryError` - If the output size is invalid during manual code creation.

## ChipDeviceController

```python
class ChipDeviceController(ChipDeviceControllerBase)
```

The ChipDeviceCommissioner binding, named as ChipDeviceController

TODO: This class contains DEPRECATED functions, we should update the test scripts to avoid the usage of those functions.

#### Commission

```python
async def Commission(nodeid) -> int
```

Start the auto-commissioning process on a node after establishing a PASE connection.
This function is intended to be used in conjunction with `EstablishPASESessionBLE` or
`EstablishPASESessionIP`. It can be called either before or after the DevicePairingDelegate
receives the OnPairingComplete call. Commissioners that want to perform simple
auto-commissioning should use the supplied "CommissionWithCode" function, which will
establish the PASE connection and commission automatically.

**Raises**:

  - A ChipStackError on failure.
  

**Returns**:

  - Effective Node ID of the device (as defined by the assigned NOC).

#### CommissionThread

```python
async def CommissionThread(discriminator,
                           setupPinCode,
                           nodeId,
                           threadOperationalDataset: bytes,
                           isShortDiscriminator: bool = False) -> int
```

Commissions a Thread device over BLE.

**Returns**:

- `int` - Effective Node ID of the device (as defined by the assigned NOC).

#### CommissionWiFi

```python
async def CommissionWiFi(discriminator,
                         setupPinCode,
                         nodeId,
                         ssid: str,
                         credentials: str,
                         isShortDiscriminator: bool = False) -> int
```

Commissions a Wi-Fi device over BLE.

**Returns**:

- `int` - Effective Node ID of the device (as defined by the assigned NOC).

#### SetWiFiCredentials

```python
def SetWiFiCredentials(ssid: str, credentials: str)
```

Set the Wi-Fi credentials to set during commissioning.

**Raises**:

- `ChipStackError` - On failure.

#### SetThreadOperationalDataset

```python
def SetThreadOperationalDataset(threadOperationalDataset)
```

Set the Thread operational dataset to set during commissioning.

**Raises**:

- `ChipStackError` - On failure.

#### ResetCommissioningParameters

```python
def ResetCommissioningParameters()
```

Sets the commissioning parameters back to the default values.

**Raises**:

- `ChipStackError` - On failure.

#### SetTimeZone

```python
def SetTimeZone(offset: int, validAt: int, name: str = "")
```

Set the time zone to set during commissioning. Currently only one time zone entry is supported.

**Raises**:

- `ChipStackError` - On failure.

#### SetDSTOffset

```python
def SetDSTOffset(offset: int, validStarting: int, validUntil: int)
```

Set the DST offset to set during commissioning. Currently only one DST entry is supported.

**Raises**:

- `ChipStackError` - On failure.

#### SetTCAcknowledgements

```python
def SetTCAcknowledgements(tcAcceptedVersion: int, tcUserResponse: int)
```

Set the TC acknowledgements to set during commissioning.

**Raises**:

- `ChipStackError` - On failure.

#### SetSkipCommissioningComplete

```python
def SetSkipCommissioningComplete(skipCommissioningComplete: bool)
```

Set whether to skip the commissioning complete callback.

**Raises**:

- `ChipStackError` - On failure.

#### SetDefaultNTP

```python
def SetDefaultNTP(defaultNTP: str)
```

Set the DefaultNTP to set during commissioning.

**Raises**:

- `ChipStackError` - On failure.

#### SetTrustedTimeSource

```python
def SetTrustedTimeSource(nodeId: int, endpoint: int)
```

Set the trusted time source nodeId to set during commissioning. This must be a node on the commissioner fabric.

**Raises**:

- `ChipStackError` - On failure.

#### SetCheckMatchingFabric

```python
def SetCheckMatchingFabric(check: bool)
```

Instructs the auto-commissioner to perform a matching fabric check before commissioning.

**Raises**:

- `ChipStackError` - On failure.

#### GenerateICDRegistrationParameters

```python
def GenerateICDRegistrationParameters()
```

Generates ICD registration parameters for this controller.

**Returns**:

- `ICDRegistrationParameters` - An object containing the generated parameters
  including symmetricKey, checkInNodeId, monitoredSubject, stayActiveMs,
  and clientType.

#### EnableICDRegistration

```python
def EnableICDRegistration(parameters: ICDRegistrationParameters)
```

Enables ICD registration for the following commissioning session.

**Arguments**:

- `parameters` - A ICDRegistrationParameters for the parameters used for ICD registration, or None for default arguments.
  

**Raises**:

- `ChipStackError` - On failure.

#### DisableICDRegistration

```python
def DisableICDRegistration()
```

Disables ICD registration.

**Raises**:

- `ChipStackError` - On failure.

#### GetFabricCheckResult

```python
def GetFabricCheckResult() -> int
```

Returns the fabric check result if SetCheckMatchingFabric was used.

**Returns**:

- `int` - The fabric check result, or `-1` if no check was performed.

#### CommissionOnNetwork

```python
async def CommissionOnNetwork(
        nodeId: int,
        setupPinCode: int,
        filterType: DiscoveryFilterType = DiscoveryFilterType.NONE,
        filter: typing.Any = None,
        discoveryTimeoutMsec: int = 30000) -> int
```

Does the routine for OnNetworkCommissioning, with a filter for mDNS discovery.
Supported filters are:

DiscoveryFilterType.NONE
DiscoveryFilterType.SHORT_DISCRIMINATOR
DiscoveryFilterType.LONG_DISCRIMINATOR
DiscoveryFilterType.VENDOR_ID
DiscoveryFilterType.DEVICE_TYPE
DiscoveryFilterType.COMMISSIONING_MODE
DiscoveryFilterType.INSTANCE_NAME
DiscoveryFilterType.COMMISSIONER
DiscoveryFilterType.COMPRESSED_FABRIC_ID

The filter can be an integer, a string or None depending on the actual type of selected filter.

**Raises**:

- `ChipStackError` - On failure.
  

**Returns**:

  - Effective Node ID of the device (as defined by the assigned NOC)

#### get\_rcac

```python
def get_rcac()
```

Passes captured RCAC data back to Python test modules for validation
- Setting buffer size to max size mentioned in spec:
- Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/06c4d55962954546ecf093c221fe1dab57645028/policies/matter_certificate_policy.adoc#615-key-sizes

**Returns**:

- `bytes` - A bytes sentence representing the RCAC, or None if no data.

#### CommissionWithCode

```python
async def CommissionWithCode(
        setupPayload: str,
        nodeid: int,
        discoveryType: DiscoveryType = DiscoveryType.DISCOVERY_ALL) -> int
```

Commission with the given nodeid from the setupPayload.
setupPayload may be a QR or manual code.

**Raises**:

- `ChipStackError` - On failure.
  

**Returns**:

  Effective Node ID of the device (as defined by the assigned NOC)

#### CommissionIP

```python
async def CommissionIP(ipaddr: str, setupPinCode: int, nodeid: int) -> int
```

DEPRECATED, DO NOT USE! Use `CommissionOnNetwork` or `CommissionWithCode`

**Raises**:

- `ChipStackError` - On failure.
  

**Returns**:

  Effective Node ID of the device (as defined by the assigned NOC)

#### NOCChainCallback

```python
def NOCChainCallback(nocChain)
```

Callback function for handling the NOC chain result.

**Returns**:

  None

#### IssueNOCChain

```python
async def IssueNOCChain(
        csr: Clusters.OperationalCredentials.Commands.CSRResponse,
        nodeId: int)
```

Issue an NOC chain using the associated OperationalCredentialsDelegate.
The NOC chain will be provided in TLV cert format.

**Returns**:

- `asyncio.Future` - A future object that is the result of the NOC Chain operation.

#### SetDACRevocationSetPath

```python
def SetDACRevocationSetPath(dacRevocationSetPath: typing.Optional[str])
```

Set the path to the device attestation revocation set JSON file.

**Arguments**:

- `dacRevocationSetPath` - Path to the JSON file containing the device attestation revocation set.
  

**Raises**:

- `ChipStackError` - On failure.

## BareChipDeviceController

```python
class BareChipDeviceController(ChipDeviceControllerBase)
```

A bare device controller without AutoCommissioner support.

#### \_\_init\_\_

```python
def __init__(operationalKey: p256keypair.P256Keypair,
             noc: bytes,
             icac: typing.Union[bytes, None],
             rcac: bytes,
             ipk: typing.Union[bytes, None],
             adminVendorId: int,
             name: typing.Optional[str] = None)
```

Creates a controller without AutoCommissioner.

The allocated controller uses the noc, icac, rcac and ipk instead of the default,
random generated certificates / keys. Which is suitable for creating a controller
for manually signing certificates for testing.

**Arguments**:

- `operationalKey` - A P256Keypair object for the operational key of the controller.
- `noc` - The NOC for the controller, in bytes.
- `icac` - The optional ICAC for the controller.
- `rcac` - The RCAC for the controller.
- `ipk` - The optional IPK for the controller, when None is provided, the defaultIpk
  will be used.
- `adminVendorId` - The adminVendorId of the controller.
- `name` - The name of the controller, for debugging use only.
  

**Raises**:

- `ChipStackError` - On failure

