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

class MCMediaPlaybackSubscribeToCurrentStateExampleViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MCMediaPlaybackSubscribeToCurrentStateExampleViewModel")
    
    @Published var status: String?;

    // VendorId of the MCEndpoint on the MCCastingPlayer that the MCCastingApp desires to interact with after connection
    let sampleEndpointVid: Int = 65521

    func subscribe(castingPlayer: MCCastingPlayer)
    {
        self.Log.info("MCMediaPlaybackSubscribeToCurrentStateExampleViewModel.subscribe()")
        castingPlayer.logAllEndpoints()

        // Use MCEndpointSelector to select the endpoint
        guard let endpoint = MCEndpointSelector.selectEndpoint(from: castingPlayer, sampleEndpointVid: sampleEndpointVid) else {
            self.Log.error("MCMediaPlaybackSubscribeToCurrentStateExampleViewModel.subscribe() No endpoint matching the example VID or identifier 1 found")
            DispatchQueue.main.async {
                self.status = "No endpoint matching the example VID or identifier 1 found"
            }
            return
        }

        self.Log.info("MCMediaPlaybackSubscribeToCurrentStateExampleViewModel.subscribe() selected endpoint: \(endpoint.description)")

        // validate that the selected endpoint supports the MediaPlayback cluster
        if(!endpoint.hasCluster(MCEndpointClusterTypeMediaPlayback))
        {
            self.Log.error("MCMediaPlaybackSubscribeToCurrentStateExampleViewModel.subscribe() No MediaPlayback cluster supporting endpoint found")
            DispatchQueue.main.async
            {
                self.status = "No MediaPlayback cluster supporting endpoint found"
            }
            return
        }
        
        // get MediaPlayback cluster from the endpoint
        let mediaPlaybackCluster: MCMediaPlaybackCluster = endpoint.cluster(for: MCEndpointClusterTypeMediaPlayback) as! MCMediaPlaybackCluster

        // get the currentStateAttribute from the mediaPlaybackCluster
        let currentStateAttribute: MCMediaPlaybackClusterCurrentStateAttribute? = mediaPlaybackCluster.currentStateAttribute()
        if(currentStateAttribute == nil)
        {
            self.Log.error("CurrentState attribute not supported on cluster")
            DispatchQueue.main.async
            {
                self.status = "CurrentState attribute not supported on cluster"
            }
            return
        }
            
                
        // call read on currentStateAttribute and pass in a completion block
        currentStateAttribute!.subscribe(nil, completion: { context, before, after, err in
            let dateFormatter = DateFormatter()
            dateFormatter.dateFormat = "HH:mm:ss"
            let currentTime = dateFormatter.string(from: Date())
            DispatchQueue.main.async
            {
                if(err != nil)
                {
                    self.Log.error("Error when reading CurrentState value \(String(describing: err)) at \(currentTime)")
                    self.status = "Error when reading CurrentState value \(String(describing: err)) at \(currentTime)"
                    return
                }
                if(before != nil)
                {
                    self.Log.info("Read CurrentState value: \(String(describing: after)) Before: \(String(describing: before)) at \(currentTime)")
                    self.status = "Read CurrentState value: \(String(describing: after)) Before: \(String(describing: before)) at \(currentTime)"
                }
                else
                {
                    self.Log.info("Read CurrentState value: \(String(describing: after)) at \(currentTime)")
                    self.status = "Read CurrentState value: \(String(describing: after)) at \(currentTime)"
                }
            }
        }, minInterval: 0, maxInterval: 1)
    }
}
