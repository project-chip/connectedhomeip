# How to Add New Device Types & Clusters

This document outlines the process needed to add a new Matter device type and
related clusters. Obviously, the steps below assume that the related Matter
specifications were properly reviewed and approved.

## Add the cluster definitions to the SDK

The following steps should be followed to add new cluster definitions to the
SDK.

**Add your new cluster definition to an appropriately-name file in this
directory:**
[src/app/zap-templates/zcl/data-model/chip](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/data-model/chip)

**Add references to each new cluster definition to these files:**

1. [.github/workflows/tests.yaml](https://github.com/project-chip/connectedhomeip/tree/master/.github/workflows/tests.yaml)
2. [scripts/rules.matterlint](https://github.com/project-chip/connectedhomeip/tree/master/scripts/rules.matterlint)
3. [src/app/zap-templates/zcl/zcl-with-test-extensions.json](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/zcl-with-test-extensions.json)
4. [src/app/zap-templates/zcl/zcl.json](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/zcl.json)
5. If it is a derived cluster, add a reference to the base cluster definition.
   (e.g. in mode-base-cluster.xml you may need to add cluster codes - otherwise
   you may get strange exceptions which aren't clear when running regen_all.py)

    > ```
    > <struct name="ModeOptionStruct">
    >  <cluster code="0x0051"/> <!-- Laundry Washer Mode -->
    >  <cluster code="YOUR NEW CLUSTER ID"/>
    > </struct>
    > ```

6. [src/controller/python/chip/clusters/\_\_init\_\_.py](https://github.com/project-chip/connectedhomeip/tree/master/src/controller/python/chip/clusters/__init__.py)

**Enable your new cluster in the Python and Android clients** in
[src/controller/data_model/controller-clusters.zap](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/data_model/controller-clusters.zap)

You will need the ZAP tool to edit the ZAP file.

-   Unless you already have the tool installed, you can use one of the
    [nightly builds](https://github.com/project-chip/zap/releases)
-   [ZAP tool info](https://developers.home.google.com/matter/tools/zap)
-   [ZAP tool repo](https://github.com/project-chip/zap)

Use the ZAP GUI tool to edit the file above:

1. From the command line, navigate to the directory containing
   [controller-clusters.zap](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/data_model)
2. Run zap: `../../../scripts/tools/zap/run_zaptool.sh controller-clusters.zap`.
   Alternatively, run the zap tool and navigate to the zap file that you wish to
   open, or run as
   `./scripts/tools/zap/run_zaptool.sh src/controller/data_model/controller-clusters.zap`.
3. In the gui, select `Endpoint-1` from the left pane.
4. Open the cluster group, for example, `Appliances`.
5. Find the cluster to be enabled, e.g. `Dishwasher Control`.
6. In the Enable column, select "Client" from the drop-down box.
7. Click `File->Save` to save the configuration.
8. Close the GUI.

**Add entries for your new cluster to
[BUILD.gn](c/src/controller/data_model/BUILD.gn)** in the outputs section of the
java-jni-generate bits. The entries should look like
"jni/YourClusterNameClient-InvokeSubscribeImpl.cpp" and
"jni/YourClusterNameClient-ReadImpl.cpp".

**Add an entry to the ClientDirectories** section of
[src/app/zap_cluster_list.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap_cluster_list.json).

**Update `chip-tool`**

1. Regenerate all zap generated code using `./scripts/tools/zap_regen_all.py`
2. Rebuild chip-tool and it will have new cluster support:
   `./scripts/examples/gn_build_example.sh examples/chip-tool SOME-PATH/`

## Add the device type definition to the SDK

1. Add the XML definition of the device to
   [matter-devices.xml](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap-templates/zcl/data-model/chip/matter-devices.xml)
2. Implement the incoming command behaviors common to all applications. The
   parsing of the payload from TLV to a C++ struct is done by code
   auto-generated from the XML (see
   [zap-generated](https://github.com/project-chip/connectedhomeip/blob/master/zzz_generated/app-common/app-common/zap-generated))
   The rest of the functionality must be manually implemented. Note: for the
   auto-generated code run one of the following:
    1. for everything: `./scripts/tools/zap_regen_all.py`
    2. just for the app-common part:
       `./scripts/tools/zap/generate.py -t src/app/common/templates/templates.json -o zzz_generated/app-common/app-common/zap-generated src/controller/data_model/controller-clusters.zap`
3. Implement the read/write/storage operations for the attributes of any type,
   list, or struct which are not the global attributes present in all clusters.
   For example, there's no need to implement CommandList, AttributeList etc. For
   the attributes which are not list of struct type, the handling code is
   implemented generically so most likely no work is needed for them.
4. Implement any attribute spec requirements that are common to all
   applications. For example: code to enforce specific ranges, code for handling
   the interactions between attributes etc.

## Implement Code and Tests

Implement the clusters, the example cluster server application and add the
related SDK tests.

1. Implement new clusters here:
   [src/app/clusters](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters)
2. Implement tests here:
   [src/app/tests/suites](https://github.com/project-chip/connectedhomeip/tree/master/src/app/tests/suites)
3. Implement the example cluster server application:
    1. The YAML tests will run against this server
    2. Depending on the clusters, there are two options:
        1. Enable them in the all-clusters-app and use that as the example
           cluster server app. If choosing this option, consider adding an
           example application that has just the relevant clusters enabled; this
           part is a nice to have.
        2. If the clusters have complicated global application requirements
           consider using a separate example app. see the door lock, bridge, TV,
           OTA clusters.
    3. NOTES: If adding a new cluster derived from `mode-base` into
       [examples/all-clusters-app/](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/)
       (Assume your new cluster is called `XZYMode`):
        1. Create your new `xyz-mode-cluster.xml` in
           [src/app/zap-templates/zcl/data-model/chip](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/data-model/chip)
           (as above). Note that this is a much cut down cluster definition
           compared to normal clusters, since it derives from
           [mode-base-cluster.xml](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/data-model/chip/mode-base-cluster.xml).
           See
           [dishwasher-mode-cluster.xml](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/data-model/chip/dishwasher-mode-cluster.xml)
           as an example. Note you should review if you need the `StartUpMode`
           and `OnMode` attributes based on the spec.
        2. Check that you have added your cluster code into
           [mode-base-cluster.xml](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/data-model/chip/mode-base-cluster.xml)
            1. `<struct name="ModeTagStruct"> <cluster code="0xXXXX">` - replace
               `XXXX` with your cluster ID
            2. `<struct name="ModeOptionStruct"> <cluster code="0xXXXX">` -
               replace `XXXX` with your cluster ID
            3. `<bitmap name="Feature" type="bitmap32"> <cluster code="0xXXXX">` -
               replace `XXXX` with your cluster ID
        3. Add your new Mode definitions in `xyz-mode.h`. In this header you
           define the modes / labels and tags. See
           [dishwasher-mode.h](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/all-clusters-common/include/dishwasher-mode.h)
           as an example.
        4. Add your new stub to instantiate the mode. See
           [dishwasher-mode.cpp](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/all-clusters-common/src/dishwasher-mode.cpp)
           as an example.
        5. In
           [examples/all-clusters-app/linux/main-common.cpp](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/linux/main-common.cpp):
            1. Add `#include "xyz-mode.h"`
            2. In `ApplicationShutdown()`, add a call to your
               `Clusters::XYZMode::Shutdown();`.
        6. In
           [examples/all-clusters-app/linux/BUILD.gn](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/linux/BUILD.gn),
           add the `xyz-mode.cpp` file to the `sources` section.
        7. In
           [src/app/common/templates/config-data.yaml](https://github.com/project-chip/connectedhomeip/blob/master/src/app/common/templates/config-data.yaml):
            1. Add a `<XYZMode>::ModeTag` to the `EnumsNotUsedAsTypeInXML`
               section.
            2. Add an `XYZ Mode` entry to the
               `CommandHandlerInterfaceOnlyClusters` section.
        8. In
           [src/app/util/util.cpp](https://github.com/project-chip/connectedhomeip/blob/master/src/app/util/util.cpp),
           in the `// Cluster Init Functions...` section, add a void
           `MatterXYZModePluginServerInitCallback() {}` definition.
        9. In
           [src/app/zap-templates/zcl/zcl-with-test-extensions.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap-templates/zcl/zcl-with-test-extensions.json):
            1. Add the `xyz-mode-cluster.xml` to the `xmlFile` list
            2. In the `attributeAccessInterfaceAttributes` entry, add your new
               entry
               `"XYZ Mode": [ "SupportedModes", "CurrentMode", "FeatureMap" ]` -
               this will mean ZAP won't generate code to handle these attributes
        10. In
            [src/app/zap_cluster_list.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap_cluster_list.json):
            1. Add your `XYZ_MODE_CLUSTER: []` to `ClientDirectories: { }`
               object
            2. Add your `"XYZ_MODE_CLUSTER": ["mode-base-server"]` to
               `"ServerDirectories": { }`
    4. The code under
       [src/app/tests/suites/certification](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/suites/certification)
       for YAML or
       [src/python_testing](https://github.com/project-chip/connectedhomeip/tree/master/src/python_testing)
       for Python should ideally implement the test plan (section 4 below).
    5. A test under
       [src/app/tests/suites](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/suites)
       (not certification) can test anything, in addition to, or preceding the
       official YAML representing the official test plan.
4. Add the test plan, using the templates below:

    1. [cluster_test_plan_template.adoc](https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/template/cluster_test_plan_template.adoc)
    2. [test_plan_template.adoc](https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/template/test_plan_template.adoc)

    Also, ask to be added to the private `csg-tt-test-plans` Slack channel.

5. Note: the CHIP-Tool reference client is generated from XML
6. If applicable, add tests:
    1. For relatively simple tests, add YAML tests here:
       [src/app/tests/suites/certification](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/suites/certification).
       Remember to update this file:
       [src/app/tests/suites/certification/PICS.yaml](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/suites/certification/PICS.yaml)
    2. For more complex tests, add Python tests here:
       [src/python_testing](https://github.com/project-chip/connectedhomeip/tree/master/src/python_testing)
    3. To add tests to CI, if applicable:
        1. Add the Python tests here:
           [.github/workflows/tests.yaml](https://github.com/project-chip/connectedhomeip/tree/master/.github/workflows/tests.yaml).
           Remember to provide all arguments required for each Python script,
           such as the endpoint PIXIT.
        2. Add the YAML tests by editing this file:
           [src/app/tests/suites/ciTests.json](https://github.com/project-chip/connectedhomeip/tree/master/src/app/tests/suites/ciTests.json)
            1. Create a section ("MyDevice") which lists all YAML tests for your
               device
            2. Add the section's name to the list of items named "collection"
            3. Do a ZAP code regen: `./scripts/tools/zap_regen_all.py`.
7. Add the device type spec to the test plan tools:

    1. [tools/device_type_requirements](https://github.com/CHIP-Specifications/chip-test-plans/tree/master/tools/device_type_requirements)
    2. The file above is used by
       [src/app/tests/suites/certification/Test_TC_DESC_2_1.yaml](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/suites/certification/Test_TC_DESC_2_1.yaml)

    Note: the plan is to make the DM tools generate the device type requirements
    data based on the spec, so the above will become obsolete.

8. Add the device type to Chef:
   [examples/chef/devices](https://github.com/project-chip/connectedhomeip/tree/master/examples/chef/devices)

## Q & A

**Q1**: What kind of devices can be used for the test events? Can one of them be
the example cluster server app running on a RasPI? Do the independent
realizations need to run on vendor hardware or can they also run on generic
hardware, such as ESP32 or RasPI?

**A1**: one realization can be the test harness + the all clusters example app +
RasPI. the two independent realizations need to run on target hardware, which
may be mock-ups, prototypes etc

**Q2**: How can the Chef tool be used for any of the deliverables above?

**A2**: TBD

**Q3**: What is the process for using the Zap tool in order to auto-generate
code and to commit the results to the git repo?

**A3**: Search for zap_regen above. Add all the changed files to the repo after.

**Q4**: Where can the older cluster definitions be found?

**A4**: src/app/zap-templates/zcl/data-model/silabs/general.xml

**Q5**: Where can I find ZAP documentation?

**A5**: https://github.com/project-chip/zap/blob/master/README.md
