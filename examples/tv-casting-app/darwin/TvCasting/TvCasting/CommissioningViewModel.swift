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

    @Published var connectionSuccess: Bool?;

    @Published var connectionStatus: String?;

    func prepareForCommissioning(selectedCommissioner: DiscoveredNodeData?) {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.openBasicCommissioningWindow(DispatchQueue.main,
                commissioningWindowRequestedHandler: { (result: Bool) -> () in
                    DispatchQueue.main.async {
                        self.commisisoningWindowOpened = result
                    }
                },
                commissioningCompleteCallback: { (result: Bool) -> () in
                    self.Log.info("Commissioning status: \(result)")
                    DispatchQueue.main.async {
                        self.commisisoningComplete = result
                    }
                },
                onConnectionSuccessCallback: { (videoPlayer: VideoPlayer) -> () in
                    DispatchQueue.main.async {
                        self.connectionSuccess = true
                        self.connectionStatus = "Connected to \(String(describing: videoPlayer))"
                        self.Log.info("ConnectionViewModel.verifyOrEstablishConnection.onConnectionSuccessCallback called with \(videoPlayer.nodeId)")
                    }
                },
                onConnectionFailureCallback: { (error: MatterError) -> () in
                    DispatchQueue.main.async {
                        self.connectionSuccess = false
                        self.connectionStatus = "Failed to connect to video player!"
                        self.Log.info("ConnectionViewModel.verifyOrEstablishConnection.onConnectionFailureCallback called with \(error)")
                    }
                },
                onNewOrUpdatedEndpointCallback: { (contentApp: ContentApp) -> () in
                    DispatchQueue.main.async {
                        self.Log.info("CommissioningViewModel.openBasicCommissioningWindow.onNewOrUpdatedEndpointCallback called with \(contentApp.endpointId)")
                    }
                })
        }
                
        // Send User directed commissioning request if a commissioner with a known IP addr was selected
        if(selectedCommissioner != nil && selectedCommissioner!.numIPs > 0)
        {
            sendUserDirectedCommissioningRequest(selectedCommissioner: selectedCommissioner)
        }
    }
    
    private func sendUserDirectedCommissioningRequest(selectedCommissioner: DiscoveredNodeData?) {        
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.sendUserDirectedCommissioningRequest(selectedCommissioner!, clientQueue: DispatchQueue.main, udcRequestSentHandler: { (result: Bool) -> () in
                self.udcRequestSent = result
            })
        }
    }
}
