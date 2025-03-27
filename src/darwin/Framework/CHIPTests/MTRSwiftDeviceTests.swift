import Matter
import XCTest

// This should eventually grow into a Swift copy of MTRDeviceTests

// Fixture: chip-all-clusters-app --KVS "$(mktemp -t chip-test-kvs)" --interface-id -1

struct DeviceConstants {
    static let testVendorID = 0xFFF1
    static let onboardingPayload = "MT:-24J0AFN00KA0648G00"
    static let deviceID = 0x12344321
    static let timeoutInSeconds : Double = 3
    static let pairingTimeoutInSeconds : Double = 30
}

var sConnectedDevice: MTRBaseDevice? = nil

var sController: MTRDeviceController? = nil

var sTestKeys: MTRTestKeys? = nil

class MTRSwiftDeviceTestControllerDelegate : NSObject, MTRDeviceControllerDelegate {
    let expectation: XCTestExpectation

    init(withExpectation providedExpectation: XCTestExpectation) {
        expectation = providedExpectation
    }

    func controller(_ controller: MTRDeviceController, statusUpdate status: MTRCommissioningStatus) {
        XCTAssertNotEqual(status, MTRCommissioningStatus.failed)
    }

    func controller(_ controller: MTRDeviceController, commissioningSessionEstablishmentDone error: Error?) {
        XCTAssertNil(error)

        do {
            try controller.commissionNode(withID: DeviceConstants.deviceID as NSNumber, commissioningParams: MTRCommissioningParameters())
        } catch {
            XCTFail("Could not start commissioning of node: \(error)")
        }

        // Keep waiting for commissioningComplete
    }

    func controller(_ controller: MTRDeviceController, commissioningComplete error: Error?, nodeID: NSNumber?) {
        XCTAssertNil(error)
        XCTAssertEqual(nodeID, DeviceConstants.deviceID as NSNumber)
        sConnectedDevice = MTRBaseDevice(nodeID: nodeID!, controller: controller)
        expectation.fulfill()
    }
}

typealias MTRDeviceTestDelegateDataHandler = ([[ String: Any ]]) -> Void

class MTRSwiftDeviceTestDelegate : NSObject, MTRDeviceDelegate {
    var onReachable : () -> Void
    var onNotReachable : (() -> Void)? = nil
    var onAttributeDataReceived : MTRDeviceTestDelegateDataHandler? = nil
    var onEventDataReceived : MTRDeviceTestDelegateDataHandler? = nil
    var onReportEnd : (() -> Void)? = nil

    init(withReachableHandler handler : @escaping () -> Void) {
        onReachable = handler
    }

    func device(_ device: MTRDevice, stateChanged state : MTRDeviceState) {
        if (state == MTRDeviceState.reachable) {
           onReachable()
        } else {
           onNotReachable?()
        }
    }

    func device(_ device : MTRDevice, receivedAttributeReport attributeReport : [[ String: Any ]])
    {
        onAttributeDataReceived?(attributeReport)
    }

    func device(_ device : MTRDevice, receivedEventReport eventReport : [[ String : Any ]])
    {
        onEventDataReceived?(eventReport)
    }

    @objc func unitTestReportEnd(forDevice : MTRDevice)
    {
        onReportEnd?()
    }

    @objc func unitTestMaxIntervalOverrideForSubscription(_ device : MTRDevice) -> NSNumber
    {
      // Make sure our subscriptions time out in finite time.
      return 2; // seconds
    }

    @objc func unitTestSuppressTimeBasedReachabilityChanges(_ device : MTRDevice) -> Bool
    {
      // Allowing time-based reachability changes just makes the tests
      // non-deterministic and can lead to random failures.  Suppress them
      // unconditionally for now.  If we ever add tests that try to exercise that
      // codepath, we can make this configurable.
      return true;
    }
}

class MTRSwiftDeviceTests : XCTestCase {
    static override func setUp()
    {
        super.setUp()

        let factory = MTRDeviceControllerFactory.sharedInstance()
        
        let storage = MTRTestStorage()
        let factoryParams = MTRDeviceControllerFactoryParams(storage: storage)
        
        do {
            try factory.start(factoryParams)
        } catch {
            XCTFail("Count not start controller factory: \(error)")
        }
        XCTAssertTrue(factory.isRunning)
        
        let testKeys = MTRTestKeys()
        sTestKeys = testKeys
        
        // Needs to match what startControllerOnExistingFabric calls elsewhere in
        // this file do.
        let params = MTRDeviceControllerStartupParams(ipk: testKeys.ipk, fabricID: 1, nocSigner:testKeys)
        params.vendorID = DeviceConstants.testVendorID as NSNumber
        
        let controller : MTRDeviceController
        do {
            controller = try factory.createController(onNewFabric: params)
        } catch {
            XCTFail("Could not create controller: \(error)")
            return
        }
        XCTAssertTrue(controller.isRunning)
        
        sController = controller
        
        let expectation = XCTestExpectation(description : "Commissioning Complete")

        let controllerDelegate = MTRSwiftDeviceTestControllerDelegate(withExpectation: expectation)
        let serialQueue = DispatchQueue(label: "com.chip.device_controller_delegate")
        
        controller.setDeviceControllerDelegate(controllerDelegate, queue: serialQueue)
        
        let payload : MTRSetupPayload
        do {
            payload = try MTRSetupPayload(onboardingPayload: DeviceConstants.onboardingPayload)
        } catch {
            XCTFail("Could not parse setup payload: \(error)")
            return
        }
        
        do {
            try controller.setupCommissioningSession(with:payload, newNodeID: DeviceConstants.deviceID as NSNumber)
        } catch {
            XCTFail("Could not start setting up PASE session: \(error)")
            return        }

        XCTAssertEqual(XCTWaiter.wait(for: [expectation], timeout: DeviceConstants.pairingTimeoutInSeconds), XCTWaiter.Result.completed)
    }
    
    static override func tearDown() {
        XCTAssertNotNil(sConnectedDevice)
        ResetCommissionee(sConnectedDevice!, DispatchQueue.main, nil, UInt16(DeviceConstants.timeoutInSeconds))
        let controller = sController
        XCTAssertNotNil(controller)
        controller!.shutdown()
        XCTAssertFalse(controller!.isRunning)
        
        MTRDeviceControllerFactory.sharedInstance().stop()

        super.tearDown()
    }

    override func setUp()
    {
        super.setUp()
        self.continueAfterFailure = false
    }

    func test017_TestMTRDeviceBasics()
    {
        let device = MTRDevice(nodeID: DeviceConstants.deviceID as NSNumber, controller:sController!)
        let queue = DispatchQueue.main
        
        // Given reachable state becomes true before underlying OnSubscriptionEstablished callback, this expectation is necessary but
        // not sufficient as a mark to the end of reports
        let subscriptionExpectation = expectation(description: "Subscription has been set up")
        
        let delegate = MTRSwiftDeviceTestDelegate(withReachableHandler: { () -> Void in
            subscriptionExpectation.fulfill()
        })
        
        var attributeReportsReceived : Int = 0
        delegate.onAttributeDataReceived = { (data: [[ String: Any ]]) -> Void in
            attributeReportsReceived += data.count
        }
        
        // This is dependent on current implementation that priming reports send attributes and events in that order, and also that
        // events in this test would fit in one report. So receiving events would mean all attributes and events have been received, and
        // can satisfy the test below.
        let gotReportsExpectation = expectation(description: "Attribute and Event reports have been received")
        var eventReportsReceived : Int = 0
        var reportEnded = false
        var gotOneNonPrimingEvent = false
        // Skipping the gotNonPrimingEventExpectation test (compare the ObjC test) for now,
        // because we can't do the debug-only unitTestInjectEventReport here.
        delegate.onEventDataReceived = { (eventReport: [[ String: Any ]]) -> Void in
            eventReportsReceived += eventReport.count
            
            for eventDict in eventReport {
                let eventTimeTypeNumber = eventDict[MTREventTimeTypeKey] as! NSNumber?
                XCTAssertNotNil(eventTimeTypeNumber)
                let eventTimeType = MTREventTimeType(rawValue: eventTimeTypeNumber!.uintValue)
                XCTAssert((eventTimeType == MTREventTimeType.systemUpTime) || (eventTimeType == MTREventTimeType.timestampDate))
                if (eventTimeType == MTREventTimeType.systemUpTime) {
                    XCTAssertNotNil(eventDict[MTREventSystemUpTimeKey])
                    XCTAssertNotNil(device.estimatedStartTime)
                } else if (eventTimeType == MTREventTimeType.timestampDate) {
                    XCTAssertNotNil(eventDict[MTREventTimestampDateKey])
                }
                
                if (!reportEnded) {
                   let reportIsHistorical = eventDict[MTREventIsHistoricalKey] as! NSNumber?
                   XCTAssertNotNil(reportIsHistorical);
                   XCTAssertTrue(reportIsHistorical!.boolValue);
                } else {
                   if (!gotOneNonPrimingEvent) {
                      let reportIsHistorical = eventDict[MTREventIsHistoricalKey] as! NSNumber?
                      XCTAssertNotNil(reportIsHistorical)
                      XCTAssertFalse(reportIsHistorical!.boolValue)
                      gotOneNonPrimingEvent = true
                   }
                }
            }
        }
        delegate.onReportEnd = { () -> Void in
            reportEnded = true
            gotReportsExpectation.fulfill()
        }
        
        device.setDelegate(_: delegate, queue:queue)
        
        // Test batching and duplicate check
        //   - Read 13 different attributes in a row, expect that the 1st to go out by itself, the next 9 batch, and then the 3 after
        //     are correctly queued in one batch
        //   - Then read 3 duplicates and expect them to be filtered
        //   - Note that these tests can only be verified via logs
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 0, params: nil)
        
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 1, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 2, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 3, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 4, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 5, params: nil)
        
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 6, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.colorControlID.rawValue), attributeID: 7, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 0, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 1, params: nil)
        
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 2, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 3, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 4, params: nil)
        
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 4, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 4, params: nil)
        device.readAttribute(withEndpointID: 1, clusterID: NSNumber(value: MTRClusterIDType.levelControlID.rawValue), attributeID: 4, params: nil)
        
        wait(for: [ subscriptionExpectation, gotReportsExpectation ], timeout:60)
        
        delegate.onReportEnd = nil
        
        XCTAssertNotEqual(attributeReportsReceived, 0)
        XCTAssertNotEqual(eventReportsReceived, 0)
        
        // Before resubscribe, first test write failure and expected value effects
        let testEndpointID = 1 as NSNumber
        let testClusterID = 8 as NSNumber
        let testAttributeID = 10000 as NSNumber // choose a nonexistent attribute to cause a failure
        let expectedValueReportedExpectation = expectation(description: "Expected value reported")
        let expectedValueRemovedExpectation =    expectation(description: "Expected value removed")
        delegate.onAttributeDataReceived = { (attributeReport: [[ String: Any ]]) -> Void in
            for attributeDict in attributeReport {
                let attributePath = attributeDict[MTRAttributePathKey] as! MTRAttributePath
                XCTAssertNotNil(attributePath)
                if (attributePath.endpoint == testEndpointID &&
                    attributePath.cluster == testClusterID &&
                    attributePath.attribute == testAttributeID) {
                    let data = attributeDict[MTRDataKey]
                    if (data != nil) {
                        expectedValueReportedExpectation.fulfill()
                    } else {
                        expectedValueRemovedExpectation.fulfill()
                    }
                }
            }
        }
        
        let writeValue = [ "type": "UnsignedInteger", "value": 200 ] as [String: Any]
        device.writeAttribute(withEndpointID: testEndpointID,
                              clusterID: testClusterID,
                              attributeID: testAttributeID,
                              value: writeValue,
                              expectedValueInterval: 20000,
                              timedWriteTimeout: nil)
        
        // expected value interval is 20s but expect it get reverted immediately as the write fails because it's writing to a
        // nonexistent attribute
        wait(for: [ expectedValueReportedExpectation, expectedValueRemovedExpectation ], timeout: 5, enforceOrder: true)
        
        // Test if previous value is reported on a write
        let testOnTimeValue : UInt32 = 10;
        let onTimeWriteSuccess = expectation(description: "OnTime write success");
        let onTimePreviousValue = expectation(description: "OnTime previous value");
        delegate.onAttributeDataReceived = { (data: [[ String: Any ]]) -> Void in
            NSLog("GOT SOME DATA: %@", data)
            for attributeResponseValue in data {
                let path = attributeResponseValue[MTRAttributePathKey] as! MTRAttributePath
                if (path.cluster == (MTRClusterIDType.onOffID.rawValue as NSNumber) &&
                    path.attribute == (MTRAttributeIDType.clusterOnOffAttributeOnTimeID.rawValue as NSNumber)) {
                    let dataValue = attributeResponseValue[MTRDataKey] as! NSDictionary?
                    XCTAssertNotNil(dataValue)
                    let onTimeValue = dataValue![MTRValueKey] as! NSNumber?
                    if (onTimeValue != nil && (onTimeValue!.uint32Value == testOnTimeValue)) {
                        onTimeWriteSuccess.fulfill();
                    }

                    let previousDataValue = attributeResponseValue[MTRPreviousDataKey] as! NSDictionary?
                    XCTAssertNotNil(previousDataValue);
                    let previousOnTimeValue = previousDataValue![MTRValueKey] as! NSNumber?
                    if (previousOnTimeValue != nil) {
                        onTimePreviousValue.fulfill()
                    }
                }
            }
        };
        let writeOnTimeValue = [ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : testOnTimeValue ] as [String: Any]
        device.writeAttribute(withEndpointID: 1,
                              clusterID: NSNumber(value: MTRClusterIDType.onOffID.rawValue),
                              attributeID: NSNumber(value: MTRAttributeIDType.clusterOnOffAttributeOnTimeID.rawValue),
                              value: writeOnTimeValue,
                              expectedValueInterval: 10000,
                              timedWriteTimeout: nil);

        wait(for: [ onTimeWriteSuccess, onTimePreviousValue ], timeout: 10);

        // TODO: Skipping test for cache-clearing for now, because we can't call unitTestClearClusterData here.

        // Test if errors are properly received
        // TODO: We might stop reporting these altogether from MTRDevice, and then
        // this test will need updating.
        let readThroughForUnknownAttributesParams = MTRReadParams()
        readThroughForUnknownAttributesParams.shouldAssumeUnknownAttributesReportable = false;
        let attributeReportErrorExpectation = expectation(description: "Attribute read error")
        delegate.onAttributeDataReceived = { (data: [[ String: Any ]]) -> Void in
            for attributeResponseValue in data {
                if (attributeResponseValue[MTRErrorKey] != nil) {
                    attributeReportErrorExpectation.fulfill()
                }
            }
        }
        // use the nonexistent attribute and expect read error
        device.readAttribute(withEndpointID: testEndpointID, clusterID: testClusterID, attributeID: testAttributeID, params: readThroughForUnknownAttributesParams)
        wait(for: [ attributeReportErrorExpectation ], timeout: 10)
        
        // Resubscription test setup
        let subscriptionDroppedExpectation = expectation(description: "Subscription has dropped")
        delegate.onNotReachable = { () -> Void in
            subscriptionDroppedExpectation.fulfill()
        };
        let resubscriptionReachableExpectation = expectation(description: "Resubscription has become reachable")
        delegate.onReachable = { () -> Void in
            resubscriptionReachableExpectation.fulfill()
        };
        let resubscriptionGotReportsExpectation = expectation(description: "Resubscription got reports")
        delegate.onReportEnd = { () -> Void in
            resubscriptionGotReportsExpectation.fulfill()
        }
        
        // reset the onAttributeDataReceived to validate the following resubscribe test
        attributeReportsReceived = 0;
        eventReportsReceived = 0;
        delegate.onAttributeDataReceived = { (data: [[ String: Any ]]) -> Void in
            attributeReportsReceived += data.count;
        };
        
        delegate.onEventDataReceived = { (eventReport: [[ String: Any ]]) -> Void in
            eventReportsReceived += eventReport.count;
        };
        
        // Now trigger another subscription which will cause ours to drop; we should re-subscribe after that.
        let baseDevice = sConnectedDevice
        let params = MTRSubscribeParams(minInterval: 1, maxInterval: 2)
        params.shouldResubscribeAutomatically = false;
        params.shouldReplaceExistingSubscriptions = true;
        // Create second subscription which will cancel the first subscription.  We
        // can use a non-existent path here to cut down on the work that gets done.
        baseDevice?.subscribeToAttributes(withEndpointID: 10000,
                                          clusterID: 6,
                                          attributeID: 0,
                                          params: params,
                                          queue: queue,
                                          reportHandler: { (_: [[String : Any]]?, _: Error?) -> Void in
        })
        
        wait(for: [ subscriptionDroppedExpectation ], timeout:60)
        
        // Check that device resets start time on subscription drop
        XCTAssertNil(device.estimatedStartTime)
        
        wait(for: [ resubscriptionReachableExpectation, resubscriptionGotReportsExpectation ], timeout:60)
        
        // Now make sure we ignore later tests.
        device.remove(_: delegate)
        
        // Make sure we got no updated reports (because we had a cluster state cache
        // with data versions) during the resubscribe.
//        XCTAssertEqual(attributeReportsReceived, 0);
        XCTAssertEqual(eventReportsReceived, 0);
    }
    
    func test018_SubscriptionErrorWhenNotResubscribing()
    {
        let device = sConnectedDevice!
        let queue = DispatchQueue.main
        
        let firstSubscribeExpectation = expectation(description: "First subscription complete")
        let errorExpectation = expectation(description: "First subscription errored out")
        
        // Subscribe
        let params = MTRSubscribeParams(minInterval: 1, maxInterval: 10)
        params.shouldResubscribeAutomatically = false
        params.shouldReplaceExistingSubscriptions = true // Not strictly needed, but checking that doing this does not
        // affect this subscription erroring out correctly.
        var subscriptionEstablished = false
        device.subscribeToAttributes(withEndpointID: 1,
                                     clusterID: 6,
                                     attributeID: 0,
                                     params: params,
                                     queue: queue,
                                     reportHandler: { values, error in
            if (subscriptionEstablished) {
                // We should only get an error here.
                XCTAssertNil(values)
                XCTAssertNotNil(error)
                errorExpectation.fulfill()
            } else {
                XCTAssertNotNil(values)
                XCTAssertNil(error)
            }
        },
                                     subscriptionEstablished: {
            NSLog("subscribe attribute: OnOff established")
            XCTAssertFalse(subscriptionEstablished)
            subscriptionEstablished = true
            firstSubscribeExpectation.fulfill()
        })
        
        // Wait till establishment
        wait(for: [ firstSubscribeExpectation ], timeout: DeviceConstants.timeoutInSeconds)
        
        // Create second subscription which will cancel the first subscription.  We
        // can use a non-existent path here to cut down on the work that gets done.
        params.shouldReplaceExistingSubscriptions = true
        
        device.subscribeToAttributes(withEndpointID: 10000,
                                     clusterID: 6,
                                     attributeID: 0,
                                     params: params,
                                     queue: queue,
                                     reportHandler: { _, _ in
        })
        wait(for: [ errorExpectation ], timeout: 60)
    }
    
    func test019_MTRDeviceMultipleCommands() async
    {
        let device = MTRDevice(nodeID: DeviceConstants.deviceID as NSNumber, controller:sController!)
        let queue = DispatchQueue.main
        
        let opcredsCluster = MTRClusterOperationalCredentials(device: device, endpointID: 0, queue: queue)!
        let onOffCluster = MTRClusterOnOff(device: device, endpointID: 1, queue: queue)!
        let badOnOffCluster = MTRClusterOnOff(device: device, endpointID: 0, queue: queue)!
        
        // Ensure our existing fabric label is not "Test".  This uses a "base"
        // cluster to ensure read-through to the other side.
        let baseOpCredsCluster = MTRBaseClusterOperationalCredentials(device: sConnectedDevice!, endpointID: 0, queue: queue)!
        var fabricList = try! await baseOpCredsCluster.readAttributeFabrics(with: nil)
        XCTAssertEqual(fabricList.count, 1)
        var entry = fabricList[0] as! MTROperationalCredentialsClusterFabricDescriptorStruct
        XCTAssertNotEqual(entry.label, "Test")
        
        let currentFabricIndex = try! await baseOpCredsCluster.readAttributeCurrentFabricIndex()
        
        // NOTE: The command invocations do not use "await", because we actually want
        // to do them all sort of in parallel, or at least queue them all on the MTRDevice
        // before we get a chance to finish the first one.
        let onExpectation = expectation(description: "On command executed")
        onOffCluster.on(withExpectedValues: nil, expectedValueInterval: nil) { error in
            XCTAssertNil(error)
            onExpectation.fulfill()
        }
        
        let offFailedExpectation = expectation(description: "Off command failed")
        badOnOffCluster.off(withExpectedValues: nil, expectedValueInterval: nil) { error in
            XCTAssertNotNil(error)
            offFailedExpectation.fulfill()
        }
        
        let updateLabelExpectation = expectation(description: "Fabric label updated")
        let params = MTROperationalCredentialsClusterUpdateFabricLabelParams()
        params.label = "Test"
        opcredsCluster.updateFabricLabel(with: params, expectedValues: nil, expectedValueInterval: nil) { data, error in
            XCTAssertNil(error)
            XCTAssertNotNil(data)
            XCTAssertEqual(data?.statusCode, 0)
            XCTAssertEqual(data?.fabricIndex, currentFabricIndex)
            updateLabelExpectation.fulfill()
        }
        
        let offExpectation = expectation(description: "Off command executed")
        // Send this one via MTRDevice, to test that codepath
        device.invokeCommand(withEndpointID: 1,
                             clusterID: NSNumber(value: MTRClusterIDType.onOffID.rawValue),
                             commandID: NSNumber(value: MTRCommandIDType.clusterOnOffCommandOffID.rawValue),
                             commandFields: nil,
                             expectedValues: nil,
                             expectedValueInterval: nil,
                             queue: queue) { data, error in
            XCTAssertNil(error)
            offExpectation.fulfill()
        }
        
        let onFailedExpectation = expectation(description: "On command failed")
        badOnOffCluster.on(withExpectedValues: nil, expectedValueInterval: nil) { error in
            XCTAssertNotNil(error)
            onFailedExpectation.fulfill()
        }
        
        let updateLabelFailedExpectation = expectation(description: "Fabric label update failed")
        params.label = "12345678901234567890123445678901234567890" // Too long
        opcredsCluster.updateFabricLabel(with: params,
                                         expectedValues: nil,
                                         expectedValueInterval: nil) { data, error in
            XCTAssertNotNil(error)
            XCTAssertNil(data)
            updateLabelFailedExpectation.fulfill()
        }
        
        wait(for: [ onExpectation,
                    offFailedExpectation,
                    updateLabelExpectation,
                    offExpectation,
                    onFailedExpectation,
                    updateLabelFailedExpectation ],
             timeout: 60,
             enforceOrder: true)
        
        // Now make sure our fabric label got updated.
        fabricList = try! await baseOpCredsCluster.readAttributeFabrics(with: nil)
        XCTAssertNotNil(fabricList)
        XCTAssertEqual(fabricList.count, 1)
        entry = fabricList[0] as! MTROperationalCredentialsClusterFabricDescriptorStruct
        XCTAssertEqual(entry.label, "Test")
    }
    
    // Note: test027_AttestationChallenge is not implementable in Swift so far,
    // because the attestationChallenge property is internal-only
    
    func test030_DeviceAndClusterProperties() {
        let queue = DispatchQueue.main

        let device = MTRDevice(nodeID: DeviceConstants.deviceID as NSNumber, controller:sController!)
        XCTAssertEqual(device.deviceController, sController)
        XCTAssertEqual(device.__nodeID, DeviceConstants.deviceID as NSNumber)
        
        let cluster = MTRClusterOperationalCredentials(device: device, endpointID: 0, queue: queue)!
        XCTAssertEqual(cluster.device, device)
        XCTAssertEqual(cluster.__endpointID, 0 as NSNumber)
    }
}
