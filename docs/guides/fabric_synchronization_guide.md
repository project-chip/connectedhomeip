# Fabric Synchronization Guide

-   [Fabric Synchronization Guide](#fabric-synchronization-guide)
    -   [Fabric Sync Example Applications](#fabric-sync-example-applications)
    -   [Run Fabric Sync Demo on RP4](#run-fabric-sync-demo-on-rp4)

## Fabric Sync Example Applications

Fabric-Admin and Fabric-Bridge applications are provided to demonstrate Fabric
Synchronization feature. You can find them in the examples.

### Building the Example Application

-   Building the Fabric-Admin Application

    [Fabric-Admin](https://github.com/project-chip/connectedhomeip/tree/master/examples/fabric-admin/README.md)

*   Building the Fabric-Bridge Application

    [Fabric-Bridge](https://github.com/project-chip/connectedhomeip/tree/master/examples/fabric-bridge-app/linux/README.md)

## Run Fabric Sync Demo on RP4

### Setup Fabric Source

Connect to the Fabric Source server:

```
ssh ubuntu@xxx.xxx.xxx.xxx
```

Password: <password>

Run the Fabric Source script:

```
./run_fabric_source.sh
```

### Setup Fabric Sink

Connect to the Fabric Sink server:

```
ssh ubuntu@xxx.xxx.xxx.xxx
```

Password: <password>

Run the Fabric Sink script:

```
./run_fabric_sink.sh
```

### Fabric Sync Setup

Enable Fabric Auto Sync:

In Fabric-Sync console:

```
fabricsync enable-auto-sync 1
```

Pair the Fabric-Source bridge to Fabric-Sync with node ID 1:

```
fabricsync add-bridge 1 <fabric-sink-ip>
```

### Pair Light Example to Fabric-Source

Pair the Light Example with node ID 3 using its payload number:

```
pairing already-discovered 3 20202021 <ip> 5540
```

After the Light Example is successfully paired in Fabric-Source, it will be
synced to Fabric-Sink with a new assigned node ID.

Toggle the Light Example:

From Fabric-Source:

```
onoff on <node-id> 1
onoff off <node-id> 1
```

From Fabric-Sink: (Use the node ID assigned)

```
onoff on x 1
onoff off x 1
```

### Remove Light Example from Fabric-Source

Unpair the Light Example:

```
pairing unpair <node-id>
```

After the Light Example is successfully unpaired from Fabric-Source, it will
also be removed from the Fabric-Sink.

### Pair Commercial Switch to Fabric-Source

Pair the switch using its payload number:

In Fabric-Source console:

```
pairing code-wifi <node-id> <ssid> <passwd> <payload>
```

After the switch is successfully paired in Fabric-Source, it will be synced to
Fabric-Sink with a new assigned node ID.

Toggle the switch:

From Fabric-Source:

```
onoff on <node-id> 1
onoff off <node-id> 1
```

From Fabric-Sink: (Use the node ID assigned)

```
onoff on <node-id> 1
onoff off <node-id> 1
```

### Remove Switch from Fabric-Source

Unpair the switch:

```
pairing unpair <node-id>
```

After the switch is successfully unpaired from Fabric-Source, it will also be
removed from the Fabric-Sink.
