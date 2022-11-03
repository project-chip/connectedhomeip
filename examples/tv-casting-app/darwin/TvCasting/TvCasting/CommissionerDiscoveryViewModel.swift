/*
 * SPDX-FileCopyrightText: (c) 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation
import os.log

class CommissionerDiscoveryViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "CommissionerDiscoveryViewModel")
    
    @Published var commissioners: [DiscoveredNodeData] = []
    
    @Published var discoveryRequestStatus: Bool?;
    
    func discoverAndUpdate() {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.discoverCommissioners(DispatchQueue.main, discoveryRequestSentHandler:  { (result: Bool) -> () in
                self.discoveryRequestStatus = result
            })
        }
        
        Task {
            try? await Task.sleep(nanoseconds: 5_000_000_000)  // Wait for commissioners to respond
            updateCommissioners()
        }
    }
    
    private func updateCommissioners() {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            var i: Int32 = 0
            var commissioner: DiscoveredNodeData?;
            repeat {
                castingServerBridge.getDiscoveredCommissioner(i, clientQueue: DispatchQueue.main, discoveredCommissionerHandler: { (result: DiscoveredNodeData?) -> () in
                    commissioner = result;
                    if(commissioner != nil){
                        if(self.commissioners.contains(commissioner!))
                        {
                            self.Log.info("Skipping previously discovered commissioner \(commissioner!.description)")
                        }
                        else
                        {
                            self.commissioners.append(commissioner!)
                        }
                    }
                })
                i += 1
            } while(commissioner != nil)
        }
    }
}
