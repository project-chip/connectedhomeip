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

    var body: some Scene {
        WindowGroup {
            ContentView()
                .onAppear(perform: {
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
                        
                        castingServerBridge.initApp(appParameters, clientQueue: DispatchQueue.main, initAppStatusHandler: { (result: Bool) -> () in
                            self.Log.info("initApp result \(result)")
                        })
                    }
                })
        }
    }
}
