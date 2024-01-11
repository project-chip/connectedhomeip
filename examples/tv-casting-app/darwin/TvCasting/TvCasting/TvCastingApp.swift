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

import SwiftUI
import os.log

@main
struct TvCastingApp: App {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "TvCastingApp")
    @State
    var firstAppActivation: Bool = true

    var body: some Scene {
        WindowGroup {
            ContentView()
                .onAppear(perform: {
                    if ProcessInfo.processInfo.environment["CHIP_CASTING_SIMPLIFIED"] == "1"
                    {
                        self.Log.info("CHIP_CASTING_SIMPLIFIED = 1")
                        
                        let err: Error? = MCInitializationExample().initialize()
                        if err != nil
                        {
                            self.Log.error("MCCastingApp initialization failed \(err)")
                            return
                        }
                    }
                    else
                    {
                        self.Log.info("CHIP_CASTING_SIMPLIFIED = 0")

                        if let castingServerBridge = CastingServerBridge.getSharedInstance()
                        {
                            let appParameters: AppParameters = AppParameters()
                            
                            var rotatingDeviceIdUniqueId: [UInt8] = [UInt8](repeating: 0, count: 16 )
                            for i in (0...15)
                            {
                                rotatingDeviceIdUniqueId[i] = UInt8.random(in: 0..<255)
                            }
                            appParameters.rotatingDeviceIdUniqueId = Data(rotatingDeviceIdUniqueId)
                            
                            let onboardingParameters: OnboardingPayload = OnboardingPayload()
                            onboardingParameters.setupPasscode = 20202021
                            onboardingParameters.setupDiscriminator = 3840
                            
                            appParameters.onboardingPayload = onboardingParameters
                            
                            let err = castingServerBridge.initializeApp(appParameters, clientQueue: DispatchQueue.main, initAppStatusHandler: { (result: Bool) -> () in
                                self.Log.info("initializeApp result \(result)")
                            })
                            self.Log.info("initializeApp return value \(err)")
                        }
                    }
                })
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.willResignActiveNotification)) { _ in
                    self.Log.info("TvCastingApp: UIApplication.willResignActiveNotification")
                    if ProcessInfo.processInfo.environment["CHIP_CASTING_SIMPLIFIED"] == "1"
                    {
                        if let castingApp = MCCastingApp.getSharedInstance()
                        {
                            castingApp.stop(completionBlock: { (err : Error?) -> () in
                                if err != nil
                                {
                                    self.Log.error("MCCastingApp stop failed \(err)")
                                }
                            })
                        }
                    }
                    else if let castingServerBridge = CastingServerBridge.getSharedInstance()
                    {
                        castingServerBridge.stopMatterServer()
                    }
                }
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.didBecomeActiveNotification)) { _ in
                    self.Log.info("TvCastingApp: UIApplication.didBecomeActiveNotification")
                    if ProcessInfo.processInfo.environment["CHIP_CASTING_SIMPLIFIED"] == "1"
                    {
                        if let castingApp = MCCastingApp.getSharedInstance()
                        {
                            castingApp.start(completionBlock: { (err : Error?) -> () in
                                if err != nil
                                {
                                    self.Log.error("MCCastingApp start failed \(err)")
                                }
                            })
                        }
                    }
                    else
                    {
                        if(!firstAppActivation)
                        {
                            if let castingServerBridge = CastingServerBridge.getSharedInstance()
                            {
                                castingServerBridge.startMatterServer(DispatchQueue.main, startMatterServerCompletionCallback: { (error: MatterError) -> () in
                                    DispatchQueue.main.async {
                                        self.Log.info("TvCastingApp.startMatterServerCompletionCallback called with \(error)")
                                    }
                                })
                            }
                        }
                        firstAppActivation = false
                    }
                }
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.didEnterBackgroundNotification)) { _ in
                    self.Log.info("TvCastingApp: UIApplication.didEnterBackgroundNotification")
                }
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.didFinishLaunchingNotification)) { _ in
                    self.Log.info("TvCastingApp: UIApplication.didFinishLaunchingNotification")
                }
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.willEnterForegroundNotification)) { _ in
                    self.Log.info("TvCastingApp: UIApplication.willEnterForegroundNotification")
                }
                .onReceive(NotificationCenter.default.publisher(for: UIApplication.willTerminateNotification)) { _ in
                    self.Log.info("TvCastingApp: UIApplication.willTerminateNotification")
                }
        }
    }
}
