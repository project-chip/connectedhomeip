# Configuration options

This file lists down few config options to be configured through menuconfig for
specific scenarios.

### Building with IPV4 Disabled

Configure below options through `idf.py menuconfig` and build the app.

```
CONFIG_DISABLE_IPV4=y
CONFIG_LWIP_IPV4=n
```

### Executable component not in "main" component

The ESP-IDF framework allows renaming the main component, which can be useful if
you want to place the `app_main()` function in a different component.

For required changes in the executable component, please refer to the
[esp-idf documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#renaming-main-component).

You need to list the required components in `idf_component_register()`. If this
module contains Matter related code, you may need to include
`chip, app_update, spi_flash, and nvs_flash` as `PRIV_REQUIRES`, along with any
other necessary dependencies.
