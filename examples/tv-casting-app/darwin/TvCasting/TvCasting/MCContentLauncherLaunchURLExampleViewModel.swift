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

class MCContentLauncherLaunchURLExampleViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MCContentLauncherLaunchURLExampleViewModel")

    @Published var status: String?;

    // VendorId of the MCEndpoint on the MCCastingPlayer that the MCCastingApp desires to interact with after connection
    let sampleEndpointVid: Int = 65521

    func invokeCommand(castingPlayer: MCCastingPlayer, contentUrl: String, displayString: String)
    {
        self.Log.info("MCContentLauncherLaunchURLExampleViewModel.invokeCommand()")
        castingPlayer.logAllEndpoints()

        // Use MCEndpointSelector to select the endpoint
        guard let endpoint = MCEndpointSelector.selectEndpoint(from: castingPlayer, sampleEndpointVid: sampleEndpointVid) else {
            self.Log.error("MCContentLauncherLaunchURLExampleViewModel.invokeCommand() No endpoint matching the example VID or identifier 1 found")
            DispatchQueue.main.async {
                self.status = "No endpoint matching the example VID or identifier 1 found"
            }
            return
        }

        self.Log.info("MCContentLauncherLaunchURLExampleViewModel.invokeCommand() selected endpoint: \(endpoint.description)")

        // validate that the selected endpoint supports the ContentLauncher cluster
        if(!endpoint.hasCluster(MCEndpointClusterTypeContentLauncher))
        {
            self.Log.error("MCContentLauncherLaunchURLExampleViewModel.invokeCommand() No ContentLauncher cluster supporting endpoint found")
            DispatchQueue.main.async
            {
                self.status = "No ContentLauncher cluster supporting endpoint found"
            }
            return
        }
        
        // get ContentLauncher cluster from the endpoint
        let contentLaunchercluster: MCContentLauncherCluster = endpoint.cluster(for: MCEndpointClusterTypeContentLauncher) as! MCContentLauncherCluster

        // get the launchURLCommand from the contentLauncherCluster
        let launchURLCommand: MCContentLauncherClusterLaunchURLCommand? = contentLaunchercluster.launchURLCommand()
        if(launchURLCommand == nil)
        {
            self.Log.error("LaunchURL not supported on cluster")
            DispatchQueue.main.async
            {
                self.status = "LaunchURL not supported on cluster"
            }
            return
        }
            
        // create the LaunchURL request
        let request: MCContentLauncherClusterLaunchURLParams = MCContentLauncherClusterLaunchURLParams()
        request.contentURL = contentUrl
        request.displayString = displayString
                
        // call invoke on launchURLCommand while passing in a completion block
        launchURLCommand!.invoke(request, context: nil, completion: { context, err, response in
            DispatchQueue.main.async
            {
                if(err == nil)
                {
                    self.Log.info("LaunchURLCommand invoke completion success with \(String(describing: response))")
                    self.status = "Success. Response data: \(String(describing: response?.data))"
                }
                else
                {
                    self.Log.error("LaunchURLCommand invoke completion failure with \(String(describing: err))")
                    self.status = "Failure: \(String(describing: err))"
                }
            }
        },
        timedInvokeTimeoutMs: 5000) // time out after 5000ms
    }
}
