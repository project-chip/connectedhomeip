# CHIP on-device testing

_Requirements and high-level design_

## Background

The ability to run tests on actual and emulated hardware is paramount in
embedded projects. CHIP is no exception. We want on-device testing to be a first
class goal of CHIP architecture. On-device testing requirements apply both to
Continuous Integration testing for main CHIP software stack development and to
eventual CHIP product certification. This document explores the requirements and
evaluates potential solutions.

## Overview of requirements

A good device test infrastructure is built on four pillars.

### Pillar 1: Using a test framework

A test framework provides a testing structure that developers can follow and
potentially reduces some of the burden of test setup and teardown (less
boilerplate). Support for state-oriented and asynchronous structuring of tests
would be beneficial. Many test frameworks leverage scripting languages such as
Python to simplify the quick development of tests and to leverage rich sets of
libraries for device/systems access and results generation.

### Pillar 2: Dispatching tests

Tests can run on lab machines or on the developer's local workstation. Tests can
be triggered manually by the developer or as a result of completion of a
changeset built on a continuous integration (CI) server. CHIP involves multiple
stakeholders, many of which will want to contribute to the testing efforts with
lab capacity. The infrastructure therefore must be prepared for
cross-organization test dispatch.

To facilitate uniform dispatch of tests we will probably need a simple
request/response protocol. Potentially HTTPS based and RESTful. Due to the long
running nature of device tests the response for a test scheduling request could
be a test ID, not the test result. That ID could be used to query the test
status, subscribe for notifications on status changes and to pull the test
results. Core aspects of such a scheme include the conventions for request
artifacts contents and minimum expected results contents once the run is
complete.

### Pillar 3: Interacting with devices

The test host environment has to reset devices, flash images on them, issue
commands, monitor status and collect test results. It may also need to integrate
both virtual (simulated) and real devices together. This can at first be done in
an ad-hoc way per platform but eventually we can go into device access
abstraction, i.e. define a common device testing interface which CHIP-compliant
devices can expose. The test host has to be prepared for driving multiple
devices at the same time for a single test, e.g. for tests that check
communication between multiple devices.

### Pillar 4: Collecting results

Ideally, test results are output in standard formats and similar or analogous
results between different devices and tests are output the same way. This
ensures reusability of code that processes similar data while allowing
aggregation of results across different dimensions. Failed tests must propagate
errors from device platform layers all the way to the CHIP stack and present
errors and potential stack traces in a standard result format. As the purpose of
on-device tests is to capture bugs, it is important that the test outputs
highlight the failure reason(s) and developers don't have to browse through
thousands of lines of logs to find the one line that sheds light on why a test
failed.

## Priorities

In the spirit of CHIP's charter, it would be great to see something taking-off
as soon as possible, to support continuous testing of the evolving CHIP stack.
We could then improve on that first iteration, even if we have to throw away
some temporary concepts and code.

Test dispatch (Pillar 2) arises as the highest priority, because all other
pillars can have ad-hoc solutions. The first need is an interface between a
CircleCI job and a test execution host at a participating organization. This
would enable dispatching tests to a variety of existing in-house infrastructure,
while retaining common request/response protocols to shield the CI system from
implementation details of each lab.

The next most important goal is to provide a test framework (Pillar 1). With a
standard framework developers can start writing tests, even if those tests will
be device specific and of ad-hoc input and output format. The general structure
of tests will however be present and later the tests can be adapted to standard
interactions (Pillar 3) and result formats (Pillar 4).

Specifying result formats (Pillar 4) for the most common outputs
(success/failure, failure reason, stack trace, memory and CPU usage time series,
pcaps of network traffic, etc.) will be an ongoing effort. The simplest output
formats can be specified together with the test framework.

Lastly, we want to look into a common device interaction interface that would
enable reusing tests between different devices.

## Baseline hardware platforms for CHIP

The TSG is targeting the following platforms/boards for early bringup:

-   Nordic nRF52 board <TODO: REF>
-   SiLabs `XXXX` board <TODO:REF>
-   Espressif ESP32 `XXXX` board <TODO:REF>
