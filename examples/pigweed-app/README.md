# Pigweed example app

This example app exercises functionalities in third_party/pigweed, to see what
is needed for integrating Pigweed to CHIP, as well as a precursor to CHIP
functionalities like on-device testing.

## Currently implemented features

### Echo RPC:

```
python -m pw_hdlc_lite.rpc_console --device /dev/ttyACM0 -b 115200 -p $CHIP_ROOT/third_party/pigweed/repo/pw_rpc/pw_rpc_protos/echo.proto -o /tmp/pw_rpc.out
```

will start an interactive python shell where Echo RPC can be invoked as

```
rpcs.pw.rpc.EchoService.Echo(msg="hi")
```
