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

class MTRConnectionExampleViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MTRConnectionExampleViewModel")
    
    // VendorId of the MTREndpoint on the MTRCastingPlayer that the MTRCastingApp desires to interact with after connection
    let kDesiredEndpointVendorId: UInt16 = 65521;
    
    @Published var connectionSuccess: Bool?;

    @Published var connectionStatus: String?;

    func connect(selectedCastingPlayer: MTRCastingPlayer?) {
        let desiredEndpointFilter: MTREndpointFilter = MTREndpointFilter()
        desiredEndpointFilter.vendorId = kDesiredEndpointVendorId
        selectedCastingPlayer?.verifyOrEstablishConnection(completionBlock: { err in
            self.Log.error("MTRConnectionExampleViewModel connect() completed with \(err)")
            if(err == nil)
            {
                self.connectionSuccess = true
                self.connectionStatus = "Connected!"
            }
            else
            {
                self.connectionSuccess = false
                self.connectionStatus = "Connection failed with \(String(describing: err))"
            }
        }, desiredEndpointFilter: desiredEndpointFilter)
    }
}
