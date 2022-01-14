# Matter Shell - Device Layer module

The chip::DeviceLayer APIs may be invoked via the Matter Shell CLI.

## Command List

-   [help](#help)
-   [config](#config)
-   [get](#get-parameter)
-   [start](#start)

## Command Details

### help

List the Device CLI commands.

```bash
> device help
  help            Usage: device <subcommand>
  start           Start the device layer. Usage: device start
  get             Get configuration value. Usage: device get <param_name>
  config          Dump entire configuration of device. Usage: device dump
Done
```

### config

Dump the configuration of the device.

```bash
> device config
VendorId:        235a
ProductId:       feff
HardwareVersion: 0001
SerialNumber:    <None>
ServiceId:       <None>
FabricId:        <None>
PinCode:         <None>
Discriminator:   <None>
DeviceId:        <None>
DeviceCert:      <None>
DeviceCaCerts:   <None>
MfrDeviceId:     <None>
MfrDeviceCert:   <None>
MfgDeviceCaCerts:<None>
```

### get \<parameter\>

-   parameter: name of field to query

Where valid parameter names include:

-   vendorid: Vendor Identifier
-   productid: Product Identifier
-   hardwarever: Hardware Version
-   serial: Serial Number
-   deviceid: Device Identification Number
-   cert: Device Certificate
-   cacerts: Device CA Certificates
-   mfrdeviceid: Manufacturer Device Identification Number
-   mfrcert: Manufacturer Device Certificate
-   mfrcacerts: Manufacturer Device CA Certs
-   pincode: Setup Pin Code
-   discriminator: Setup Discriminator
-   serviceid: Service Identifier
-   fabricid: Fabric Identifier

```bash
> device get vendorid
235a
Done
```

### start

Initialize the Matter stack and start the device layer event loop.

```bash
> device start
Init CHIP Stack
Starting Platform Manager Event Loop
Done
```
