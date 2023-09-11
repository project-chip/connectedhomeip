/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
