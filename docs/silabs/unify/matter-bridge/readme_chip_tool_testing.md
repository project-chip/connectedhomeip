# Introduction
This README shows the PICS file of various chip-tool tests executed on Unify Matter Bridge with Z-Wave protocol controller

For information on what is PICS, refer [README](../../src/app/tests/suites/README.md)

And section [Running chip-tool tests on Unify Matter Bridge endpoints](./readme_user.md##running-chip-tool-tests-on-unify-matter-bridge-endpoints)

## BridgeDeviceBasicInfo

Use [Test_TC_BRBINFO.PICS](./PICS/Test_TC_BRBINFO.PICS) for following tests

## OccupancySensing 

Use [Test_TC_OCC.PICS](./PICS/Test_TC_OCC.PICS) for following tests

## Level

Use [Test_TC_LEVEL.PICS](./PICS/Test_TC_LEVEL.PICS)

## OnOff

Note: Running chip-tool tests for OnOff cluster needs '`--delayInMs 1000`' command line option to '`chip-tool`' to handle delays between Bridge and Unify

Use [Test_TC_OO.PICS](./PICS/Test_TC_OO.PICS)

## ColorControl

Use [Test_TC_CC.PICS](./PICS/Test_TC_CC.PICS)

## Groups

Use  [Test_TC_G.PICS](./PICS/Test_TC_G.PICS)


## General Diagnostics

Use [Test_TC_DGGEN.PICS](./PICS/Test_TC_DGGEN.PICS)

## Thermostat

Use [Test_TC_TSTAT.PICS](./PICS/Test_TC_TSTAT.PICS)
# Test results

When running the Unify Matter Bridge with the chip-tool tests, multiple 
factors may affect the test result. First of all the capabilities of the backing protocol controller is important. Many required features of Matter are optional in Unify, so in some cases the protocol controller is not able to do the proper translation which is required for the test cases to pass. The capabilities of actual end device may also affect the test results.


|  Test Suite          | ZPC         | PICS File            |Z-Wave End Device| Notes     |
|----------------------|-------------|----------------------|-----------------|-----------|
| Test_TC_BRBINFO_1_1  | OK          | Test_TC_BRBINFO.PICS ||           |
| Test_TC_BRBINFO_2_1  | OK          | Test_TC_BRBINFO.PICS ||           |
| Test_TC_BRBINFO_2_2  | OK          | Test_TC_BRBINFO.PICS ||           |
| Test_TC_BRBINFO_2_3  | OK          | Test_TC_BRBINFO.PICS |PowerStrip Sample |           |
| Test_TC_DGGEN_1_1| OK          | Test_TC_DGGEN.PICS |endpoint 1|           |
| Test_TC_DGGEN_2_1| OK          | Test_TC_DGGEN.PICS |endpoint 1|           |
| Test_TC_DGGEN_2_2| OK          | Test_TC_DGGEN.PICS |endpoint 1|           |
| Test_TC_DGGEN_2_3| OK          | Test_TC_DGGEN.PICS |endpoint 1|           |
| Test_TC_G_1_1  | OK          | Test_TC_G.PICS |Zipato Bulb or endpoint 1|           |
| Test_TC_G_2_1  | OK          | Test_TC_G.PICS |Zipato Bulb or endpoint 1|           |
| Test_TC_G_2_2  | OK          | Test_TC_G.PICS |Zipato Bulb or endpoint 1|           |
| Test_TC_G_2_3  | OK          | Test_TC_G.PICS |Zipato Bulb or endpoint 1|           |
| Test_TC_OO_1_1       | OK          | Test_TC_OCC.PICS |Boundary Motion Sensor|           |
| Test_TC_OO_2_1       | OK          | Test_TC_OCC.PICS |Boundary Motion Sensor|           |
| Test_TC_OO_2_3       | OK          | Test_TC_OCC.PICS |Boundary Motion Sensor|           |
| Test_TC_OO_1_1       | OK          | Test_TC_OO.PICS |Zipato Bulb|           |
| Test_TC_OO_2_1       | OK          | Test_TC_OO.PICS |Zipato Bulb|           |
| Test_TC_OO_2_2       | OK          | Test_TC_OO.PICS |Zipato Bulb|           |
| Test_TC_OO_2_3       | OK          | Test_TC_OO.PICS |Zipato Bulb|           |
| Test_TC_LVL_1_1      | OK          | Test_TC_LEVEL.PICS   |PowerStrip Sample|           |
| Test_TC_LVL_1_2      | OK          | Test_TC_LEVEL.PICS   |PowerStrip Sample|           |
| Test_TC_LVL_2_2      | 3 Failures  | Test_TC_BRBINFO.PICS |PowerStrip Sample| Due to missing information of startup level on Z-Wave we are not able to deduct this.
| Test_TC_LVL_2_3      | NA          |                      |PowerStrip Sample|           |
| Test_TC_LVL_3_1      | 10 Failures |Test_TC_LEVEL.PICS    |PowerStrip Sample| Due to timing issues reported values falls outside the expected range |
| Test_TC_LVL_4_1      |  4 Failures |Test_TC_LEVEL.PICS    |PowerStrip Sample| Due to timing issues reported values falls outside the expected range |
| Test_TC_LVL_5_1      |  1 Failures |Test_TC_LEVEL.PICS    |PowerStrip Sample| Due to timing issues reported values falls outside the expected range |
| Test_TC_LVL_6_1      | 2 Failures |Test_TC_LEVEL.PICS    |PowerStrip Sample| Due to timing issues reported values falls outside the expected range |
| Test_TC_LVL_7_1  | NA          | Test_TC_LEVEL.PICS ||           |
| Test_TC_LVL_8_1  | NA          | Test_TC_LEVEL.PICS ||           |
| Test_TC_STAT_1_1 | OK          | Test_TC_TSTAT.PICS ||           |
| Test_TC_STAT_2_1 | OK          | Test_TC_TSTAT.PICS ||           |
| Test_TC_STAT_2_2 | OK          | Test_TC_TSTAT.PICS ||           |
|                  |             |                    ||           |
