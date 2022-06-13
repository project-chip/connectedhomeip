# Lock Application for Linux

This application is quite different from regular lock-app. The app showcases the
current implementation of the Door Lock cluster and doesn't rely on the On/Off
cluster to do the job.

For now it is not possible to change lock parameters from the app, the
functionality should be probably presented either as CLI or something that could
be controlled from tests (like RPC in lighting-app).

## Building

The application could be build in the same manner as `all-clusters-app`:

```
? scripts/examples/gn_build_example.sh examples/lock-app/linux out/lock-app chip_config_network_layer_ble=false
```
