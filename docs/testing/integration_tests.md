# Integration and Certification Tests

Integration tests use a server and a controller or controllers to test the
behavior of a device. Certification tests are all integration tests. For
certified products, the device under test (DUT) is tested against one of the SDK
controller implementations (either chip-tool or the python-based controller,
depending on the test type). For software component certification, the software
component is tested against a sample device built from the SDK.

Certification tests require an accompanying certification test plan in order to
be used in the certification testing process. More information about test plans
can be found in the
[test plans repository](https://github.com/CHIP-Specifications/chip-test-plans/tree/master/docs).
Integration testing can also be used outside of the certification testing
program to test device behavior in the SDK. Certification tests are all run in
the [CI](./ci_testing).

There are two main integration test types:

-   [YAML](./yaml.md)
-   [Python framework](./python.md)

YAML is a human-readable serialization language that uses structured tags to
define test steps. Tests are defined in YAML, and parsed and run through a
runner that is backed by the chip-tool controller.

The Python framework tests are written in python and use the
[Mobly](https://github.com/google/mobly) test framework to execute tests.

## Which test framework to use

Both types of tests can be run through the Test Harness for certification
testing, locally for the purposes of development and in the CI for the SDK. The
appropriate test framework to use is whatever lets you automate your tests in a
way that is understandable, readable, and has the features you need

-   YAML
    -   pros: more readable, simpler to write, easy for ATLs to parse and
        understand
    -   cons: conditionals are harder (not all supported), no branch control,
        schema not well documented
-   python
    -   pros: full programming language, full control API with support for core
        (certs, commissioning, etc), less plumbing if you need to add features,
        can use python libraries
    -   cons: more complex, can be harder to read
