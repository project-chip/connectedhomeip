/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

import Foundation
import os.log

private class CallbackHelper {
    var testCaseName: String;
    var certTestViewModel: CertTestViewModel;
    let logger = Logger(subsystem: "com.matter.casting", category: "CertTestViewModel")

    init(testCaseName: String, certTestViewModel: CertTestViewModel) {
        self.testCaseName = testCaseName
        self.certTestViewModel = certTestViewModel
    }

    func responseCallback(err: MatterError)
    {
        logger.info("CertTestViewModel.responseCallback.\(self.testCaseName) err? \(err)")
        if (err == MATTER_NO_ERROR) {
            certTestViewModel.onTestPassed(testCaseName)
        } else {
            certTestViewModel.onTestFailed(testCaseName)
        }
    }

    func requestSentHandler(err: MatterError)
    {
        logger.info("CertTestViewModel.requestSentHandler.\(self.testCaseName) err? \(err)")
        if (err != MATTER_NO_ERROR) {
            certTestViewModel.onTestFailed(testCaseName)
        }
    }

    func requestSentHandlerError(result: MatterError)
    {
        logger.warning("CertTestViewModel.requestSentHandler.\(self.testCaseName).  Code : \(result.code). Message : \(result.message ?? "")")
        requestSentHandler(err: result)
    }

    func successCallbackString(result: String)
    {
        logger.info("CertTestViewModel.successCallback.\(self.testCaseName) result \(result)")
        certTestViewModel.onTestPassed(testCaseName)
    }

    func successCallbackInteger(result: UInt16)
    {
        logger.info("CertTestViewModel.successCallback.\(self.testCaseName) result \(result)")
        certTestViewModel.onTestPassed(testCaseName)
    }

    func successCallbackNumber(result: NSNumber)
    {
        logger.info("CertTestViewModel.successCallback.\(self.testCaseName) result \(result)")
        certTestViewModel.onTestPassed(testCaseName)
    }

    func failureCallback(result: MatterError)
    {
        logger.info("CertTestViewModel.failureCallback.\(self.testCaseName) failed. Code : \(result.code). Message : \(result.message ?? "")")
        certTestViewModel.onTestFailed(testCaseName)
    }
}

private struct TestContext {
    let castingServerBridge: CastingServerBridge
    let deviceEndpoint: ContentApp
    let contentAppEndpoint: ContentApp
    let parallalizationEnabled: Bool
}

class CertTestViewModel: ObservableObject {
    private let logger = Logger(subsystem: "com.matter.casting", category: "CertTestViewModel")

    private let tests: [(String, (TestContext, CallbackHelper) -> ())] = [
        ("keypadInput_sendKey", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.keypadInput_sendKey(
                context.deviceEndpoint,
                keyCode: 10,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("applicationLauncher_launch", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationLauncher_launch(
                context.deviceEndpoint,
                catalogVendorId: 123,
                applicationId: "exampleid",
                data: nil,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("applicationLauncher_stop", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationLauncher_stop(
                context.deviceEndpoint,
                catalogVendorId: 123, applicationId: "exampleid",
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("applicationLauncher_hide", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationLauncher_hide(
                context.deviceEndpoint,
                catalogVendorId: 123, applicationId: "exampleid",
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("targetNavigator_navigateTarget", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.targetNavigator_navigateTarget(
                context.deviceEndpoint,
                target: 1, data: "",
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("contentLauncher_launchUrl", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.contentLauncher_launchUrl(
                context.contentAppEndpoint,
                contentUrl: "https://dummyurl",
                contentDisplayStr: "Dummy Content",
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("contentLauncher_launchContent", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.contentLauncher_launchContent(
                context.contentAppEndpoint,
                contentSearch: ContentLauncher_ContentSearch(
                    parameterList: [
                        ContentLauncher_Parameter(
                            type: ContentLauncher_ParameterEnum.Video,
                            value: "Dummy Video",
                            externalIDList: [
                                ContentLauncher_AdditionalInfo(
                                    name: "imdb",
                                    value: "dummyId"
                                ),
                            ]
                        ),
                    ]
                ),
                autoPlay: true, data: "",
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_play", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_play(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_next", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_next(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_skipForward", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_skipForward(
                context.contentAppEndpoint,
                deltaPositionMilliseconds: 10000,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_skipBackward", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_skipBackward(
                context.contentAppEndpoint,
                deltaPositionMilliseconds: 10000,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_pause", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_pause(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_stopPlayback", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_stopPlayback(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_seek", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_seek(
                context.contentAppEndpoint,
                position: 10000,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_previous", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_previous(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_rewind", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_rewind(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_fastForward", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_fastForward(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("mediaPlayback_startOver", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_startOver(
                context.contentAppEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("onOff_on", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.onOff_(
                on: context.deviceEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("onOff_off", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.onOff_off(
                context.deviceEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("onOff_toggle", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.onOff_toggle(
                context.deviceEndpoint,
                responseCallback:callbackHelper.responseCallback,
                clientQueue: DispatchQueue.main,
                requestSentHandler:callbackHelper.requestSentHandler
            )
        }),
        ("applicationBasic_readApplicationVersion", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationBasic_readApplicationVersion(
                context.contentAppEndpoint,
                clientQueue: DispatchQueue.main,
                requestSentHandler: callbackHelper.requestSentHandlerError,
                successCallback: callbackHelper.successCallbackString,
                failureCallback: callbackHelper.failureCallback
            )
        }),
        ("applicationBasic_readVendorName", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationBasic_readVendorName(
                context.contentAppEndpoint,
                clientQueue: DispatchQueue.main,
                requestSentHandler: callbackHelper.requestSentHandlerError,
                successCallback: callbackHelper.successCallbackString,
                failureCallback: callbackHelper.failureCallback
            )
        }),
        ("applicationBasic_readApplicationName", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationBasic_readApplicationName(
                context.contentAppEndpoint,
                clientQueue: DispatchQueue.main,
                requestSentHandler: callbackHelper.requestSentHandlerError,
                successCallback: callbackHelper.successCallbackString,
                failureCallback: callbackHelper.failureCallback
            )
        }),
        ("applicationBasic_readVendorID", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationBasic_readVendorID(
                context.contentAppEndpoint,
                clientQueue: DispatchQueue.main,
                requestSentHandler: callbackHelper.requestSentHandlerError,
                successCallback: callbackHelper.successCallbackNumber,
                failureCallback: callbackHelper.failureCallback
            )
        }),
        ("applicationBasic_readProductID", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.applicationBasic_readProductID(
                context.contentAppEndpoint,
                clientQueue: DispatchQueue.main,
                requestSentHandler: callbackHelper.requestSentHandlerError,
                successCallback: callbackHelper.successCallbackInteger,
                failureCallback: callbackHelper.failureCallback
            )
        }),
        ("mediaPlayback_subscribeCurrentState", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.mediaPlayback_subscribeCurrentState(
                context.contentAppEndpoint,
                minInterval: 0, maxInterval: 2,
                clientQueue: DispatchQueue.main,
                requestSentHandler: callbackHelper.requestSentHandlerError,
                successCallback: {(state : MediaPlayback_PlaybackState) -> () in},
                failureCallback: callbackHelper.failureCallback,
                subscriptionEstablishedCallback: {() -> () in}
            )
        }),
        ("shutdownAllSubscriptions", { (context: TestContext, callbackHelper: CallbackHelper) -> () in
            context.castingServerBridge.shutdownAllSubscriptions(DispatchQueue.main,
                                                                 requestSentHandler: {() -> () in}
            )
        }),
    ]

    @Published var status: String?

    @Published var contentAppIds: [String] = []

    private var targetVideoPlayer: VideoPlayer?
    private var deviceEndpoint: ContentApp?
    private var deviceSpeakerEndpoint: ContentApp?
    private var testContext: TestContext?
    private var testsCompleted: Int = 0

    /**
     Appends the result data to the View Model's text content.

     @param result The data to append.
     */
    private func appendTestResult(_ result: String) {
        status = status?.appending("\n\(result)") ?? result
    }

    /**
     A helper function that logs a warning message and appends the warning to the View Model's text content.

     @param message The message to log.
     */
    private func warn(_ message: String) {
        logger.warning("\(message)")
        appendTestResult(message)
    }

    /**
     Executes a test and subscribes to receive callbacks when the test is complete.

     @param description A description of the test to be used for logging and display to the user.
     @param context A data structure representing the context in which the test is to be executed.
     @param test The test to execute.
     */
    private func runTest(_ description: String, context: TestContext, _ test: (TestContext, CallbackHelper) -> ())
    {
        test(context, CallbackHelper(testCaseName: description, certTestViewModel: self));
    }

    private func runNextText(context: TestContext) {
        // The number of tests completed is the index of the next test to run.
        let testsCompletedSnapshot = self.testsCompleted
        guard testsCompletedSnapshot < self.tests.count else {
            // There are no more tests to run, bail now.
            return
        }

        // Run the test
        let (description, test) = tests[testsCompletedSnapshot]
        runTest(description, context: context, test)
    }

    /**
     The function to be invoked when a test is completed.

     Note that this function is intentionally not thread safe (e.g. scheduled on a particular Dispatch Queue) to exercise the thread safety of the
     APIs being invoked by the test cases themselves. In a real application, you would want to run this on a known Dispatch Queue for safety.
     */
    private func onTestCompleted() {
        // Increment the "tests completed" counter
        testsCompleted += 1

        // Run the next test, if we're running tests sequentially.
        if let testContext = self.testContext {
            let runningTestsSequentially = !testContext.parallalizationEnabled
            if runningTestsSequentially {
                runNextText(context: testContext)
            }
        }
    }

    /**
     The function to be invoked when a test is completed successfully.

     @param description The name of the test that passed.
     */
    fileprivate func onTestPassed(_ description: String) {
        onTestCompleted()
        DispatchQueue.main.async { [weak self] in
            guard let strongSelf = self else {
                return
            }
            strongSelf.appendTestResult("🟢 \(description) PASSED")
        }
    }

    /**
     The function to be invoked when a test is completed unsuccessfully.

     @param description The name of the test that failed.
     */
    fileprivate func onTestFailed(_ description: String) {
        onTestCompleted()
        DispatchQueue.main.async { [weak self] in
            guard let strongSelf = self else {
                return
            }
            strongSelf.appendTestResult("🔴 \(description) FAILED")
        }
    }

    /**
     Begins executing all certification tests.

     @param targetContentAppId: The Endpoint ID of the target Content App against which the tests will be run.
     @param parallelizeTests: A flag that determines whether to run the tests in parallel (`true`) or sequentially (`false`).
     */
    func launchTests(targetContentAppId: String?, inParallel parallelizeTests: Bool)
    {
        // Reset the status text at the beginning of the test run
        status = ""
        testContext = nil
        testsCompleted = 0

        guard let nonNilTargetContentAppId = targetContentAppId, !nonNilTargetContentAppId.isEmpty else {
            warn("Missing input parameter(s)!")
            return
        }

        guard let targetVideoPlayer = self.targetVideoPlayer else {
            warn("Target Video Player is not configured")
            return
        }

        guard let targetContentApp = (targetVideoPlayer.contentApps as! [ContentApp]).first(
            where: { contentApp in return UInt16(targetContentAppId!) == contentApp.endpointId }
        )
        else {
            warn("Content App \(nonNilTargetContentAppId) is not supported")
            return
        }

        guard let castingServerBridge = CastingServerBridge.getSharedInstance() else {
            warn("Casting Server is unavailable")
            return
        }

        // Set up the context for running the tests and store a snapshot in this View Model.
        let testContext = TestContext(
            castingServerBridge: castingServerBridge,
            deviceEndpoint: deviceEndpoint!,
            contentAppEndpoint: targetContentApp,
            parallalizationEnabled: parallelizeTests
        )
        self.testContext = testContext

        // If parallelization is enabled, we simply schedule all of the tests at the same time and let
        // the Dispatch Queue figure it out; otherwise, we start the first test and schedule each
        // subsequent test when the completion callback of the prior test is invoked. This also is a
        // low-touch way of validating that callbacks can synchronously invoke CastingServerBridge APIs
        // from the Dispatch Queue on which the callback is invoked.
        if (parallelizeTests) {
            for (description, test) in tests {
                runTest(description, context: testContext, test)
            }
        } else {
            runNextText(context: testContext)
        }
    }

    func populateAndInitializeEndpoints()
    {
        guard let castingServerBridge = CastingServerBridge.getSharedInstance() else {
            return
        }

        castingServerBridge.getActiveTargetVideoPlayers(
            DispatchQueue.main,
            activeTargetVideoPlayersHandler: { (targetVideoPlayers: NSMutableArray?) -> () in
                let targetVideoPlayer: VideoPlayer = targetVideoPlayers![0] as! VideoPlayer
                if(targetVideoPlayer.isInitialized && targetVideoPlayer.isConnected)
                {
                    self.targetVideoPlayer = targetVideoPlayer
                    for contentApp in (targetVideoPlayer.contentApps as! [ContentApp]) {
                        if(contentApp.endpointId == 1) {
                            self.deviceEndpoint = contentApp
                        } else if(contentApp.endpointId == 2) {
                            self.deviceSpeakerEndpoint = contentApp
                        } else
                        {
                            self.contentAppIds.append(String(contentApp.endpointId))
                        }
                    }
                }
            }
        )
    }
}
