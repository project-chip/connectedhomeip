# Table of Contents

-   [Table of Contents](#table-of-contents)
-   [chip.ChipDeviceCtrl](#chipchipdevicectrl)
    -   [CommissionableNode](#commissionablenode)
        -   [Commission](#commission)
    -   [DeviceProxyWrapper](#deviceproxywrapper)
    -   [ChipDeviceControllerBase](#chipdevicecontrollerbase)
        -   [Shutdown](#shutdown)
        -   [ShutdownAll](#shutdownall)
        -   [ExpireSessions](#expiresessions)
        -   [DiscoverCommissionableNodes](#discovercommissionablenodes)
        -   [OpenCommissioningWindow](#opencommissioningwindow)
        -   [GetFabricIdInternal](#getfabricidinternal)
        -   [GetNodeIdInternal](#getnodeidinternal)
        -   [GetConnectedDeviceSync](#getconnecteddevicesync)
        -   [ComputeRoundTripTimeout](#computeroundtriptimeout)
        -   [GetRemoteSessionParameters](#getremotesessionparameters)
        -   [TestOnlySendBatchCommands](#testonlysendbatchcommands)
        -   [TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke](#testonlysendcommandtimedrequestflagwithnotimedinvoke)
        -   [SendCommand](#sendcommand)
        -   [SendBatchCommands](#sendbatchcommands)
        -   [SendGroupCommand](#sendgroupcommand)
        -   [WriteAttribute](#writeattribute)
        -   [WriteGroupAttribute](#writegroupattribute)
        -   [Read](#read)
        -   [ReadAttribute](#readattribute)
        -   [ReadEvent](#readevent)
        -   [ZCLSend](#zclsend)
        -   [ZCLReadAttribute](#zclreadattribute)
        -   [ZCLWriteAttribute](#zclwriteattribute)
        -   [ZCLSubscribeAttribute](#zclsubscribeattribute)
        -   [InitGroupTestingData](#initgrouptestingdata)
    -   [ChipDeviceController](#chipdevicecontroller)
        -   [Commission](#commission-1)
        -   [CommissionThread](#commissionthread)
        -   [CommissionWiFi](#commissionwifi)
        -   [SetWiFiCredentials](#setwificredentials)
        -   [SetThreadOperationalDataset](#setthreadoperationaldataset)
        -   [ResetCommissioningParameters](#resetcommissioningparameters)
        -   [SetTimeZone](#settimezone)
        -   [SetDSTOffset](#setdstoffset)
        -   [SetDefaultNTP](#setdefaultntp)
        -   [SetTrustedTimeSource](#settrustedtimesource)
        -   [SetCheckMatchingFabric](#setcheckmatchingfabric)
        -   [GetFabricCheckResult](#getfabriccheckresult)
        -   [CommissionOnNetwork](#commissiononnetwork)
        -   [CommissionWithCode](#commissionwithcode)
        -   [CommissionIP](#commissionip)
        -   [IssueNOCChain](#issuenocchain)
    -   [BareChipDeviceController](#barechipdevicecontroller)
        -   [\_\_init\_\_](#__init__)

<a id="chip.ChipDeviceCtrl"></a>

# chip.ChipDeviceCtrl

Chip Device Controller interface

<a id="chip.ChipDeviceCtrl.CommissionableNode"></a>

## CommissionableNode

```python
class CommissionableNode(discovery.CommissionableNode)
```

<a id="chip.ChipDeviceCtrl.CommissionableNode.Commission"></a>

#### Commission

```python
def Commission(nodeId: int, setupPinCode: int) -> PyChipError
```

Commission the device using the device controller discovered this device.

nodeId: The nodeId commissioned to the device setupPinCode: The setup pin code
of the device

<a id="chip.ChipDeviceCtrl.DeviceProxyWrapper"></a>

## DeviceProxyWrapper

```python
class DeviceProxyWrapper()
```

Encapsulates a pointer to OperationalDeviceProxy on the c++ side that needs to
be freed when DeviceProxyWrapper goes out of scope. There is a potential issue
where if this is copied around that a double free will occur, but how this is
used today that is not an issue that needs to be accounted for and it will
become very apparent if that happens.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase"></a>

## ChipDeviceControllerBase

```python
class ChipDeviceControllerBase()
```

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.Shutdown"></a>

#### Shutdown

```python
def Shutdown()
```

Shuts down this controller and reclaims any used resources, including the bound
C++ constructor instance in the SDK.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ShutdownAll"></a>

#### ShutdownAll

```python
def ShutdownAll()
```

Shut down all active controllers and reclaim any used resources.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ExpireSessions"></a>

#### ExpireSessions

```python
def ExpireSessions(nodeid)
```

Close all sessions with `nodeid` (if any existed) so that sessions get
re-established.

This is needed to properly handle operations that invalidate a node's state,
such as UpdateNOC.

WARNING: ONLY CALL THIS IF YOU UNDERSTAND THE SIDE-EFFECTS

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.DiscoverCommissionableNodes"></a>

#### DiscoverCommissionableNodes

```python
def DiscoverCommissionableNodes(
    filterType: discovery.FilterType = discovery.FilterType.NONE,
    filter: typing.Any = None,
    stopOnFirst: bool = False,
    timeoutSecond: int = 5
) -> typing.Union[None, CommissionableNode, typing.List[CommissionableNode]]
```

Discover commissionable nodes via DNS-SD with specified filters. Supported
filters are:

    discovery.FilterType.NONE
    discovery.FilterType.SHORT_DISCRIMINATOR
    discovery.FilterType.LONG_DISCRIMINATOR
    discovery.FilterType.VENDOR_ID
    discovery.FilterType.DEVICE_TYPE
    discovery.FilterType.COMMISSIONING_MODE
    discovery.FilterType.INSTANCE_NAME
    discovery.FilterType.COMMISSIONER
    discovery.FilterType.COMPRESSED_FABRIC_ID

This function will always return a list of CommissionableDevice. When
stopOnFirst is set, this function will return when at least one device is
discovered or on timeout.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.OpenCommissioningWindow"></a>

#### OpenCommissioningWindow

```python
def OpenCommissioningWindow(
        nodeid: int, timeout: int, iteration: int, discriminator: int,
        option: CommissioningWindowPasscode) -> CommissioningParameters
```

Opens a commissioning window on the device with the given nodeid. nodeid: Node
id of the device timeout: Command timeout iteration: The PAKE iteration count
associated with the PAKE Passcode ID and ephemeral PAKE passcode verifier to be
used for this commissioning. Valid range: 1000 - 100000 Ignored if option == 0
discriminator: The long discriminator for the DNS-SD advertisement. Valid range:
0-4095 Ignored if option == 0 option: 0 = kOriginalSetupCode 1 =
kTokenWithRandomPIN

Returns CommissioningParameters

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.GetFabricIdInternal"></a>

#### GetFabricIdInternal

```python
def GetFabricIdInternal()
```

Get the fabric ID from the object. Only used to validate cached value from
property.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.GetNodeIdInternal"></a>

#### GetNodeIdInternal

```python
def GetNodeIdInternal() -> int
```

Get the node ID from the object. Only used to validate cached value from
property.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.GetConnectedDeviceSync"></a>

#### GetConnectedDeviceSync

```python
def GetConnectedDeviceSync(nodeid, allowPASE=True, timeoutMs: int = None)
```

Returns DeviceProxyWrapper upon success.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ComputeRoundTripTimeout"></a>

#### ComputeRoundTripTimeout

```python
def ComputeRoundTripTimeout(nodeid, upperLayerProcessingTimeoutMs: int = 0)
```

Returns a computed timeout value based on the round-trip time it takes for the
peer at the other end of the session to receive a message, process it and send
it back. This is computed based on the session type, the type of transport,
sleepy characteristics of the target and a caller-provided value for the time it
takes to process a message at the upper layer on the target For group sessions.

This will result in a session being established if one wasn't already.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.GetRemoteSessionParameters"></a>

#### GetRemoteSessionParameters

```python
def GetRemoteSessionParameters(nodeid) -> typing.Optional[SessionParameters]
```

Returns the SessionParameters of reported by the remote node associated with
`nodeid`. If there is some error in getting SessionParameters None is returned.

This will result in a session being established if one wasn't already
established.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.TestOnlySendBatchCommands"></a>

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

Please see SendBatchCommands for description. TestOnly overridable arguments:
remoteMaxPathsPerInvoke: Overrides the number of batch commands we think can be
sent to remote node. suppressTimedRequestMessage: When set to true, we suppress
sending Timed Request Message. commandRefsOverride: List of commandRefs to use
for each command with the same index in `commands`.

**Returns**:

-   TestOnlyBatchCommandResponse

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke"></a>

#### TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke

```python
async def TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(
        nodeid: int,
        endpoint: int,
        payload: ClusterObjects.ClusterCommand,
        responseType=None)
```

Please see SendCommand for description.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.SendCommand"></a>

#### SendCommand

```python
async def SendCommand(nodeid: int,
                      endpoint: int,
                      payload: ClusterObjects.ClusterCommand,
                      responseType=None,
                      timedRequestTimeoutMs: typing.Union[None, int] = None,
                      interactionTimeoutMs: typing.Union[None, int] = None,
                      busyWaitMs: typing.Union[None, int] = None,
                      suppressResponse: typing.Union[None, bool] = None)
```

Send a cluster-object encapsulated command to a node and get returned a future
that can be awaited upon to receive the response. If a valid responseType is
passed in, that will be used to de-serialize the object. If not, the type will
be automatically deduced from the metadata received over the wire.

timedWriteTimeoutMs: Timeout for a timed invoke request. Omit or set to 'None'
to indicate a non-timed request. interactionTimeoutMs: Overall timeout for the
interaction. Omit or set to 'None' to have the SDK automatically compute the
right timeout value based on transport characteristics as well as the
responsiveness of the target.

**Returns**:

-   command response. The type of the response is defined by the command.

**Raises**:

-   InteractionModelError on error

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.SendBatchCommands"></a>

#### SendBatchCommands

```python
async def SendBatchCommands(
        nodeid: int,
        commands: typing.List[ClusterCommand.InvokeRequestInfo],
        timedRequestTimeoutMs: typing.Optional[int] = None,
        interactionTimeoutMs: typing.Optional[int] = None,
        busyWaitMs: typing.Optional[int] = None,
        suppressResponse: typing.Optional[bool] = None)
```

Send a batch of cluster-object encapsulated commands to a node and get returned
a future that can be awaited upon to receive the responses. If a valid
responseType is passed in, that will be used to de-serialize the object. If not,
the type will be automatically deduced from the metadata received over the wire.

nodeId: Target's Node ID commands: A list of InvokeRequestInfo containing the
commands to invoke. timedWriteTimeoutMs: Timeout for a timed invoke request.
Omit or set to 'None' to indicate a non-timed request. interactionTimeoutMs:
Overall timeout for the interaction. Omit or set to 'None' to have the SDK
automatically compute the right timeout value based on transport characteristics
as well as the responsiveness of the target. busyWaitMs: How long to wait in ms
after sending command to device before performing any other operations.
suppressResponse: Do not send a response to this action

**Returns**:

-   List of command responses in the same order as what was given in `commands`.
    The type of the response is defined by the command.
-   A value of `None` indicates success.
-   If only a single command fails, for example with `UNSUPPORTED_COMMAND`, the
    corresponding index associated with the command will, contain
    `interaction_model.Status.UnsupportedCommand`.
-   If a command is not responded to by server, command will contain
    `interaction_model.Status.NoCommandResponse`

**Raises**:

-   InteractionModelError if error with sending of InvokeRequestMessage fails as
    a whole.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.SendGroupCommand"></a>

#### SendGroupCommand

```python
def SendGroupCommand(groupid: int,
                     payload: ClusterObjects.ClusterCommand,
                     busyWaitMs: typing.Union[None, int] = None)
```

Send a group cluster-object encapsulated command to a group_id and get returned
a future that can be awaited upon to get confirmation command was sent.

**Returns**:

-   None: responses are not sent to group commands

**Raises**:

-   InteractionModelError on error

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.WriteAttribute"></a>

#### WriteAttribute

```python
async def WriteAttribute(nodeid: int,
                         attributes: typing.List[typing.Tuple[
                             int, ClusterObjects.ClusterAttributeDescriptor]],
                         timedRequestTimeoutMs: typing.Union[None, int] = None,
                         interactionTimeoutMs: typing.Union[None, int] = None,
                         busyWaitMs: typing.Union[None, int] = None)
```

Write a list of attributes on a target node.

nodeId: Target's Node ID timedWriteTimeoutMs: Timeout for a timed write request.
Omit or set to 'None' to indicate a non-timed request. attributes: A list of
tuples of type (endpoint, cluster-object): interactionTimeoutMs: Overall timeout
for the interaction. Omit or set to 'None' to have the SDK automatically compute
the right timeout value based on transport characteristics as well as the
responsiveness of the target.

E.g (1, Clusters.UnitTesting.Attributes.XYZAttribute('hello')) -- Write 'hello'
to the XYZ attribute on the test cluster to endpoint 1

**Returns**:

-   [AttributeStatus] (list - one for each path)

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.WriteGroupAttribute"></a>

#### WriteGroupAttribute

```python
def WriteGroupAttribute(groupid: int,
                        attributes: typing.List[typing.Tuple[
                            ClusterObjects.ClusterAttributeDescriptor, int]],
                        busyWaitMs: typing.Union[None, int] = None)
```

Write a list of attributes on a target group.

groupid: Group ID to send write attribute to. attributes: A list of tuples of
type (cluster-object, data-version). The data-version can be omitted.

E.g (Clusters.UnitTesting.Attributes.XYZAttribute('hello'), 1) -- Group Write
'hello' with data version 1

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.Read"></a>

#### Read

```python
async def Read(
        nodeid: int,
        attributes: typing.
    List[typing.Union[
        None,  # Empty tuple, all wildcard
        typing.Tuple[int],  # Endpoint
        # Wildcard endpoint, Cluster id present
        typing.Tuple[typing.Type[ClusterObjects.Cluster]],
        # Wildcard endpoint, Cluster + Attribute present
        typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]],
        # Wildcard attribute id
        typing.Tuple[int, typing.Type[ClusterObjects.Cluster]],
        # Concrete path
        typing.Tuple[
            int,
            typing.Type[ClusterObjects.ClusterAttributeDescriptor]]]] = None,
        dataVersionFilters: typing.List[typing.Tuple[
            int, typing.Type[ClusterObjects.Cluster], int]] = None,
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
            typing.Tuple[int, typing.Type[ClusterObjects.ClusterEvent],
                         int]]] = None,
        eventNumberFilter: typing.Optional[int] = None,
        returnClusterObject: bool = False,
        reportInterval: typing.Tuple[int, int] = None,
        fabricFiltered: bool = True,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True)
```

Read a list of attributes and/or events from a target node

nodeId: Target's Node ID attributes: A list of tuples of varying types depending
on the type of read being requested: (endpoint, Clusters.ClusterA.AttributeA):
Endpoint = specific, Cluster = specific, Attribute = specific (endpoint,
Clusters.ClusterA): Endpoint = specific, Cluster = specific, Attribute = _
(Clusters.ClusterA.AttributeA): Endpoint = _, Cluster = specific, Attribute =
specific endpoint: Endpoint = specific, Cluster = _, Attribute = _
Clusters.ClusterA: Endpoint = _, Cluster = specific, Attribute = _ '_' or ():
Endpoint = _, Cluster = _, Attribute = _

The cluster and attributes specified above are to be selected from the generated
cluster objects.

e.g. ReadAttribute(1, [ 1 ] ) -- case 4 above. ReadAttribute(1, [
Clusters.BasicInformation ] ) -- case 5 above. ReadAttribute(1, [ (1,
Clusters.BasicInformation.Attributes.Location ] ) -- case 1 above.

dataVersionFilters: A list of tuples of (endpoint, cluster, data version).

events: A list of tuples of varying types depending on the type of read being
requested: (endpoint, Clusters.ClusterA.EventA, urgent): Endpoint = specific,
Cluster = specific, Event = specific, Urgent = True/False (endpoint,
Clusters.ClusterA, urgent): Endpoint = specific, Cluster = specific, Event = _,
Urgent = True/False (Clusters.ClusterA.EventA, urgent): Endpoint = _, Cluster =
specific, Event = specific, Urgent = True/False endpoint: Endpoint = specific,
Cluster = _, Event = _, Urgent = True/False Clusters.ClusterA: Endpoint = _,
Cluster = specific, Event = _, Urgent = True/False '_' or (): Endpoint = _,
Cluster = _, Event = _, Urgent = True/False

eventNumberFilter: Optional minimum event number filter.

returnClusterObject: This returns the data as consolidated cluster objects, with
all attributes for a cluster inside a single cluster-wide cluster object.

reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling).
Used by establishing subscriptions. When not provided, a read request will be
sent.

**Returns**:

-   AsyncReadTransaction.ReadResponse. Please see ReadAttribute and ReadEvent
    for examples of how to access data.

**Raises**:

-   InteractionModelError (chip.interaction_model) on error

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ReadAttribute"></a>

#### ReadAttribute

```python
async def ReadAttribute(
        nodeid: int,
        attributes: typing.
    List[typing.Union[
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
                     typing.Type[ClusterObjects.ClusterAttributeDescriptor]]]],
        dataVersionFilters: typing.List[typing.Tuple[
            int, typing.Type[ClusterObjects.Cluster], int]] = None,
        returnClusterObject: bool = False,
        reportInterval: typing.Tuple[int, int] = None,
        fabricFiltered: bool = True,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True)
```

Read a list of attributes from a target node, this is a wrapper of
DeviceController.Read()

nodeId: Target's Node ID attributes: A list of tuples of varying types depending
on the type of read being requested: (endpoint, Clusters.ClusterA.AttributeA):
Endpoint = specific, Cluster = specific, Attribute = specific (endpoint,
Clusters.ClusterA): Endpoint = specific, Cluster = specific, Attribute = _
(Clusters.ClusterA.AttributeA): Endpoint = _, Cluster = specific, Attribute =
specific endpoint: Endpoint = specific, Cluster = _, Attribute = _
Clusters.ClusterA: Endpoint = _, Cluster = specific, Attribute = _ '_' or ():
Endpoint = _, Cluster = _, Attribute = _

The cluster and attributes specified above are to be selected from the generated
cluster objects.

e.g. ReadAttribute(1, [ 1 ] ) -- case 4 above. ReadAttribute(1, [
Clusters.BasicInformation ] ) -- case 5 above. ReadAttribute(1, [ (1,
Clusters.BasicInformation.Attributes.Location ] ) -- case 1 above.

returnClusterObject: This returns the data as consolidated cluster objects, with
all attributes for a cluster inside a single cluster-wide cluster object.

reportInterval: A tuple of two int-s for (MinIntervalFloor, MaxIntervalCeiling).
Used by establishing subscriptions. When not provided, a read request will be
sent.

**Returns**:

-   subscription request: ClusterAttribute.SubscriptionTransaction To get
    notified on attribute change use SetAttributeUpdateCallback on the returned
    SubscriptionTransaction. This is used to set a callback function, which is a
    callable of type Callable[[TypedAttributePath, SubscriptionTransaction],
    None] Get the attribute value from the change path using GetAttribute on the
    SubscriptionTransaction You can await changes in the main loop using a
    trigger mechanism from the callback. ex. queue.SimpleQueue

-   read request: AsyncReadTransaction.ReadResponse.attributes. This is of type
    AttributeCache.attributeCache (Attribute.py), which is a dict mapping
    endpoints to a list of Cluster (ClusterObjects.py) classes (dict[int,
    List[Cluster]]) Access as
    returned_object[endpoint_id][<cluster class>][<Attribute class>] Ex. To
    access the OnTime attribute from the OnOff cluster on endpoint 1
    returned_object[1][clusters.onoff][Clusters.OnOff.Attributes.OnTime]

**Raises**:

-   InteractionModelError (chip.interaction_model) on error

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ReadEvent"></a>

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
        reportInterval: typing.Tuple[int, int] = None,
        keepSubscriptions: bool = False,
        autoResubscribe: bool = True)
```

Read a list of events from a target node, this is a wrapper of
DeviceController.Read()

nodeId: Target's Node ID events: A list of tuples of varying types depending on
the type of read being requested: (endpoint, Clusters.ClusterA.EventA, urgent):
Endpoint = specific, Cluster = specific, Event = specific, Urgent = True/False
(endpoint, Clusters.ClusterA, urgent): Endpoint = specific, Cluster = specific,
Event = _, Urgent = True/False (Clusters.ClusterA.EventA, urgent): Endpoint = _,
Cluster = specific, Event = specific, Urgent = True/False endpoint: Endpoint =
specific, Cluster = _, Event = _, Urgent = True/False Clusters.ClusterA:
Endpoint = _, Cluster = specific, Event = _, Urgent = True/False '_' or ():
Endpoint = _, Cluster = _, Event = _, Urgent = True/False

The cluster and events specified above are to be selected from the generated
cluster objects.

e.g. ReadEvent(1, [ 1 ] ) -- case 4 above. ReadEvent(1, [
Clusters.BasicInformation ] ) -- case 5 above. ReadEvent(1, [ (1,
Clusters.BasicInformation.Events.Location ] ) -- case 1 above.

eventNumberFilter: Optional minimum event number filter. reportInterval: A tuple
of two int-s for (MinIntervalFloor, MaxIntervalCeiling). Used by establishing
subscriptions. When not provided, a read request will be sent.

**Returns**:

-   subscription request: ClusterAttribute.SubscriptionTransaction To get
    notified on event subscriptions, use the SetEventUpdateCallback function on
    the returned SubscriptionTransaction. This is a callable of type
    Callable[[EventReadResult, SubscriptionTransaction], None] You can await
    events using a trigger mechanism in the callback. ex. queue.SimpleQueue

-   read request: AsyncReadTransaction.ReadResponse.events. This is a
    List[ClusterEvent].

**Raises**:

-   InteractionModelError (chip.interaction_model) on error

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ZCLSend"></a>

#### ZCLSend

```python
def ZCLSend(cluster, command, nodeid, endpoint, groupid, args, blocking=False)
```

Wrapper over SendCommand that catches the exceptions Returns a tuple of
(errorCode, CommandResponse)

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ZCLReadAttribute"></a>

#### ZCLReadAttribute

```python
def ZCLReadAttribute(cluster,
                     attribute,
                     nodeid,
                     endpoint,
                     groupid,
                     blocking=True)
```

Wrapper over ReadAttribute for a single attribute Returns an AttributeReadResult

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ZCLWriteAttribute"></a>

#### ZCLWriteAttribute

```python
def ZCLWriteAttribute(cluster: str,
                      attribute: str,
                      nodeid,
                      endpoint,
                      groupid,
                      value,
                      dataVersion=0,
                      blocking=True)
```

Wrapper over WriteAttribute for a single attribute return PyChipError

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.ZCLSubscribeAttribute"></a>

#### ZCLSubscribeAttribute

```python
def ZCLSubscribeAttribute(cluster,
                          attribute,
                          nodeid,
                          endpoint,
                          minInterval,
                          maxInterval,
                          blocking=True,
                          keepSubscriptions=False,
                          autoResubscribe=True)
```

Wrapper over ReadAttribute for a single attribute Returns a
SubscriptionTransaction. See ReadAttribute for more information.

<a id="chip.ChipDeviceCtrl.ChipDeviceControllerBase.InitGroupTestingData"></a>

#### InitGroupTestingData

```python
def InitGroupTestingData()
```

Populates the Device Controller's GroupDataProvider with known test group info
and keys.

<a id="chip.ChipDeviceCtrl.ChipDeviceController"></a>

## ChipDeviceController

```python
class ChipDeviceController(ChipDeviceControllerBase)
```

The ChipDeviceCommissioner binding, named as ChipDeviceController

TODO: This class contains DEPRECATED functions, we should update the test
scripts to avoid the usage of those functions.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.Commission"></a>

#### Commission

```python
def Commission(nodeid) -> PyChipError
```

Start the auto-commissioning process on a node after establishing a PASE
connection. This function is intended to be used in conjunction with
`EstablishPASESessionBLE` or `EstablishPASESessionIP`. It can be called either
before or after the DevicePairingDelegate receives the OnPairingComplete call.
Commissioners that want to perform simple auto-commissioning should use the
supplied "PairDevice" functions above, which will establish the PASE connection
and commission automatically.

**Returns**:

-   `bool` - True if successful, False otherwise.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.CommissionThread"></a>

#### CommissionThread

```python
def CommissionThread(discriminator, setupPinCode, nodeId,
                     threadOperationalDataset: bytes) -> PyChipError
```

Commissions a Thread device over BLE

<a id="chip.ChipDeviceCtrl.ChipDeviceController.CommissionWiFi"></a>

#### CommissionWiFi

```python
def CommissionWiFi(discriminator, setupPinCode, nodeId, ssid: str,
                   credentials: str) -> PyChipError
```

Commissions a Wi-Fi device over BLE.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetWiFiCredentials"></a>

#### SetWiFiCredentials

```python
def SetWiFiCredentials(ssid: str, credentials: str)
```

Set the Wi-Fi credentials to set during commissioning.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetThreadOperationalDataset"></a>

#### SetThreadOperationalDataset

```python
def SetThreadOperationalDataset(threadOperationalDataset)
```

Set the Thread operational dataset to set during commissioning.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.ResetCommissioningParameters"></a>

#### ResetCommissioningParameters

```python
def ResetCommissioningParameters()
```

Sets the commissioning parameters back to the default values.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetTimeZone"></a>

#### SetTimeZone

```python
def SetTimeZone(offset: int, validAt: int, name: str = "")
```

Set the time zone to set during commissioning. Currently only one time zone
entry is supported

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetDSTOffset"></a>

#### SetDSTOffset

```python
def SetDSTOffset(offset: int, validStarting: int, validUntil: int)
```

Set the DST offset to set during commissioning. Currently only one DST entry is
supported

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetDefaultNTP"></a>

#### SetDefaultNTP

```python
def SetDefaultNTP(defaultNTP: str)
```

Set the DefaultNTP to set during commissioning

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetTrustedTimeSource"></a>

#### SetTrustedTimeSource

```python
def SetTrustedTimeSource(nodeId: int, endpoint: int)
```

Set the trusted time source nodeId to set during commissioning. This must be a
node on the commissioner fabric.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.SetCheckMatchingFabric"></a>

#### SetCheckMatchingFabric

```python
def SetCheckMatchingFabric(check: bool)
```

Instructs the auto-commissioner to perform a matching fabric check before
commissioning.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.GetFabricCheckResult"></a>

#### GetFabricCheckResult

```python
def GetFabricCheckResult() -> int
```

Returns the fabric check result if SetCheckMatchingFabric was used.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.CommissionOnNetwork"></a>

#### CommissionOnNetwork

```python
def CommissionOnNetwork(
        nodeId: int,
        setupPinCode: int,
        filterType: DiscoveryFilterType = DiscoveryFilterType.NONE,
        filter: typing.Any = None,
        discoveryTimeoutMsec: int = 30000) -> PyChipError
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

The filter can be an integer, a string or None depending on the actual type of
selected filter.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.CommissionWithCode"></a>

#### CommissionWithCode

```python
def CommissionWithCode(
        setupPayload: str,
        nodeid: int,
        discoveryType: DiscoveryType = DiscoveryType.DISCOVERY_ALL
) -> PyChipError
```

Commission with the given nodeid from the setupPayload. setupPayload may be a QR
or manual code.

<a id="chip.ChipDeviceCtrl.ChipDeviceController.CommissionIP"></a>

#### CommissionIP

```python
def CommissionIP(ipaddr: str, setupPinCode: int, nodeid: int) -> PyChipError
```

DEPRECATED, DO NOT USE! Use `CommissionOnNetwork` or `CommissionWithCode`

<a id="chip.ChipDeviceCtrl.ChipDeviceController.IssueNOCChain"></a>

#### IssueNOCChain

```python
def IssueNOCChain(csr: Clusters.OperationalCredentials.Commands.CSRResponse,
                  nodeId: int)
```

Issue an NOC chain using the associated OperationalCredentialsDelegate. The NOC
chain will be provided in TLV cert format.

<a id="chip.ChipDeviceCtrl.BareChipDeviceController"></a>

## BareChipDeviceController

```python
class BareChipDeviceController(ChipDeviceControllerBase)
```

A bare device controller without AutoCommissioner support.

<a id="chip.ChipDeviceCtrl.BareChipDeviceController.__init__"></a>

#### \_\_init\_\_

```python
def __init__(operationalKey: p256keypair.P256Keypair,
             noc: bytes,
             icac: typing.Union[bytes, None],
             rcac: bytes,
             ipk: typing.Union[bytes, None],
             adminVendorId: int,
             name: str = None)
```

Creates a controller without AutoCommissioner.

The allocated controller uses the noc, icac, rcac and ipk instead of the
default, random generated certificates / keys. Which is suitable for creating a
controller for manually signing certificates for testing.

**Arguments**:

-   `operationalKey` - A P256Keypair object for the operational key of the
    controller.
-   `noc` - The NOC for the controller, in bytes.
-   `icac` - The optional ICAC for the controller.
-   `rcac` - The RCAC for the controller.
-   `ipk` - The optional IPK for the controller, when None is provided, the
    defaultIpk will be used.
-   `adminVendorId` - The adminVendorId of the controller.
-   `name` - The name of the controller, for debugging use only.
