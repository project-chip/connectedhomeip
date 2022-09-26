# Matter ESP32 Light-switch Example

This example demonstrates the Matter Light-switch application on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Testing the example](#testing-the-example)

---

## Testing the example

-   After successful commissioning, use the chip-tool to write the ACL in
    Lighting device to allow access from Lighting-switch device and chip-tool.

        $ ./out/debug/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<LIGHT SWITCH NODE ID>], "targets": null }]' <LIGHTING APP NODE ID> 0

-   After successful commissioning, use the chip-tool for binding in
    Lighting-switch.

        $ ./out/debug/chip-tool binding write binding '[{"fabricIndex": 1, "node":<LIGHTING APP NODE ID>, "endpoint":1, "cluster":6}]' <LIGHT SWITCH NODE ID> 1

-   Test toggle:

    Press `boot` button to toggle LED.

    Using matter shell on:

        matter switch onoff on

    off:

        matter switch onoff off
