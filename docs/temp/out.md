Thursday 3/21 Stamford Ballroom 9:30–10:30 AM

Matter TSG \- Testable SDK Code \[MA\-TSG\-02\-PM3\-006\]

# Attendance Check-In

Scan this QR Code in your Canapii app to check\-in\. Your session attendance must be tracked for voting rights purpose\.

Open the Canapii

app & click Scan QR

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session0.png)

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session1.png)

# Connectivity Standards Alliance policy reminders

__Participation Notice: __ You are participating here as a member company of the Connectivity Standards Alliance \(Alliance\) and your organization has signed the Membership Agreement; you are responsible to be aware of its contents and your obligations with regards to it\.  Activities must adhere to Alliance Policies\, Processes and Procedures as well as basic principles such as fairness\, due process\, respect for minority opinions\, and common sense\.

__Code of Conduct Notice:__  The Alliance is committed to creating a safe and healthy environment that promotes respect\, dignity\, integrity\, professionalism\, ethical behavior and compliance with applicable laws and regulations\.  Your participation here is subject to the Alliance Code of Conduct\.

__Confidentiality Notice: __ You are required to keep all discussions and documents CONFIDENTIAL to the Alliance member companies of record\.

__Recording of Meeting__ :  If this meeting is being recorded\, the purposes of the recording are to provide the opportunity for those who are unable to join this session the ability to listen to this session later and meet their attendance requirements\, and to ensure that accurate minutes of the meeting can be prepared\.  If you do not agree to the meeting being recorded\, please announce your disagreement and the session moderator will excuse you from the meeting\. The recording will be available on Causeway for 30 days after which it will be deleted in accordance with the Alliance’s Record Retention Policy\.

__IPR Policy Notice: __ In connection with the development of an Alliance deliverable that requires use of patented inventions\, the use of patented inventions shall be governed by the Alliance IPR Policy\.

__Antitrust Risk Notice: __ Attendees are reminded that participation in industry fora involves the potential for antitrust concerns or risks\. To avoid such concerns and risks\, participants should carefully observe Alliance Antitrust Guidelines\.

__Questions__ : Participants having questions about these policies or procedures should consult first with their company’s legal counsel\, and subsequently with Alliance leadership or counsel as necessary\.

This slide only provides a summary of Alliance policies\.

The full text is available at:[ ](https://csa-iot.org/resources/governing-documents/) <span style="color:#0B4CB4"> _[https://csa\-iot\.org/resources/governing\-documents/](https://csa-iot.org/resources/governing-documents/)_ </span>  <span style="color:#193BF0"> </span>

---

Do not need to read these, just need to be posted per Ps and Ps.


# Testing in the SDK:
Unit tests, integration tests and CI

<span style="color:#FFFFFF">Singapore MM 2024 \- Thursday AM2</span>

---

Welcome to the testable SDK code session. As you probably know, for 1.4, there are some new requirements around code quality that are going to be enforced going forward, and that includes requiring tests for new incoming code. Given that we didn’t require this before, it tends to be less common than one might hope, so we put together this session to help new devs navigate how to do that, and the tools that are available.

However, before we begin, we need to continue a small Google tradition for these types of sessions, and thus I shall begin with a dad joke. What did the pirate say on his 80th birthday? I’m 80.

# Agenda

* Unit tests
  * Unit tests vs\. integration tests
  * SDK unit testing basics
  * Designing clusters for testing and portability
  * Unit testing for existing clusters
  * Utilities
  * Debugging unit tests
* Integration testing
  * TestEventTriggers\, NamedPipes and FaultInjection
  * Whole stack testing
* 1\.4 Testing expectations

# Unit Testing vs. Integration Testing

# Integration tests

Cluster Implementations

In SDK \- YAML and Python tests \(cert and non\-cert\)

Use an external controller

Tests the entire software stack with real components

Ember \(generated\)

Matter Core Protocol

Implementation

Platform Implementation

Cluster Implementations

Ember \(generated\)

__Matter Core Protocol__

__Implementation__

Platform Implementation

# Unit tests

Runs on small pieces \(“units”\) of business logic\.

Uses no external controller

Tests at the public interface of classes and functions\.

Requires definition of an API that separates the cluster logic from the global Ember and message delivery layers\.

Cluster Implementations

Ember \(generated\)

__Matter Core Protocol__

__Implementation__

Platform Implementation

Cluster Implementations

---

Cut off the bottom layers and drive the tests at the cluster implementation API layer

# Unit testing: Why?

* __Much __ faster than integration tests\.
* Runs as part of build process\.
* Allows testing specific error conditions that are difficult to trigger under normal operating conditions\.
  * e\.g\. out of memory errors etc\.
* Allows testing different device compositions without defining multiple example applications\.
  * e\.g\. feature combinations not in example apps\.

# Unit Testing in the SDK

# SDK unit testing

Right now we’re still using nlUnitTest\, we can demo how to do that\, or point to a reference

there was\, at one point\, a push to move to PW unit test\. Did that go anywhere?

Need a good\, compact demo here\, probably one already in the SDK

# Unit testing - nlUnitTest

\#include \<lib/support/UnitTestContext\.h>

\#include \<lib/support/UnitTestRegistration\.h>

\#include \<nlunit\-test\.h>

static void TestName\(nlTestSuite \* apSuite\, void \* apContext\) \{

// Do some testy things here\, then check the results using NL\_TEST\_ASSERT

NL\_TEST\_ASSERT\(apSuite\, \<boolean condition>\)

\}

See src/lib/support/tests/TestSpan\.cpp for a great example of a good unit test\.

---

Each test gets an nlTestSuite object that is passed into the test assertions, and a void* context that is yours to do with as you reuqire

class YourTestContext : public Test::AppContext \{

\};

static void TestName\(nlTestSuite \* inSuite\, void \* apContext\) \{

// If you register the test suite with a context\, cast

// apContext as appropriate

YourTestContext \* ctx = static\_cast\<YourTestContext \*>\(aContext\);

\.\.\.  some work \.\.\.

NL\_TEST\_ASSERT\(inSuite\, value == 17\);

\}

---

Inherit from AppContext, cast to your class type. Instantiated as part of the test setup. see next slide

static const nlTest sTests\[\] =

\{

NL\_TEST\_DEF\("TestName"\, TestName\)\,  // Can have multiple of these

NL\_TEST\_SENTINEL\(\)      // If you forget this\, you’re going to have a bad time

\};

nlTestSuite sSuite =

\{

"TheNameOfYourTestSuite"\,  // Test name

&sTests\[0\]\,                // The list of tests to run

TestContext::Initialize\,   // Runs before all the tests \(can be nullptr\)

TestContext::Finalize      // Runs after all the tests \(can be nullptr\)

\};

int YourTestSuiteName\(\)

\{

return chip::ExecuteTestsWithContext\<YourTestContext>\(&sSuite\);  // or “without”

\}

CHIP\_REGISTER\_TEST\_SUITE\(YourTestSuiteName\)

# nlUnitTest - Compiling and running

* Add to src/some\_directory/tests/BUILD\.gn
  * chip\_test\_suite\_using\_nltest\("tests"\)
    * See for example src/lib/support/tests/BUILD\.gn
* \./gn\_build\.sh will build and run all tests
  * CI runs this\, so any unit tests that get added will automatically be added to the CI
* Test binaries are compiled into:
  * out/debug/\<host\_compiler>/tests
  * e\.g\. out/debug/linux\_x64\_clang/tests
* Tests are run when \./gn\_build\.sh runs\, but you can run them individually in a debugger from their location\.

# Designing Clusters for Testing and Portability

# New cluster development: designing for testing and portability

* Talk about your disco ball approach
* code snippets? vid of running these? Can we demo the disco balls in the session?
* Do you want to land part of the disco ball \(ex no features?\) before the MM to point people at as a demo? Or just show some code? Problem here is that we don’t want the disco ball XML landing in the SDK because it can end up being part of the public APIs
* main points
  * majority of the logic should be pulled out into a testable class and the attribute writing portion should be a light wrapper at best
  * base logic class should handle all the common code \- constraint checking\, ensuring the incoming requests are well formed etc\.
    * this should be unit tested very well
  * carefully consider the public API required to act on the required changes \(ie the software \-> hardware layer\)
    * this can be swapped out for testing
  * can \(and should\) also unit test the AttributeAccessInterface layer to ensure errors returned from the logic class are properly encoded

# Unit Testable, Modular Cluster Design

* Separate the cluster logic from the on\-the\-wire data model
  * Server vs\. ClusterLogic
  * Makes the cluster logic unit\-testable without generating TLV\.
* Separate the basic cluster logic from code that is platform\- or device\-specific\.
  * ClusterLogic uses a ClusterDriver
  * Makes the cluster logic portable between platforms / manufacturers
  * Removes necessity of overriding global singleton functions like PostAttributeChangeCallback\.

Interaction

Model

__Matter__

__wire__

__protocol__

# Cluster logic APIs - A design pattern proposal

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session2.png)

---

ClusterServerClass
light wrapper over the cluster logic class
ClusterLogic
all common logic, per endpoint
ClusterDriver
platform specific interface to ex. gpios or whatnot

# ClusterServerClass

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session3.png)

* __Very__  light wrapper over ClusterLogic
  * Translates Interaction Model wire format handling into API calls for of cluster logic methods\.
* Implements both the AttributeAccessInterface and the CommandHandler interfaces so ClusterLogic properly handles data dependencies between command and attributes\.

<span style="color:#4EC9B0"> __CHIP\_ERROR__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __DiscoBallServer__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#DCDCAA"> __Read__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#569CD6"> __const__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __ConcreteReadAttributePath__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\,__ </span>

<span style="color:#4EC9B0"> __    AttributeValueEncoder__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __\{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#4EC9B0"> __DiscoBallClusterLogic__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\*__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __= __ </span>  <span style="color:#DCDCAA"> __FindEndpoint__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mEndpointId__ </span>  <span style="color:#CCCCCC"> __\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#569CD6"> __VerifyOrReturnError__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\!=__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __nullptr__ </span>  <span style="color:#CCCCCC"> __\, __ </span>

<span style="color:#569CD6"> __  CHIP\_IM\_GLOBAL\_STATUS__ </span>  <span style="color:#CCCCCC"> __\(UnsupportedEndpoint\)\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __switch__ </span>  <span style="color:#CCCCCC"> __ \(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mAttributeId__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __   \{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __case__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __Clusters__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __DiscoBall__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Attributes__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Run__ </span>  <span style="color:#CCCCCC"> __::Id:__ </span>

<span style="color:#CCCCCC"> __       __ </span>  <span style="color:#C586C0"> __return__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#DCDCAA"> __Encode__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __\->__ </span>  <span style="color:#DCDCAA"> __GetRunAttribute__ </span>  <span style="color:#CCCCCC"> __\(\)\);__ </span>

<span style="color:#CCCCCC"> __   …__ </span>

<span style="color:#CCCCCC"> __   \}__ </span>

<span style="color:#CCCCCC"> __\}__ </span>

<span style="color:#4EC9B0"> __CHIP\_ERROR__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __DiscoBallServer__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#DCDCAA"> __Read__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#569CD6"> __const__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __ConcreteReadAttributePath__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\,__ </span>

<span style="color:#4EC9B0"> __    AttributeValueEncoder__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __\{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#4EC9B0"> __DiscoBallClusterLogic__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\*__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __= __ </span>  <span style="color:#DCDCAA"> __FindEndpoint__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mEndpointId__ </span>  <span style="color:#CCCCCC"> __\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#569CD6"> __VerifyOrReturnError__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\!=__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __nullptr__ </span>  <span style="color:#CCCCCC"> __\, __ </span>

<span style="color:#569CD6"> __  CHIP\_IM\_GLOBAL\_STATUS__ </span>  <span style="color:#CCCCCC"> __\(UnsupportedEndpoint\)\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __switch__ </span>  <span style="color:#CCCCCC"> __ \(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mAttributeId__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __   \{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __case__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __Clusters__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __DiscoBall__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Attributes__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Run__ </span>  <span style="color:#CCCCCC"> __::Id:__ </span>

<span style="color:#CCCCCC"> __       __ </span>  <span style="color:#C586C0"> __return__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#DCDCAA"> __Encode__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __\->__ </span>  <span style="color:#DCDCAA"> __GetRunAttribute__ </span>  <span style="color:#CCCCCC"> __\(\)\);__ </span>

<span style="color:#CCCCCC"> __   …__ </span>

<span style="color:#CCCCCC"> __   \}__ </span>

<span style="color:#CCCCCC"> __\}__ </span>

<span style="color:#4EC9B0"> __CHIP\_ERROR__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __DiscoBallServer__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#DCDCAA"> __Read__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#569CD6"> __const__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __ConcreteReadAttributePath__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\,__ </span>

<span style="color:#4EC9B0"> __    AttributeValueEncoder__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __\{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#4EC9B0"> __DiscoBallClusterLogic__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\*__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __= __ </span>  <span style="color:#DCDCAA"> __FindEndpoint__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mEndpointId__ </span>  <span style="color:#CCCCCC"> __\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#569CD6"> __VerifyOrReturnError__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\!=__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __nullptr__ </span>  <span style="color:#CCCCCC"> __\, __ </span>

<span style="color:#569CD6"> __  CHIP\_IM\_GLOBAL\_STATUS__ </span>  <span style="color:#CCCCCC"> __\(UnsupportedEndpoint\)\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __switch__ </span>  <span style="color:#CCCCCC"> __ \(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mAttributeId__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __   \{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __case__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __Clusters__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __DiscoBall__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Attributes__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Run__ </span>  <span style="color:#CCCCCC"> __::Id:__ </span>

<span style="color:#CCCCCC"> __       __ </span>  <span style="color:#C586C0"> __return__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#DCDCAA"> __Encode__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __\->__ </span>  <span style="color:#DCDCAA"> __GetRunAttribute__ </span>  <span style="color:#CCCCCC"> __\(\)\);__ </span>

<span style="color:#CCCCCC"> __   …__ </span>

<span style="color:#CCCCCC"> __   \}__ </span>

<span style="color:#CCCCCC"> __\}__ </span>

<span style="color:#4EC9B0"> __CHIP\_ERROR__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __DiscoBallServer__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#DCDCAA"> __Read__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#569CD6"> __const__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __ConcreteReadAttributePath__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\,__ </span>

<span style="color:#4EC9B0"> __    AttributeValueEncoder__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __&__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __\{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#4EC9B0"> __DiscoBallClusterLogic__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\*__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __= __ </span>  <span style="color:#DCDCAA"> __FindEndpoint__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mEndpointId__ </span>  <span style="color:#CCCCCC"> __\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#569CD6"> __VerifyOrReturnError__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#D4D4D4"> __\!=__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#569CD6"> __nullptr__ </span>  <span style="color:#CCCCCC"> __\, __ </span>

<span style="color:#569CD6"> __  CHIP\_IM\_GLOBAL\_STATUS__ </span>  <span style="color:#CCCCCC"> __\(UnsupportedEndpoint\)\);__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __switch__ </span>  <span style="color:#CCCCCC"> __ \(__ </span>  <span style="color:#9CDCFE"> __aPath__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#9CDCFE"> __mAttributeId__ </span>  <span style="color:#CCCCCC"> __\)__ </span>

<span style="color:#CCCCCC"> __   \{__ </span>

<span style="color:#CCCCCC"> __   __ </span>  <span style="color:#C586C0"> __case__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#4EC9B0"> __Clusters__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __DiscoBall__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Attributes__ </span>  <span style="color:#CCCCCC"> __::__ </span>  <span style="color:#4EC9B0"> __Run__ </span>  <span style="color:#CCCCCC"> __::Id:__ </span>

<span style="color:#CCCCCC"> __       __ </span>  <span style="color:#C586C0"> __return__ </span>  <span style="color:#CCCCCC"> __ __ </span>  <span style="color:#9CDCFE"> __aEncoder__ </span>  <span style="color:#CCCCCC"> __\.__ </span>  <span style="color:#DCDCAA"> __Encode__ </span>  <span style="color:#CCCCCC"> __\(__ </span>  <span style="color:#9CDCFE"> __cluster__ </span>  <span style="color:#CCCCCC"> __\->__ </span>  <span style="color:#DCDCAA"> __GetRunAttribute__ </span>  <span style="color:#CCCCCC"> __\(\)\);__ </span>

<span style="color:#CCCCCC"> __   …__ </span>

<span style="color:#CCCCCC"> __   \}__ </span>

<span style="color:#CCCCCC"> __\}__ </span>

# ClusterLogic

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session4.png)

* All code that is SHARED between platforms\.
* Attribute getter/setter and command handling
  * No TLV parsing\.
  * No direct calls to Ember/IM/LogEvent etc\.
* Receives “plain data” Matter requests from ClusterServer class\, performs required common actions\, and calls driver class to perform platform\- or hardware\-specific actions\.
* Receives driver updates \(e\.g\. application\-driven value changes\) and updates state as appropriate\.\.

* Should handle spec\-requirements for:
  * Range checking \(CONSTRAINT\_ERROR\)
  * Attribute and metadata storage \(persistent or in\-memory\)
  * Data dependencies between commands and attributes
  * Event generation / dirty attributes \(callback to server\)
  * Calling driver when platform or hardware interactions are required
* API recommendation:
  * Maintain all cluster state in a separate data\-only class\.
  * Provider getters/setters for application logic to use\.
  * Implement handlers for all commands\, conditional on features\.
  * Let the caller provide \(inject\) dependencies\. Avoid explicit memory management\.

# ClusterDriver

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session5.png)

Implements hardware or platform\-specific actions required on cluster interactions or when application wants to report state changes\.

# MatterContext

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session6.png)

* Logic must not directly use global resource because they cannot be isolated for testing\.
* MatterContext holds pointers to Matter stack objects which can be injected/faked for testing
  * Wrapper over IM Engine interface functions for marking attributes dirty\, and logging events\.
  * Storage
  * Anything you’d normally access with Server::GetInstance\(\)

# ClusterDriver

* Called by ClusterLogic
  * Translates attribute changes and commands into application action\.
  * Reports external changes back to the ClusterLogic\.
* API design will vary by cluster
  * Recommend using a generic API where possible so the driver can port easily to other platforms\.
    * e\.g\. attribute changed callback with changes listed
  * It’s important to be careful about the design and revisit it early if issues arise\.

# Unit testing with ClusterLogic

# Unit testing the ClusterLogic model

* Unit test instantiates the ClusterLogic\, provides MatterContext and ClusterDriver instance with fakes/mocks for testing\.
* Unit test against the API\, check the fakes/mocks to ensure they are being called as appropriate\.
  * Prefer testing for behavior rather than implementation details\.

# Unit testing the ClusterLogic

Important tests to consider:

Initialization and initial attribute correctness\.

Errors for out\-of\-range on all attribute setters and command handlers\.

All spec\-defined error conditions\, especially ones that are difficult to trigger\.

Data dependencies between commands and attributes\.

Incoming actions in different states \(stopped\, running\, etc\)\.

Calls out to storage for persistent attributes\.

Calls out to driver for changes as appropriate\.

Driver error reporting\.

Event generation and dirty attribute marking\, including attributes that are changed from the driver side\.

Others \- very dependent on the cluster\.

# Unit testing ClusterServer

* Best to have the lightest wrapping possible
  * If the wrapper is light\, the code can be covered by integration or unit tests\, or a combination\.
  * Correctness can mostly be validated by inspection if it’s trivial\.
* Important tests
  * Errors when ClusterLogic instances aren’t properly registered\.
  * Flow through to ClusterLogic for all reads/writes/command\.
* Can unit test this class by generating the TLV / path for input\, parsing the TLV output\.

# Unit testing existing clusters

* Important for clusters where there are multiple configurations that cannot easily be represented with example apps
* Option 1
  * Refactor the cluster logic to be unit\-testable\.
* Option 2
  * Test at AttributeAccessInterface boundary\.
  * Instantiate or access the cluster server instance in the test\.
  * Read / Write TLV and use TLV encode/decode functions to verify correctness\.
  * See TestPowerSourceCluster for an example of how to do this
* Additional test coverage on clusters\, especially for hard to trigger conditions\, is important\. However\,  __don’t let perfection be the enemy of progress__ \.

# Unit test utilities

# Utilities

* Mock clock
  * System::Clock::Internal::MockClock
  * src/system
* TestPersistentStorageDelegate
  * In\-memory version of storage that easily allows removal of keys\, presence checks\, etc\.
  * src/lib/support
* Consider writing fakes for use across tests as you write your own unit tests

# Debugging unit tests

* After running \./gn\_build\.sh\, test binaries are compiled into
  * out/debug/\<host\_compiler>/tests
  * e\.g\. out/debug/linux\_x64\_clang/tests
* Individual binaries\, can be run through regular tools:
  * gdb
  * valgrind
  * Your favorite tool that you tell everyone about\.

# Integration Testing

* Two common mechanisms for integration testing in the SDK
  * YAML \( _[https://project\-chip\.github\.io/connectedhomeip\-doc/testing/yaml\.html](https://project-chip.github.io/connectedhomeip-doc/testing/yaml.html)_ \)
  * python \( _[https://project\-chip\.github\.io/connectedhomeip\-doc/testing/python\.html](https://project-chip.github.io/connectedhomeip-doc/testing/python.html)_ \)
* Current cert and integration tests cover standards functionality against specific targets  \(e\.g\. examples/products\)
* Challenges:
  * Testing difficult physical conditions \(e\.g\. faults\) in cert tests\.
  * Simulating manual actions so tests can run in CI\.
  * Forcing error conditions during tests\.

# TestEventTriggers

* Used to test interactions on the DUT that are difficult to perform during cert testing \(ex\. triggering a smoke alarm\)
  * Use sparingly\!
* Uses a command in the General Diagnostics cluster
* Takes a “test key” and a “trigger code” to request that a device at cert perform a specific action
  * Currently most devices use a default key\, but it can be overridden by a specific device if required
* Documentation of TestEventTrigger IDs \- forthcoming from test plans TT
* __Needs to be turned off outside of certification tests__

* derive from TestEventTriggerHandler
  * implement HandleEventTrigger
* register with TestEventTriggerDelegate::AddHandler
* good example: src/app/clusters/energy\-evse\-server/EnergyEvseTestEventTriggerHandler\.h

# NamedPipes

* Trigger for actions on the CI\, for actions that are normally done manually at certification\. Linux\-only\.
* See examples/platform/linux/NamedPipeCommands\.h
  * OnEventCommandReceived\(const char \* json\)
* For a good example\, see examples/air\-quality\-sensor\-app/linux/AirQualitySensorAppAttrUpdateDelegate\.cpp
* Python tests can have the process ID for the pipe passed\-in so these can be used in testing \(\-\-app\-pid\)

# Fault Injection

* Used to inject conditional code paths at runtime\, e\.g\. errors
* Fault injection framework supports setting an ID for a code\. path\, and the number of times it should be skipped or run\.
* Good for testing client code’s tolerance of errors\.
* Compile\-time option\, so this can be turned off for products
  * chip\_with\_nlfaultinjection
* Managed through a fault injection cluster\, operated via a secondary non\-DUT controller\.
  * ex\. _[https://github\.com/CHIP\-Specifications/chip\-test\-plans/blob/master/src/interactiondatamodel\.adoc\#tc\-idm\-1\-3\-batched\-commands\-invoke\-request\-action\-from\-dut\-to\-th\-dut\_client](https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#tc-idm-1-3-batched-commands-invoke-request-action-from-dut-to-th-dut_client)_

* Server side:
* Multiple fault injection managers exist\.
  * Use the one from lib/support/CHIPFaultInjection\.h
  * Add new IDs to the enum there
* Uses CHIP\_FAULT\_INJECT\(aFaultID\, aStatements\)
  * See next slide\.

# Fault Injection example

<span style="color:#4EC9B0">CHIP\_ERROR</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#4EC9B0">CASEServer</span>  <span style="color:#CCCCCC">::</span>  <span style="color:#DCDCAA">OnMessageReceived</span>  <span style="color:#CCCCCC">\(</span>  <span style="color:#4EC9B0">Messaging</span>  <span style="color:#CCCCCC">::</span>  <span style="color:#4EC9B0">ExchangeContext</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#569CD6">\*</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#9CDCFE">ec</span>  <span style="color:#CCCCCC">\, </span>

<span style="color:#569CD6">   const</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#4EC9B0">PayloadHeader</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#569CD6">&</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#9CDCFE">payloadHeader</span>  <span style="color:#CCCCCC">\,</span>

<span style="color:#CCCCCC">                                        </span>  <span style="color:#4EC9B0">System</span>  <span style="color:#CCCCCC">::</span>  <span style="color:#4EC9B0">PacketBufferHandle</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#569CD6">&&</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#9CDCFE">payload</span>  <span style="color:#CCCCCC">\)</span>

<span style="color:#CCCCCC">\{</span>

<span style="color:#CCCCCC">   </span>  <span style="color:#DCDCAA">MATTER\_TRACE\_SCOPE</span>  <span style="color:#CCCCCC">\(</span>  <span style="color:#CE9178">"OnMessageReceived"</span>  <span style="color:#CCCCCC">\, </span>  <span style="color:#CE9178">"CASEServer"</span>  <span style="color:#CCCCCC">\);</span>

<span style="color:#CCCCCC">   </span>  <span style="color:#569CD6">bool</span>  <span style="color:#CCCCCC"> busy </span>  <span style="color:#D4D4D4">=</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#DCDCAA">GetSession</span>  <span style="color:#CCCCCC">\(\)\.</span>  <span style="color:#DCDCAA">GetState</span>  <span style="color:#CCCCCC">\(\) </span>  <span style="color:#D4D4D4">\!=</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#4EC9B0">CASESession</span>  <span style="color:#CCCCCC">::</span>  <span style="color:#4EC9B0">State</span>  <span style="color:#CCCCCC">::kInitialized;</span>

<span style="color:#CCCCCC">   </span>  <span style="color:#DCDCAA">CHIP\_FAULT\_INJECT</span>  <span style="color:#CCCCCC">\(</span>  <span style="color:#4EC9B0">FaultInjection</span>  <span style="color:#CCCCCC">::kFault\_CASEServerBusy\, busy </span>  <span style="color:#D4D4D4">=</span>  <span style="color:#CCCCCC"> </span>  <span style="color:#569CD6">true</span>  <span style="color:#CCCCCC">\);</span>

<span style="color:#CCCCCC">   </span>  <span style="color:#C586C0">if</span>  <span style="color:#CCCCCC"> \(busy\)</span>

<span style="color:#CCCCCC">   \{</span>

<span style="color:#CCCCCC">…</span>

# Fault Injection cluster

* Fault injection cluster is available in the SDK\, vendor\-specific \(0xFFF1FC06\)
* Example apps can be compiled with this cluster for ex\. certifying controllers against a device demonstrating specific behavior
* Command: FailAtFault
  * Type: FaultType \- use FaultType::kChipFault \(0x03\)
  * Id: int32u \- match the ID you set up for your fault
  * NumCallsToSkip: int32u \- number of times to run normally
  * NumCallsToFail: int32u \- number of times to hit the fault injection condition after NumCallsToSkip
  * TakeMutex: bool \- controls access to the fault injection manager for multi\-threaded systems\. False is fine\.

---

There’s also a fail randomly comand, but … dont.

# Whole stack testing

* For all of these things\, it’s important to inject the errors at the point where they are running through the MOST code that they can
  * Handler in the driver rather than the command handler
* In the proposed ClusterLogic pattern\, this means injection errors as close as possible to the driver layer\, rather than catching errors in the server and returning what is expected

---

We want real tests, not just test theatre

# 1.4 Expectations

* Certification tests are REQUIRED to be automated
  * no new manual tests will be accepted
  * semi\-automated tests only for tests that are verifying manual actions are reflected on device \(ex\, push x button on device\, ensure it is properly reflected on the device\)
  * tests with manual steps MUST include CI integration on example apps that can be used to simulate manual action
* New code landing in the SDK\, and changes to existing code should come with unit tests\, even if this means writing new tests for existing code

# Q&A

Thursday 3/21 Stamford Ballroom 9:30–10:30 AM

Matter TSG \- Testable SDK Code \[MA\-TSG\-02\-PM3\-006\]

# Attendance Check-In

Scan this QR Code in your Canapii app to check\-in\. Your session attendance must be tracked for voting rights purpose\.

Open the Canapii

app & click Scan QR

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session7.png)

![](img/%5BExternal%5D%20Singapore%20MM%20testing%20session8.png)

