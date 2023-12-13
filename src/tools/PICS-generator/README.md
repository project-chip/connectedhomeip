---
orphan: true
---

# Tool information

This tool reads out the supported elements and generates the appropriate PICS
files for the device. The tool outputs the PICS for all endpoints and outputs
these in a single folder.

Note: The tool does relay on what the the device is able to express and for now
there are areas which the tool can not cover:

-   PICS in base.xml
-   Only mandatory events are marked as supported, since the global attribute
    with list of events is provisional

# Setup

This tool uses the python environment used by the python_testing efforts, which
can be built using the below command.

```
scripts/build_python.sh -m platform -i out/python_env
```

Once the python environment is build it can be activated using this command:

```
source out/python_env/bin/activate
```

The script uses the json based data model in order to convert cluster
identifiers into PICS Codes. The file can be downloaded here:
[https://groups.csa-iot.org/wg/matter-csg/document/27290](https://groups.csa-iot.org/wg/matter-csg/document/27290)

NOTE: The tool has been verified using the "Specification_version
0.7-spring2024.json" version.

The script uses the PICS XML templates for generate the PICS output. These files
can be downloaded here:
[https://groups.csa-iot.org/wg/matter-csg/document/26122](https://groups.csa-iot.org/wg/matter-csg/document/26122)

NOTE: The tool has been verified using V24 PICS (used for Matter 1.2
certification)

# How to run

The tool does, as mentioned above, have external dependencies, these are
provided to the tool using these arguments:

-   --cluster-data is the absolute path to the JSON file containing the cluster
    data
-   --pics-template is the absolute path to the folder containing the PICS
    templates
-   --pics-output is the absolute path to the output folder to be used

If the device has not been commissioned this can be done by passing in the
commissioning information:

```
python3 'src/python_testing/PICSGenerator.py' --cluster-data <pathToClusterJson> --pics-template <pathToPicsTemplateFolder> --pics-output <outputPath> --commissioning-method ble-thread --discriminator <DESCRIMINATOR> --passcode <PASSCODE> --thread-dataset-hex <DATASET_AS_HEX>
```

In case the device uses a development PAA, the following parameter should be
added.

```
--paa-trust-store-path credentials/development/paa-root-certs
```

In case the device uses a production PAA, the following parameter should be
added.

```
--paa-trust-store-path credentials/production/paa-root-certs
```

If a device has already been commissioned, the tool can be executed like this:

```
python3 'src/python_testing/PICSGenerator.py' --cluster-data <pathToClusterJson> --pics-template <pathToPicsTemplateFolder> --pics-output <outputPath>
```
