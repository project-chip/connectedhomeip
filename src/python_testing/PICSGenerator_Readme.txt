### Setup ###
The script uses the json based data model in order to convert cluster identifiers into PICS Codes. The file can be downloaded here: https://groups.csa-iot.org/wg/matter-csg/document/27290 

The script uses the PICS XML templates for generate the PICS output, the PICS templates can be downloaded here:
https://groups.csa-iot.org/wg/matter-csg/document/26122

### How to run ###
In order to use the script the Python CHIP controller must be build, use the instructions at https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/python_chip_controller_building.md#building

cd into "connectedhomeip" folder

Activate the Python virtual environment using "source out/python_env/bin/activate"

If the "DUT" has not been commissioned this can be done by passing in the configuration in the following way:
"python3 'src/python_testing/PICSGenerator.py' --cluster-data '/Users/renejosefsen/Developer/GitData/connectedhomeip/src/python_testing/DeviceMapping/clusterData/Specification_version master da1249d.json' --pics-template '/Users/renejosefsen/Developer/GitData/connectedhomeip/src/python_testing/DeviceMapping/PICS/XML_version master Version V15' --pics-output '/Users/renejosefsen/Developer/GitData/connectedhomeip/src/python_testing/DeviceMapping/output' --commissioning-method ble-thread --discriminator <DESCRIMINATOR> --passcode <PASSCODE> --thread-dataset-hex <DATASET_AS_HEX> --paa-trust-store-path credentials/development/paa-root-certs"

If a "DUT" has alreasy ben commissioned, run the script using the following command: 
"python3 'src/python_testing/PICSGenerator.py' --cluster-data '/Users/renejosefsen/Developer/GitData/connectedhomeip/src/python_testing/DeviceMapping/clusterData/Specification_version master da1249d.json' --pics-template '/Users/renejosefsen/Developer/GitData/connectedhomeip/src/python_testing/DeviceMapping/PICS/XML_version master Version V15' --pics-output '/Users/renejosefsen/Developer/GitData/connectedhomeip/src/python_testing/DeviceMapping/output'"

- --cluster-data is the absolute path to the Specification_version master da1249d.json file
- --pics-template is the absolute path to the folder contaning the PICS templates (currently tested with V15)
- --pics-output is the absolute path to the output folder to be used