# CHIP ESP32 Persistent Storage Example

An example testing and demonstrating the key value storage API.

Refer to
[setup ESP-IDF and CHIP Environment](#setup-esp-idf-and-chip-environment) and
[Build, Flash, Monitor, and Commissioning](#build-flash-monitor-and-commissioning)
guides to get started.

---

-   [Introduction](#introduction)

---

### Introduction

This example serves to both test the key value storage implementation and API as
it is brought-up on different platforms, as well as provide an example for how
to use the API.

In the future this example can be moved into a unit test when available on all
platforms.

The ESP32 platform KVS is not yet fully implemented. In particular offset and
partial reads are not yet supported.
