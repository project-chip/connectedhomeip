# Matter TV Casting Darwin (iOS/macOS) App Example

This is a Matter TV Casting app for iOS and macOS that can be used to cast
content to a TV. This app discovers TVs on the local network that act as
commissioners, lets the user select one, sends the TV a User Directed
Commissioning request, enters commissioning mode, advertises itself as a
Commissionable Node and gets commissioned. Then it allows the user to send
Matter commands to the TV.

Refer to the
[Matter Casting APIs documentation](https://project-chip.github.io/connectedhomeip-doc/examples/tv-casting-app/APIs.html)
to build the Matter "Casting Client" into your consumer-facing mobile app.

## Requirements

-   Xcode 15.0 or later
-   macOS with Apple Silicon or Intel processor
-   iOS Simulator or physical iOS device for testing

## Building the App

### Using Xcode

1. Open the workspace:

    ```bash
    cd examples/tv-casting-app/darwin
    open TvCastingDarwin.xcworkspace
    ```

2. Select the appropriate scheme:

    - `TvCasting` - Main iOS app
    - `MatterTvCastingBridge` - Framework/library

3. Select your target device (simulator or physical device)

4. Build and run: `Cmd+R`

### Using Command Line

Build the framework:

```bash
cd examples/tv-casting-app/darwin
xcodebuild -workspace TvCastingDarwin.xcworkspace \
  -scheme MatterTvCastingBridge \
  -destination 'platform=iOS Simulator,name=iPhone 16' \
  build
```

## Running Unit Tests

The Darwin TV Casting app includes unit tests for the SendUDC functionality and
other features.

### Prerequisites

The test files have been created but require one-time setup in Xcode to add them
to the project:

1. **Open the project in Xcode**:

    ```bash
    cd examples/tv-casting-app/darwin
    open MatterTvCastingBridge/MatterTvCastingBridge.xcodeproj
    ```

2. **Add a test target** (if not already present):

    - File → New → Target
    - Choose "Unit Testing Bundle" under iOS
    - Name it `MatterTvCastingBridgeTests`
    - Set the target to be tested to `MatterTvCastingBridge`
    - Click Finish

3. **Add the test file to the test target**:

    - In the Project Navigator, right-click on `MatterTvCastingBridgeTests`
      folder
    - Choose "Add Files to MatterTvCastingBridge..."
    - Navigate to `MatterTvCastingBridgeTests/MCCastingPlayerSendUDCTests.m`
    - Ensure it's added to the `MatterTvCastingBridgeTests` target
    - Click Add

4. **Add OCMock dependency**:

    - Select the project in Project Navigator
    - Select the `MatterTvCastingBridgeTests` target
    - Go to "Build Phases" → "Link Binary With Libraries"
    - Add OCMock framework (install via CocoaPods or Swift Package Manager if
      needed)

### Running Tests in Xcode

Once the test target is configured:

1. Open the project in Xcode
2. Press `Cmd+U` to run all tests
3. Or click the diamond icon next to individual test methods to run specific
   tests
4. Or use the Test Navigator (`Cmd+6`) to browse and run tests

### Running Tests from Command Line

After the test target is properly configured in Xcode:

```bash
cd examples/tv-casting-app/darwin
xcodebuild test \
  -project MatterTvCastingBridge/MatterTvCastingBridge.xcodeproj \
  -scheme MatterTvCastingBridge \
  -destination 'platform=iOS Simulator,name=iPhone 16'
```

### Test Coverage

The SendUDC test suite (`MCCastingPlayerSendUDCTests.m`) includes 10 test cases
covering:

1. Basic sendUDCWithCallbacks invocation
2. NoPasscode flag handling for app detection
3. CancelPasscode flag for ending UDC sessions
4. InstanceName management with UUID-based identifiers
5. MCTargetAppInfo handling with vendor/product IDs
6. CommissionerDeclarationCallback registration
7. Complete app detection workflow simulation
8. Multiple target apps support
9. Error handling with NSError responses
10. Callback validation requirements

### Reviewing Test Code Without Running

If you prefer to review the test structure without setting up the test target:

```bash
cd examples/tv-casting-app/darwin/MatterTvCastingBridge/MatterTvCastingBridgeTests
cat MCCastingPlayerSendUDCTests.m
```

The test file is complete and demonstrates proper usage of the SendUDC API with
OCMock for mocking dependencies.

### Troubleshooting Tests

**Issue**: "Scheme is not currently configured for the test action"

-   **Solution**: The test target needs to be added to the Xcode project (see
    Prerequisites above)

**Issue**: Build errors when running tests

-   **Solution**: Ensure all dependencies are properly linked and the project
    builds successfully before running tests

**Issue**: OCMock not found

-   **Solution**: Install OCMock via CocoaPods or Swift Package Manager and link
    it to the test target

For more details on the test implementation, see the
[test documentation](../tv-casting-common/core/tests/README.md).

## Running the App

This example Matter TV Casting Darwin app can be tested with the
[example Matter tv-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-app)
running on a Raspberry Pi or other supported platform.
