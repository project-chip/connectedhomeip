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

class CommissionerDiscoveryViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "CommissionerDiscoveryViewModel")
    
    @Published var commissioners: [DiscoveredNodeData] = []
    
    @Published var discoveryRequestStatus: Bool?;
    
    func discoverAndUpdate() {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.discoverCommissioners(DispatchQueue.main,
                discoveryRequestSentHandler:  { (result: MatterError) -> () in
                    self.discoveryRequestStatus = (result == MATTER_NO_ERROR)
                },
                discoveredCommissionerHandler: { (commissioner: DiscoveredNodeData) -> () in
                    self.Log.info("discoveredCommissionerHandler called with \(commissioner)")
                    if(self.commissioners.contains(commissioner))
                    {
                        let index = self.commissioners.firstIndex(of: commissioner)
                        self.commissioners[index!] = commissioner
                        self.Log.info("Updating previously discovered commissioner \(commissioner.description)")
                    }
                    else if(commissioner.numIPs == 0)
                    {
                        self.Log.info("Skipping commissioner because it does not have any resolved IP addresses \(commissioner.description)")
                    }
                    else
                    {
                        self.commissioners.append(commissioner)
                    }
                }
            )
        }
    }
}
