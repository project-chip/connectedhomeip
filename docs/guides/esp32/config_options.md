# Configuration options

This file lists down few config options to be configured through menuconfig for
specific scenarios.

### Building with IPV4 Disabled

Configure below options through `idf.py menuconfig` and build the app.

```
CONFIG_DISABLE_IPV4=y
CONFIG_LWIP_IPV4=n
```
