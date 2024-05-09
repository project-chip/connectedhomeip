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

class MediaPlaybackViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MediaPlaybackViewModel")
    
    @Published var requestStatus: String?;
    @Published var subscriptionStatus: String?;
    @Published var readResponse: String?;

    @Published var contentAppIds: [String] = [];
    
    var targetVideoPlayer: VideoPlayer?;
    
    func subscribeCurrentState(targetContentAppId: String?, minInterval: String, maxInterval: String)
    {
        if((targetContentAppId != nil && !targetContentAppId!.isEmpty))
        {
            if let castingServerBridge = CastingServerBridge.getSharedInstance()
            {
                var targetContentApp: ContentApp?
                for contentApp in (targetVideoPlayer!.contentApps as! [ContentApp]) {
                    if(UInt16(targetContentAppId!) == contentApp.endpointId)
                    {
                        targetContentApp = contentApp
                        break
                    }
                }

                castingServerBridge.mediaPlayback_subscribeCurrentState(targetContentApp!, minInterval: UInt16(minInterval) ?? 0, maxInterval: UInt16(maxInterval) ?? 1, clientQueue: DispatchQueue.main,
                                                                        requestSentHandler: { (result: MatterError) -> () in
                    self.Log.info("MediaPlaybackViewModel.subscribeToCurrentState.requestSentHandler result \(result)")
                    self.requestStatus = result.code == 0 ? "Subscribe request sent!" : "Failed to send Subscribe request!"
                },
                                                                        successCallback: { (result: MediaPlayback_PlaybackState) -> () in
                    DispatchQueue.main.async {
                        self.Log.info("MediaPlaybackViewModel.subscribeToCurrentState.successCallback called")
                        switch(result)
                        {
                        case .Playing:
                            self.readResponse = "Current state: Playing"
                            break
                        case .Paused:
                            self.readResponse = "Current state: Paused"
                            break
                        case .NotPlaying:
                            self.readResponse = "Current state: NotPlaying"
                            break
                        case .Buffering:
                            self.readResponse = "Current state: Buffering"
                            break
                        default:
                            self.readResponse = "Current state: Unknown!"
                            break
                        }
                    }
                },
                                                                        failureCallback: { (result: MatterError) -> () in
                    DispatchQueue.main.async {
                        self.Log.info("MediaPlaybackViewModel.subscribeToCurrentState.failureCallback called with \(result)")
                        self.readResponse = "Attribute read failure!"
                    }
                },
                                                                        subscriptionEstablishedCallback: { () -> () in
                    DispatchQueue.main.async {
                        self.Log.info("MediaPlaybackViewModel.subscribeToCurrentState.subscriptionEstablishedCallback called")
                        self.subscriptionStatus = "Subscription established!"
                    }
                }
                )
            }
        }
        else
        {
            Log.debug("MediaPlaybackViewModel.subscribeToCurrentState No endpoint selected!")
            self.requestStatus = "No endpoint selected!"
        }
    }
    
    func populateContentApps()
    {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.getActiveTargetVideoPlayers(DispatchQueue.main,
                activeTargetVideoPlayersHandler: { (targetVideoPlayers: NSMutableArray?) -> () in
                let targetVideoPlayer: VideoPlayer = targetVideoPlayers![0] as! VideoPlayer
                if(targetVideoPlayer.isInitialized && targetVideoPlayer.isConnected)
                {
                    self.targetVideoPlayer = targetVideoPlayer
                    for contentApp in (targetVideoPlayer.contentApps as! [ContentApp])
                    {
                        if(contentApp.supportsMediaPlayback())
                        {
                            self.contentAppIds.append(String(contentApp.endpointId))
                        }
                    }
                }
            })
        }
    }
}
