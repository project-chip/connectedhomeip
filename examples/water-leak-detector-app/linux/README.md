# Matter Linux Water Leak Detector Example

1. Build the sample app and the chip tool

    ```bash
    ./scripts/build/build_examples.py --target linux-arm64-water-leak-detector --target linux-arm64-chip-tool build
    ```

2. Launch the sample app

    ```bash
    ./out/linux-arm64-water-leak-detector/water-leak-detector-app
    ```

3. Commission the sample with with the chip tool

    ```bash
    ./out/linux-arm64-chip-tool/chip-tool pairing code 1 34970112332
    ```

4. Read the state of the Boolean State Cluster's Value attribute. The expected
   (default) state is "false"

    ```bash
    ./out/linux-arm64-chip-tool/chip-tool booleanstate read state-value 1 1
    ```

    ```
    [1747264560.361] [32470:32481] [DMG] ReportDataMessage =
    [1747264560.361] [32470:32481] [DMG] {
    [1747264560.361] [32470:32481] [DMG]    AttributeReportIBs =
    [1747264560.361] [32470:32481] [DMG]    [
    [1747264560.361] [32470:32481] [DMG]            AttributeReportIB =
    [1747264560.361] [32470:32481] [DMG]            {
    [1747264560.361] [32470:32481] [DMG]                    AttributeDataIB =
    [1747264560.361] [32470:32481] [DMG]                    {
    [1747264560.361] [32470:32481] [DMG]                            DataVersion = 0xe2ba2ae1,
    [1747264560.361] [32470:32481] [DMG]                            AttributePathIB =
    [1747264560.361] [32470:32481] [DMG]                            {
    [1747264560.361] [32470:32481] [DMG]                                    Endpoint = 0x1,
    [1747264560.361] [32470:32481] [DMG]                                    Cluster = 0x45,
    [1747264560.361] [32470:32481] [DMG]                                    Attribute = 0x0000_0000,
    [1747264560.361] [32470:32481] [DMG]                            }
    [1747264560.361] [32470:32481] [DMG]
    [1747264560.361] [32470:32481] [DMG]                            Data = false,
    [1747264560.361] [32470:32481] [DMG]                    },
    [1747264560.361] [32470:32481] [DMG]
    [1747264560.361] [32470:32481] [DMG]            },
    [1747264560.361] [32470:32481] [DMG]
    [1747264560.361] [32470:32481] [DMG]    ],
    [1747264560.361] [32470:32481] [DMG]
    [1747264560.361] [32470:32481] [DMG]    SuppressResponse = true,
    [1747264560.361] [32470:32481] [DMG]    InteractionModelRevision = 12
    [1747264560.361] [32470:32481] [DMG] }
    ```

5. Update the LeakStatus to "true""

    ```bash
    echo '{"Name":"LeakStatus","NewValue":1}' > /tmp/chip_water_leak_detector_fifo_*
    ```

6. Read the state of the Boolean State Cluster's Value attribute. The expected
   state is "true"

    ```bash
    ./out/linux-arm64-chip-tool/chip-tool booleanstate read state-value 1 1
    ```

    ```
    [1747264625.485] [1553:1558] [DMG] ReportDataMessage =
    [1747264625.485] [1553:1558] [DMG] {
    [1747264625.485] [1553:1558] [DMG]      AttributeReportIBs =
    [1747264625.485] [1553:1558] [DMG]      [
    [1747264625.485] [1553:1558] [DMG]              AttributeReportIB =
    [1747264625.485] [1553:1558] [DMG]              {
    [1747264625.485] [1553:1558] [DMG]                      AttributeDataIB =
    [1747264625.485] [1553:1558] [DMG]                      {
    [1747264625.485] [1553:1558] [DMG]                              DataVersion = 0xe2ba2ae2,
    [1747264625.485] [1553:1558] [DMG]                              AttributePathIB =
    [1747264625.485] [1553:1558] [DMG]                              {
    [1747264625.485] [1553:1558] [DMG]                                      Endpoint = 0x1,
    [1747264625.485] [1553:1558] [DMG]                                      Cluster = 0x45,
    [1747264625.485] [1553:1558] [DMG]                                      Attribute = 0x0000_0000,
    [1747264625.485] [1553:1558] [DMG]                              }
    [1747264625.485] [1553:1558] [DMG]
    [1747264625.485] [1553:1558] [DMG]                              Data = true,
    [1747264625.485] [1553:1558] [DMG]                      },
    [1747264625.485] [1553:1558] [DMG]
    [1747264625.485] [1553:1558] [DMG]              },
    [1747264625.485] [1553:1558] [DMG]
    [1747264625.485] [1553:1558] [DMG]      ],
    [1747264625.485] [1553:1558] [DMG]
    [1747264625.485] [1553:1558] [DMG]      SuppressResponse = true,
    [1747264625.485] [1553:1558] [DMG]      InteractionModelRevision = 12
    [1747264625.485] [1553:1558] [DMG] }
    ```

7. Update the LeakStatus to "false"

    ```bash
    echo '{"Name":"LeakStatus","NewValue":0}' > /tmp/chip_water_leak_detector_fifo*
    ```

8. Read the state of the Boolean State Cluster's Value attribute. The expected
   state is "false"

    ```bash
    ./out/linux-arm64-chip-tool/chip-tool booleanstate read state-value 1 1
    ```

    ```
    [1747264683.930] [3543:3555] [DMG] ReportDataMessage =
    [1747264683.930] [3543:3555] [DMG] {
    [1747264683.930] [3543:3555] [DMG]      AttributeReportIBs =
    [1747264683.930] [3543:3555] [DMG]      [
    [1747264683.930] [3543:3555] [DMG]              AttributeReportIB =
    [1747264683.930] [3543:3555] [DMG]              {
    [1747264683.930] [3543:3555] [DMG]                      AttributeDataIB =
    [1747264683.930] [3543:3555] [DMG]                      {
    [1747264683.930] [3543:3555] [DMG]                              DataVersion = 0xe2ba2ae3,
    [1747264683.930] [3543:3555] [DMG]                              AttributePathIB =
    [1747264683.930] [3543:3555] [DMG]                              {
    [1747264683.930] [3543:3555] [DMG]                                      Endpoint = 0x1,
    [1747264683.930] [3543:3555] [DMG]                                      Cluster = 0x45,
    [1747264683.930] [3543:3555] [DMG]                                      Attribute = 0x0000_0000,
    [1747264683.930] [3543:3555] [DMG]                              }
    [1747264683.930] [3543:3555] [DMG]
    [1747264683.930] [3543:3555] [DMG]                              Data = false,
    [1747264683.930] [3543:3555] [DMG]                      },
    [1747264683.930] [3543:3555] [DMG]
    [1747264683.930] [3543:3555] [DMG]              },
    [1747264683.930] [3543:3555] [DMG]
    [1747264683.930] [3543:3555] [DMG]      ],
    [1747264683.930] [3543:3555] [DMG]
    [1747264683.930] [3543:3555] [DMG]      SuppressResponse = true,
    [1747264683.930] [3543:3555] [DMG]      InteractionModelRevision = 12
    [1747264683.930] [3543:3555] [DMG] }
    ```
