# Thread Border Router usage

This document describes the use of the Thread Border router and secondary
network interface for a Matter application

<hr>

-   [Thread Border Router usage](#thread-border-router-usage)
    -   [Thread Border Router overview](#thread-border-router-overview)
    -   [Using the Thread Border Router management cluster](#using-the-thread-border-router-management-cluster)
    -   [Using the Secondary Network commissioning interface](#using-the-secondary-network-commissioning-interface)
    -   [Using the Thread credential sharing mechanism](#using-the-thread-credential-sharing-mechanism)

<hr>

<a name="thread-border-router-overview"></a>

## Thread Border Router overview

This section contains an overview of the Border Router architecture and
describes the general use cases.

<a name="using-the-thread-border-router-management-cluster"></a>

## Using the Thread Border Router management cluster

The Thread Border Router management cluster allows provisioning of the Thread
interface using a Matter commissioner.

After the device has been provisioned over WIFI the set active dataset command
can be used to configure the Thread active dataset on the border router. Once
the dataset is set successfully the Thread network interface will be enabled and
the device will create a new PAN or join an existing one if already present.
Note that this command cannot be used on a device that already has an active
dataset configured. In this situation the set pending dataset command must be
used instead.

Before using the set active dataset command a fail-safe timer must be armed
(recommend using a timeout of 120 seconds):

```
ubuntu@ubuntu:~$ ./chip-tool generalcommissioning arm-fail-safe timeout-seconds 1 node-id 0
```

Then an active dataset in HEX TLV format (the same type used to provision a
Matter over Thread device using the `ble-thread` command) can be used to
provision the Border Router. What the active dataset should be is outside the
scope of this README but as an example one can be obtained from the OpenThread
cli on an already provisioned device using the `dataset active -x` command.

Note that the Thread Border Router management cluster has been set to endpoint 2
in the zap file.

```
ubuntu@ubuntu:~$ ./chip-tool threadborderroutermanagement set-active-dataset-request hex:<active-dataset> node id 2
```

If the active dataset command is successful, a commissioning complete command
must be send to disarm the fail-safe timer and commit the configuration to
non-volatile storage.

```
ubuntu@ubuntu:~$ ./chip-tool-19-jul generalcommissioning commissioning-complete node-id 0
```

Note that this command cannot be used on a device that already has an active
dataset configured. In this situation the set pending dataset command must be
used instead.

```
ubuntu@ubuntu:~$ ./chip-tool threadborderroutermanagement set-pending-dataset-request hex:<active-dataset> node id 2
```

To read the active dataset of an already provisioned device, for example to make
a joining Border Router use the same Thread network as an already configured
one, the get active dataset command can be used:

```
ubuntu@ubuntu:~$ ./chip-tool-19-jul threadborderroutermanagement get-active-dataset-request node-id 2
```

<a name="using-the-secondary-network-commissioning-interface"></a>

## Using the Secondary Network commissioning interface

To use the secondary network commissioning interface over Thread the device must
not be provisioned over WIFI. The regular `ble-thread` pairing is used as for
any other Matter over Thread device. The chip-tool will read all the endpoints
of the device and discover Thread network commissioning cluster on endpoint 3
and use that to provision the device. As for any other Matter over Thread device
a Thread Border Router is required in this case.

```
ubuntu@ubuntu:~$ ./chip-tool pairing ble-thread node-id hex:<active-dataset> 20202021 3840
```

<a name="using-the-thread-credential-sharing-mechanism"></a>

## Using the Thread credential sharing mechanism

The details about using the credential sharing mechanism are in the ot-nxp repo
border router application
[readme](https://github.com/NXP/ot-nxp/blob/v1.4.0-pvw1/examples/br/README-OTBR.md).
See `Ephemeral Key functionality` section.

Note that all OpenThread commands executed from then Matter CLI must have
`otcli` added before the command.
