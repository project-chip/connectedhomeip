# Testing Guides

The following guide provide an introduction to the testing mechanisms available
in the SDK.

```{toctree}
:glob:
:maxdepth: 1
:hidden:

*
```

## SDK development

### Integration and Certification tests

Integration tests test the entire software stack using the same mechanisms as a
standard controller.

![](./img/integration_tests.png)

The certification tests are all integration tests, since they run against the
product as a black box.

-   [Integration and Certification tests](./integration_tests.md)
-   [YAML](./yaml.md)
-   [Python testing framework](./python.md)
-   [Enabling tests in the CI](./ci_testing.md)
-   [Integration test utilities](./integration_test_utilities.md)

### Unit testing

Unit tests run on small pieces (“units”) of business logic. They do not use an
external controller and instead test at the public interface of the class or
function. For clusters, this requires an API that separates the cluster logic
from the global ember and message delivery layers.

![](./img/unit_tests.png)

-   [Unit tests](./unit_testing.md)

### Fuzz testing

Fuzz Testing involves providing random and unexpected data as input to functions
and methods to uncover bugs, security vulnerabilities, or to determine if the
software crashes.

Information on how to develop and run fuzz tests for SDK code can be found here:

-   [Fuzz Testing](./fuzz_testing.md)

### Testing in the CI

-   [CI testing](./ci_testing.md)

## Testing a product for certification

Product development and certification is a process that varies heavily based on
the product and your company policies and decisions. The documents provided in
this section aim to provide guidance around the general process for product
development and certification. This is not aiming to be a complete guide for all
products, but a knowledge base around common mechanisms and challenges.

-   [Product development and Certification](./product_development_and_certification/index.html)

## Developing Certification tests for a feature
