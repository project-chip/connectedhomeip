import Matter
import XCTest

// This more or less parallels the "no delegate" case in MTRPairingTests

struct Constants {
    static let localPort = 5541
    static let vendorID = 0xFFF1
    static let onboardingPayload = "MT:Y.K90SO527JA0648G00"
    static let deviceID = 0x12344321
    static let timeoutInSeconds : UInt16 = 3
}

class MTRSwiftPairingTestControllerDelegate : NSObject, MTRDeviceControllerDelegate {
    let expectation: XCTestExpectation

    init(withExpectation providedExpectation: XCTestExpectation) {
        expectation = providedExpectation
    }

    @available(macOS, introduced: 13.3)
    func controller(_ controller: MTRDeviceController, statusUpdate status: MTRCommissioningStatus) {
        XCTAssertNotEqual(status, MTRCommissioningStatus.failed)
    }

    @available(macOS, introduced: 13.3)
    func controller(_ controller: MTRDeviceController, commissioningSessionEstablishmentDone error: Error?) {
        XCTAssertNil(error)

        do {
            try controller.commissionNode(withID: Constants.deviceID as NSNumber, commissioningParams: MTRCommissioningParameters())
        } catch {
            XCTFail("Could not start commissioning of node")
        }

        // Keep waiting for commissioningComplete
    }

    func controller(_ controller: MTRDeviceController, commissioningComplete error: Error?, nodeID: NSNumber?) {
        XCTAssertNil(error)
        XCTAssertEqual(nodeID, Constants.deviceID as NSNumber)
        expectation.fulfill()
    }
}

class MTRSwiftPairingTests : XCTestCase {
    @available(macOS, introduced: 13.3)
    func test001_BasicPairing() throws {
        let factory = MTRDeviceControllerFactory.sharedInstance()
        XCTAssertNotNil(factory)

        let storage = MTRTestStorage()
        let factoryParams = MTRDeviceControllerFactoryParams(storage: storage)
        factoryParams.port = Constants.localPort as NSNumber

        try factory.start(factoryParams)
        XCTAssertTrue(factory.isRunning)

        let testKeys = MTRTestKeys()

        let params = MTRDeviceControllerStartupParams(ipk: testKeys.ipk, fabricID: 1, nocSigner: testKeys)
        params.vendorID = Constants.vendorID as NSNumber;

        let controller = try factory.createController(onNewFabric: params)
        XCTAssertTrue(factory.isRunning)

        let expectation = expectation(description: "Commissioning Complete")

        let controllerDelegate = MTRSwiftPairingTestControllerDelegate(withExpectation: expectation)
        let serialQueue = DispatchQueue(label: "com.chip.pairing")

        controller.setDeviceControllerDelegate(controllerDelegate, queue: serialQueue)

        let payload = try MTRSetupPayload(onboardingPayload: Constants.onboardingPayload)

        try controller.setupCommissioningSession(with: payload, newNodeID: Constants.deviceID as NSNumber)

        wait(for: [expectation], timeout: TimeInterval(Constants.timeoutInSeconds))

        ResetCommissionee(MTRBaseDevice(nodeID: Constants.deviceID as NSNumber, controller: controller), DispatchQueue.main, self, Constants.timeoutInSeconds)

        controller.shutdown()
        XCTAssertFalse(controller.isRunning)

        factory.stop()
        XCTAssertFalse(factory.isRunning)
    }
}
