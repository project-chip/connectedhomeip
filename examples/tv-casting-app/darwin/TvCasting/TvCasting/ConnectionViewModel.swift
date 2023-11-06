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

class ConnectionViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "ConnectionViewModel")
    
    @Published var requestSent: Bool?;
    
    @Published var connectionSuccess: Bool?;

    @Published var connectionStatus: String?;

    func verifyOrEstablishConnection(selectedVideoPlayer: VideoPlayer?) {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.verifyOrEstablishConnection(selectedVideoPlayer!, clientQueue: DispatchQueue.main,
                requestSentHandler: { (error: MatterError) -> () in
                    DispatchQueue.main.async {
                        self.requestSent = (error.code == 0)
                        self.Log.info("ConnectionViewModel.verifyOrEstablishConnection.requestSentHandler called with \(error)")
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
                        self.Log.info("ConnectionViewModel.verifyOrEstablishConnection.onNewOrUpdatedEndpointCallback called with \(contentApp.endpointId)")
                    }
                })
        }
    }
}
