# Matter ASR Light Switch Example

This example demonstrates the Matter Light Switch application on ASR platform.

---

-   [Matter ASR Light Switch Example](#matter-asr-light-switch-example)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Testing the example](#testing-the-example)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-light-switch build
```

## Testing the example

-   An additional lighting device is required to complete this example.
-   Commission lighting device with node-id `1`
-   Commission light-switch device with node-id `2`
-   After successful commissioning, use the `chip-tool` to write the ACL in
    Lighting device to allow access from light-switch device and chip-tool.
    ```
    ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [2], "targets": null }]' 1 0
    ```
-   After successful commissioning, use the `chip-tool` for binding in
    light-switch.
    ```
    ./chip-tool binding write binding '[{"fabricIndex": 1, "node":1, "endpoint":1, "cluster":6}]' 2 1
    ```
-   Light switch button

    This demo uses button to test changing the state of lighting:

    |  Name  |  Pin  |
    | :----: | :---: |
    | BUTTON | PAD12 |
