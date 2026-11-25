# On/Off Sensor

`--device on-off-sensor:1` exposes the mandatory Identify and Binding servers,
plus Identify and On/Off clients. Descriptor is provided by the endpoint base.
The sensor sends commands through bindings using its C++ `SendOn`, `SendOff`,
`SendToggle`, and `SendIdentify` methods. Optional clusters are not enabled. The
Identify server uses the platform delegate, including its TriggerEffect
capability.

## C++ Command Completion

`SendOn`, `SendOff`, `SendToggle`, and `SendIdentify` accept optional
`BindingCommandOptions`. Existing calls without options remain valid. The return
value reports admission: an error means no target was sent or queued and no
completion callback will run. `CHIP_ERROR_BUSY` rejects a submission when any
matching unicast binding already has pending work. The first command is kept.

```cpp
BindingCommandOptions options;
options.onComplete = {
    [](void * context, const Clusters::Binding::TableEntry & target, CHIP_ERROR error) {
        static_cast<MyApplication *>(context)->OnBindingCommandComplete(target, error);
    },
    &application,
};
options.caseTimeout = System::Clock::Seconds16(10);
options.responseTimeout = System::Clock::Seconds16(5);
CHIP_ERROR admission = sensor.SendToggle(options);
```

An accepted command reports one completion per matching target. No matching
bindings means success with no callbacks. Callbacks can run before `Send*`
returns. The callback context must remain valid until every target completes;
copy the target if retaining it after the callback. Callbacks may submit commands,
but must not modify the binding table during dispatch.

Unicast success means a successful Invoke response. Group success means the
local group send succeeded; receivers do not acknowledge group commands.
Pending CASE work expires after 30 seconds by default and cannot be sent after
expiry. Removing a pending binding or its fabric reports `CHIP_ERROR_CANCELLED`.
The separate response timeout also defaults to 30 seconds. A response timeout
can occur after a peer executed the command, so callers must not automatically
retry Toggle. No cancellation handle or FIFO command queue is provided.

## Running

Run commands from the SDK root after building all-devices-app:

```bash
./out/linux-x64-all-devices-clang/all-devices-app \
    --device on-off-sensor:1 --app-pipe /tmp/on-off-sensor-pipe
```

## Named-Pipe Commands

On POSIX, write one JSON object per message to the `--app-pipe` named pipe. Each
action targets the sensor endpoint specified by `EndpointId`:

```text
{"Name": "SensorOn", "EndpointId": 1}
{"Name": "SensorOff", "EndpointId": 1}
{"Name": "SensorToggle", "EndpointId": 1}
{"Name": "SensorIdentify", "EndpointId": 1, "IdentifyTime": 60}
```

Configure the sensor's Binding list and the peer's ACL before sending commands.
`SensorIdentify` takes an unsigned 16-bit duration in seconds; 0 stops
identification. The actions send commands to bound peers and do not change local
attributes.

For example, after configuring bindings and the peer ACL:

```bash
echo '{"Name": "SensorOn", "EndpointId": 1}' > /tmp/on-off-sensor-pipe
```

## Integration Tests

Identify attribute and timer tests (`Test_TC_I_2_1`, `Test_TC_I_2_2`) retain
all-clusters runs and also run against `--device on-off-sensor:1`. `TC_I_2_4.py`
adds a sensor run for Identify reporting. `TC_DeviceBasicComposition.py` covers
the sensor through its wildcard and bridged wildcard runs (`run18` and `run19`).

`TestOnOffSensor.py` starts a separate all-devices-app light, commissions the
light onto the sensor's fabric, configures its ACL and the sensor's bindings,
and verifies On/Off and Identify commands through peer attribute reports. The
test uses two sensor endpoints to check that Binding lists remain separate.

```bash
./scripts/tests/local.py python-tests \
    --test-filter TestOnOffSensor \
    --override-binary-path ALL_DEVICES_APP ./out/linux-x64-all-devices-clang/all-devices-app
```

The binding test requires POSIX named pipes and two available Matter ports (5540
for the sensor and 5543 for the peer). The test exercises mandatory clients
without enabling optional sensor clusters. Identify server TriggerEffect support
follows the platform Identify delegate.
