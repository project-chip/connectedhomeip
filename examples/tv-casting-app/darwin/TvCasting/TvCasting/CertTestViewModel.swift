/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

class CallbackHelper {
    var testCaseName: String;
    var certTestViewModel: CertTestViewModel;
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "CertTestViewModel")

    init(testCaseName: String, certTestViewModel: CertTestViewModel) {
        self.testCaseName = testCaseName
        self.certTestViewModel = certTestViewModel
    }

    func responseCallback(result: Bool)
    {
        self.Log.info("CertTestViewModel.responseCallback.\(self.testCaseName) result \(result)")
        DispatchQueue.main.async {
            self.certTestViewModel.status = result ? "Test \(self.testCaseName) successful" : "Test \(self.testCaseName) failed"
        }
    }

    func requestSentHandler(result: Bool)
    {
        self.Log.info("CertTestViewModel.requestSentHandler.\(self.testCaseName) result \(result)")
    }

    func successCallbackString(result: String)
    {
        self.Log.info("CertTestViewModel.successCallback.\(self.testCaseName) result \(result)")
        DispatchQueue.main.async {
            self.certTestViewModel.status = "Test \(self.testCaseName) successful"
        }
    }

    func successCallbackInteger(result: UInt16)
    {
        self.Log.info("CertTestViewModel.successCallback.\(self.testCaseName) result \(result)")
        DispatchQueue.main.async {
            self.certTestViewModel.status = "Test \(self.testCaseName) successful"
        }
    }

    func successCallbackNumber(result: NSNumber)
    {
        self.Log.info("CertTestViewModel.successCallback.\(self.testCaseName) result \(result)")
        DispatchQueue.main.async {
            self.certTestViewModel.status = "Test \(self.testCaseName) successful"
        }
    }

    func failureCallback(result: MatterError)
    {
        self.Log.info("CertTestViewModel.failureCallback.\(self.testCaseName) failed. Code : \(result.code). Message : \(result.message ?? "")")
        DispatchQueue.main.async {
            self.certTestViewModel.status = "Test \(self.testCaseName) failed. Code : \(result.code). Message : \(result.message ?? "")"
        }
    }

    func requestSentHandlerError(result: MatterError)
    {
        self.Log.info("CertTestViewModel.requestSentHandler.\(self.testCaseName).  Code : \(result.code). Message : result.message")
    }

}

class CertTestViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "CertTestViewModel")

    @Published var status: String?;

    @Published var contentAppIds: [String] = [];

    var targetVideoPlayer: VideoPlayer?;
    var deviceEndpoint: ContentApp?;
    var deviceSpeakerEndpoint: ContentApp?;
    var testCaseName: String = "";

    func runCertTest(testCaseName: String, test: (CallbackHelper) -> ())
    {
        self.testCaseName = testCaseName;
        test(CallbackHelper(testCaseName: testCaseName, certTestViewModel: self));
    }

    func launchTest(targetContentAppId: String?)
    {
        if (targetContentAppId != nil && !targetContentAppId!.isEmpty)
        {
            var targetContentApp: ContentApp?
            for contentApp in (targetVideoPlayer!.contentApps as! [ContentApp]) {
                if(UInt16(targetContentAppId!) == contentApp.endpointId)
                {
                    targetContentApp = contentApp
                    break
                }
            }

            if let castingServerBridge = CastingServerBridge.getSharedInstance()
            {
                runCertTest(testCaseName: "keypadInput_sendKey",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.keypadInput_sendKey(deviceEndpoint!, keyCode: 10,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "applicationLauncher_launch",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.applicationLauncher_launch(deviceEndpoint!, catalogVendorId: 123, applicationId: "exampleid", data: nil,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "applicationLauncher_stop",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.applicationLauncher_stop(deviceEndpoint!,
                            catalogVendorId: 123, applicationId: "exampleid",
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "applicationLauncher_hide",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.applicationLauncher_hide(deviceEndpoint!,
                            catalogVendorId: 123, applicationId: "exampleid",
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "targetNavigator_navigateTarget",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.targetNavigator_navigateTarget(deviceEndpoint!,
                            target: 1, data: "",
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "contentLauncher_launchUrl",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.contentLauncher_launchUrl(targetContentApp!,
                            contentUrl: "https://dummyurl",
                            contentDisplayStr: "Dummy Content",
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "contentLauncher_launchContent",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.contentLauncher_launchContent(targetContentApp!,
                            contentSearch: ContentLauncher_ContentSearch(parameterList: [ContentLauncher_Parameter(type: ContentLauncher_ParameterEnum.Video, value: "Dummy Video", externalIDList: [ContentLauncher_AdditionalInfo(name: "imdb", value: "dummyId"),]),]),
                            autoPlay: true, data: "",
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "mediaPlayback_play",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.mediaPlayback_play(targetContentApp!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "mediaPlayback_next",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.mediaPlayback_next(targetContentApp!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "mediaPlayback_skipForward",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.mediaPlayback_skipForward(targetContentApp!,
                            deltaPositionMilliseconds: 10000,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "mediaPlayback_skipBackward",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.mediaPlayback_skipBackward(targetContentApp!,
                            deltaPositionMilliseconds: 10000,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "mediaPlayback_pause",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.mediaPlayback_pause(targetContentApp!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "mediaPlayback_stopPlayback",
                    test: { (callbackHelper: CallbackHelper) -> () in
                        castingServerBridge.mediaPlayback_stopPlayback(targetContentApp!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "onOff_on",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.onOff_(on: deviceEndpoint!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "onOff_off",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.onOff_off(deviceEndpoint!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "onOff_toggle",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.onOff_toggle(deviceEndpoint!,
                            responseCallback:callbackHelper.responseCallback,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler:callbackHelper.requestSentHandler
                        )
                    }
                )

                runCertTest(testCaseName: "applicationBasic_readApplicationVersion",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.applicationBasic_readApplicationVersion(targetContentApp!,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler: callbackHelper.requestSentHandlerError,
                            successCallback: callbackHelper.successCallbackString,
                            failureCallback: callbackHelper.failureCallback
                        )
                    }
                )

                runCertTest(testCaseName: "applicationBasic_readVendorName",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.applicationBasic_readVendorName(targetContentApp!,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler: callbackHelper.requestSentHandlerError,
                            successCallback: callbackHelper.successCallbackString,
                            failureCallback: callbackHelper.failureCallback
                        )
                    }
                )

                runCertTest(testCaseName: "applicationBasic_readApplicationName",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.applicationBasic_readApplicationName(targetContentApp!,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler: callbackHelper.requestSentHandlerError,
                            successCallback: callbackHelper.successCallbackString,
                            failureCallback: callbackHelper.failureCallback
                        )
                    }
                )

                runCertTest(testCaseName: "applicationBasic_readVendorID",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.applicationBasic_readVendorID(targetContentApp!,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler: callbackHelper.requestSentHandlerError,
                            successCallback: callbackHelper.successCallbackNumber,
                            failureCallback: callbackHelper.failureCallback
                        )
                    }
                )

                runCertTest(testCaseName: "applicationBasic_readProductID",
                    test: { (callbackHelper: CallbackHelper) -> () in
                    castingServerBridge.applicationBasic_readProductID(targetContentApp!,
                            clientQueue: DispatchQueue.main,
                            requestSentHandler: callbackHelper.requestSentHandlerError,
                            successCallback: callbackHelper.successCallbackInteger,
                            failureCallback: callbackHelper.failureCallback
                        )
                    }
                )
            }
        }
        else
        {
            Log.debug("CertTestViewModel.launchTest input(s) missing!")
            self.status = "Missing input parameter(s)!"
        }
    }

    func populateAndInitializeEndpoints()
    {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.getActiveTargetVideoPlayers(DispatchQueue.main,
                activeTargetVideoPlayersHandler: { (targetVideoPlayers: NSMutableArray?) -> () in
                let targetVideoPlayer: VideoPlayer = targetVideoPlayers![0] as! VideoPlayer
                if(targetVideoPlayer.isInitialized && targetVideoPlayer.isConnected)
                {
                    self.targetVideoPlayer = targetVideoPlayer
                    for contentApp in (targetVideoPlayer.contentApps as! [ContentApp])
                    {
                        if(contentApp.endpointId == 1)
                        {
                            self.deviceEndpoint = contentApp
                        } else if(contentApp.endpointId == 2)
                        {
                            self.deviceSpeakerEndpoint = contentApp
                        } else
                        {
                            self.contentAppIds.append(String(contentApp.endpointId))
                        }
                    }
                }
            })
        }
    }
}
