/*
 * SPDX-FileCopyrightText: (c) 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
                        
                        castingServerBridge.initApp(appParameters, clientQueue: DispatchQueue.main, initAppStatusHandler: { (result: Bool) -> () in
                            self.Log.info("initApp result \(result)")
                        })
                    }
                })
        }
    }
}
