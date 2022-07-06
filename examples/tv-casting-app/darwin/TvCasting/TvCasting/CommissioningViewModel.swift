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

class CommissioningViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "CommissioningViewModel")
    
    @Published var udcRequestSent: Bool?;
    
    @Published var commisisoningWindowOpened: Bool?;

    @Published var commisisoningComplete: Bool?;

    func prepareForCommissioning(selectedCommissioner: DiscoveredNodeData?) {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.openBasicCommissioningWindow(
                { (result: Bool) -> () in
                    // commissioning complete handler code
                    self.Log.info("Commissioning status: \(result)")
                    DispatchQueue.main.async {
                        self.commisisoningComplete = result
                    }
                },
                clientQueue: DispatchQueue.main,
                commissioningWindowRequestedHandler: { (result: Bool) -> () in
                    self.commisisoningWindowOpened = result
                })
        }
        
        // TBD: Get Onboarding payload
        
        // Send User directed commissioning request if a commissioner with a known IP addr was selected
        if(selectedCommissioner != nil && selectedCommissioner!.numIPs > 0)
        {
            sendUserDirectedCommissioningRequest(selectedCommissioner: selectedCommissioner)
        }
    }
    
    private func sendUserDirectedCommissioningRequest(selectedCommissioner: DiscoveredNodeData?) {
        let ipAddress: String = selectedCommissioner!.ipAddresses[0] as! String
        let port: UInt16 = selectedCommissioner!.port
        let platformInterface: UInt32 = selectedCommissioner!.platformInterface
        
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.sendUserDirectedCommissioningRequest(ipAddress, commissionerPort: port, platformInterface: platformInterface, clientQueue: DispatchQueue.main, udcRequestSentHandler: { (result: Bool) -> () in
                self.udcRequestSent = result
            })
        }
    }
}
