# log-source-app

This is a reference application that implements an example of an diagnostic log
cluster server.

## Building

Suggest doing the following:
`scripts/examples/gn_build_example.sh examples/log-source-app/linux out/debug chip_config_network_layer_ble=false`

## Usage

`./log-source-app`

## Current Features/Limitations

### Features

-   Redirect logs into the internal buffer, send logs using diagnostic logs
    cluster.

### Limitations:

-   BDX transfer is not implemented yet (TODO)
