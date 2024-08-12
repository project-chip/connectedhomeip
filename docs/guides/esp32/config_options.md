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
you want to place the app_main() function in a different component.

For required changes in the executable component, please refer to the
[esp-idf documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#renaming-main-component).

If you're building applications that support Matter and want to place app_main()
in a component other than main, use the following command:

```
idf.py -DEXECUTABLE_COMPONENT_NAME="your_component" build
```
