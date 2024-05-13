# CHIP QPG6105 Persistent Storage Application

An example application showing the use of key value storage API on the Qorvo
QPG6105.

More detailed information to be included in
[SDK Documentation](../../platform/qpg/README.md).

## Persistent-storage-app

This example serves to both test the key value storage implementation and API as
it is brought-up on different platforms, as well as provide an example for how
to use the API.

In the future this example can be moved into a unit test when available on all
platforms.

## Persistent-storage-app button control

This application does not use any buttons.

## LED output

This application does not have any LED output.

## Logging Output

-   See [View Logging Output](../../platform/qpg/README.md)
-   At startup you will see:

```
qvCHIP v0.0.0.0 (CL:170621) r:3
============================
Qorvo KVS-Test Launching
============================
Starting FreeRTOS scheduler
Consistency fail - tag:20ef
Consistency failed
Running Tests:
[P][-] TestEmptyString(): PASSED
[P][-] TestString(): PASSED
[P][-] TestUint32(): PASSED
[P][-] TestArray(): PASSED
[P][-] TestStruct(): PASSED
[P][-] TestUpdateValue(): PASSED
[P][-] TestMultiRead(): PASSED
```
