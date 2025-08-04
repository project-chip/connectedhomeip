/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

class MCEndpointSelector {
    static let Log = Logger(subsystem: "com.matter.casting", category: "MCEndpointSelector")
    
    static func selectEndpoint(from castingPlayer: MCCastingPlayer, sampleEndpointVid: Int) -> MCEndpoint? {
        Log.info("MCEndpointSelector.selectEndpoint()")
        
        if let endpoint = castingPlayer.endpoints().filter({ $0.vendorId().intValue == sampleEndpointVid }).first {
            Log.info("MCEndpointSelector.selectEndpoint() Found endpoint matching the sampleEndpointVid: \(sampleEndpointVid)")
            return endpoint
        } else if let endpoint = castingPlayer.endpoints().filter({ $0.identifier().intValue == 1 }).first {
            Log.info("MCEndpointSelector.selectEndpoint() No endpoint matching the sampleEndpointVid: \(sampleEndpointVid), but found endpoint with identifier: 1")
            return endpoint
        }
        
        Log.error("No endpoint matching the example VID or identifier 1 found")
        return nil
    }
}
