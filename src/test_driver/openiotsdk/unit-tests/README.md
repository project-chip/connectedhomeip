# Matter Open IoT Unit Tests Application

The Open IoT SDK Unit Tests Application executes all supported unit tests on the
`FVP Fast Model` target.

The Matter unit tests are included in a set of libraries and allow to validate
most of the components used by Matter examples applications. The main goal of
this project is to run registered tests on Open IoT SDK target and check the
results. The final result is the number of tests that failed.

## Build-run-test-debug

For information on how to setup, build, run, test and debug unit tests refer to
[Open IoT SDK unit tests](../../../../docs/guides/openiotsdk_unit_tests.md).

## Application output

Expected output of each executed test application:

```
[ATM] Open IoT SDK unit-tests start
[ATM] Open IoT SDK unit-tests run...
...
[ATM] Test status: 0
[ATM] Open IoT SDK unit-tests completed
```

This means the test application launched correctly and executed all registered
test cases. The `Test status` value indicates the number of tests that failed.
