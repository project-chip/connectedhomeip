## Code Savings Guide for Building Matter Applications

*   Remove unnecessary clusters from the zap configuration. Example applications have clusters enabled to support both Thread and WiFi transport layers such as the Diagnostics clusters. 

*   Remove optional features in Matter that may not be needed for a certain application. In the EFR32 build [script](../../../scripts/examples/gn_efr32_example.sh), there are additional build arguments that are added to either add or remove optional Matter features. For example, added a build argument disable_lcd=true will save flash by disabling the lcd screen. For inspiration, a minimalistic lighting application is [provided](../../../silabs_examples/lighting-lite-app/efr32/README.md). This example showcases the minimal requirements to build a lighting application over the Thread protocol.

---
[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) | [Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)