# Bluetooth Low Energy (BLE)

For Linux systems that use the bluez Bluetooth stack the
`--ble-controller <selector>` CLI argument expects a single integer to identify
Bluetooth controllers connected to the system. Specifically, the number after
`hci` is the selector. For example, when `hciconfig` returns the following:

```
$ hciconfig
hci0:	Type: Primary  Bus: UART
    BD Address: B8:27:EB:AA:BB:CC  ACL MTU: 1021:8  SCO MTU: 64:1
    UP RUNNING PSCAN ISCAN
    RX bytes:8609495 acl:14 sco:0 events:217484 errors:0
    TX bytes:92185 acl:20 sco:0 commands:5259 errors:0

hci1:	Type: Primary  Bus: USB
    BD Address: 00:1A:7D:AA:BB:CC  ACL MTU: 310:10  SCO MTU: 64:8
    UP RUNNING PSCAN ISCAN
    RX bytes:20942 acl:1023 sco:0 events:1140 errors:0
    TX bytes:16559 acl:1011 sco:0 commands:121 errors:0
```

`hci0` will be selected by default or when `--ble-controller 0` is specified,
`hci1` will be selected when `--ble-controller 1` is specified on the command
line. For example when using chip-tool:

```
$ out/linux-x64-light/chip-lighting-app --ble-controller 1  # Select hci1
```
