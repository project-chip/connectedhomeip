import Matter
import XCTest

// This more or less parallels the "no delegate" case in MTRPairingTests, but uses the "normal"
// all-clusters-app, since it does not do any of the "interesting" VID/PID notification so far.  If
// it ever starts needing to do that, we should figure out a way to use MTRTestCase+ServerAppRunner from
// here.

struct PairingConstants {
    static let localPort = 5541
    static let vendorID = 0xFFF1
    static let onboardingPayload = "MT:-24J0AFN00KA0648G00"
    static let deviceID = 0x12344321
    static let timeoutInSeconds : UInt16 = 3
    static let pairingTimeoutInSeconds : UInt16 = 60
}

class MTRSwiftPairingTestControllerDelegate : NSObject, MTRDeviceControllerDelegate {
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
            try controller.commissionNode(withID: PairingConstants.deviceID as NSNumber, commissioningParams: MTRCommissioningParameters())
        } catch {
            XCTFail("Could not start commissioning of node: \(error)")
        }

        // Keep waiting for commissioningComplete
    }

    func controller(_ controller: MTRDeviceController, commissioningComplete error: Error?, nodeID: NSNumber?) {
        XCTAssertNil(error)
        XCTAssertEqual(nodeID, PairingConstants.deviceID as NSNumber)
        expectation.fulfill()
    }
}

class MTRSwiftPairingTests : XCTestCase {
    func test001_BasicPairing() {
        let factory = MTRDeviceControllerFactory.sharedInstance()

        let storage = MTRTestStorage()
        let factoryParams = MTRDeviceControllerFactoryParams(storage: storage)
        factoryParams.port = PairingConstants.localPort as NSNumber

        do {
            try factory.start(factoryParams)
        } catch {
            XCTFail("Could not start controller factory: \(error)")
            return
        }
        XCTAssertTrue(factory.isRunning)

        let testKeys = MTRTestKeys()

        let params = MTRDeviceControllerStartupParams(ipk: testKeys.ipk, fabricID: 1, nocSigner: testKeys)
        params.vendorID = PairingConstants.vendorID as NSNumber

        let controller: MTRDeviceController
        do {
            controller = try factory.createController(onNewFabric: params)
        } catch {
            XCTFail("Could not create controller: \(error)")
            return
        }
        XCTAssertTrue(controller.isRunning)

        let expectation = expectation(description: "Commissioning Complete")

        let controllerDelegate = MTRSwiftPairingTestControllerDelegate(withExpectation: expectation)
        let serialQueue = DispatchQueue(label: "com.chip.pairing")

        controller.setDeviceControllerDelegate(controllerDelegate, queue: serialQueue)

        let payload : MTRSetupPayload
        do {
            payload = try MTRSetupPayload(onboardingPayload: PairingConstants.onboardingPayload)
        } catch {
            XCTFail("Could not parse setup payload: \(error)")
            return
        }

        do {
            try controller.setupCommissioningSession(with: payload, newNodeID: PairingConstants.deviceID as NSNumber)
        } catch {
            XCTFail("Could not start setting up PASE session: \(error)")
            return
        }

        wait(for: [expectation], timeout: TimeInterval(PairingConstants.pairingTimeoutInSeconds))

        ResetCommissionee(MTRBaseDevice(nodeID: PairingConstants.deviceID as NSNumber, controller: controller), DispatchQueue.main, self, PairingConstants.timeoutInSeconds)

        controller.shutdown()
        XCTAssertFalse(controller.isRunning)

        factory.stop()
        XCTAssertFalse(factory.isRunning)
    }
}
