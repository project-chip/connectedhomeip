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

class MCApplicationBasicReadVendorIDExampleViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MCApplicationBasicReadVendorIDExampleViewModel")
    
    @Published var status: String?;

    // VendorId of the MCEndpoint on the MCCastingPlayer that the MCCastingApp desires to interact with after connection
    let sampleEndpointVid: Int = 65521

    func read(castingPlayer: MCCastingPlayer)
    {
        // select the MCEndpoint on the MCCastingPlayer to invoke the command on
        if let endpoint: MCEndpoint = castingPlayer.endpoints().filter({ $0.vendorId().intValue == sampleEndpointVid}).first
        {
            // validate that the selected endpoint supports the ApplicationBasic cluster
            if(!endpoint.hasCluster(MCEndpointClusterTypeApplicationBasic))
            {
                self.Log.error("No ApplicationBasic cluster supporting endpoint found")
                DispatchQueue.main.async
                {
                    self.status = "No ApplicationBasic cluster supporting endpoint found"
                }
                return
            }
            
            // get ApplicationBasic cluster from the endpoint
            let applicationBasiccluster: MCApplicationBasicCluster = endpoint.cluster(for: MCEndpointClusterTypeApplicationBasic) as! MCApplicationBasicCluster

            // get the vendorIDAttribute from the applicationBasiccluster
            let vendorIDAttribute: MCApplicationBasicClusterVendorIDAttribute? = applicationBasiccluster.vendorIDAttribute()
            if(vendorIDAttribute == nil)
            {
                self.Log.error("VendorID attribute not supported on cluster")
                DispatchQueue.main.async
                {
                    self.status = "VendorID attribute not supported on cluster"
                }
                return
            }
                
                    
            // call read on vendorIDAttribute and pass in a completion block
            vendorIDAttribute!.read(nil) { context, before, after, err in
                DispatchQueue.main.async
                {
                    if(err != nil)
                    {
                        self.Log.error("Error when reading VendorID value \(String(describing: err))")
                        self.status = "Error when reading VendorID value \(String(describing: err))"
                        return
                    }
                
                    if(before != nil)
                    {
                        self.Log.info("Read VendorID value: \(String(describing: after)) Before: \(String(describing: before))")
                        self.status = "Read VendorID value: \(String(describing: after)) Before: \(String(describing: before))"
                    }
                    else
                    {
                        self.Log.info("Read VendorID value: \(String(describing: after))")
                        self.status = "Read VendorID value: \(String(describing: after))"
                    }
                }
            }
        }
        else
        {
            self.Log.error("No endpoint matching the example VID found")
            DispatchQueue.main.async
            {
                self.status = "No endpoint matching the example VID found"
            }
        }
    }
}
