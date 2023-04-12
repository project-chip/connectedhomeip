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

class ContentLauncherViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "ContentLauncherViewModel")
    
    @Published var status: String?;
    
    @Published var contentAppIds: [String] = [];
    
    var targetVideoPlayer: VideoPlayer?;
    
    func launchUrl(targetContentAppId: String?, contentUrl: String?, contentDisplayStr: String?)
    {
        if ((targetContentAppId != nil && !targetContentAppId!.isEmpty) && (contentUrl != nil && !contentUrl!.isEmpty) && (contentDisplayStr != nil && !contentDisplayStr!.isEmpty))
        {
            var targetContentApp: ContentApp?
            for contentApp in (targetVideoPlayer!.contentApps as! [ContentApp]) {
                if(UInt16(targetContentAppId!) == contentApp.endpointId)
                {
                    targetContentApp = contentApp
                    break
                }
            }

            if let castingServerBridge = CastingServerBridge.getSharedInstance()
            {
                castingServerBridge
                    .contentLauncher_launchUrl(targetContentApp!, contentUrl: contentUrl!,
                                              contentDisplayStr: contentDisplayStr!,
                                              responseCallback:
                                                { (result: Bool) -> () in
                        self.Log.info("ContentLauncherViewModel.launchUrl.launchUrlResponseCallback result \(result)")
                        DispatchQueue.main.async {
                            self.status = result ? "Launched URL successfully" : "Launch URL failure!"
                        }
                    },
                    clientQueue: DispatchQueue.main,
                    requestSentHandler: { (result: Bool) -> () in
                        self.Log.info("ContentLauncherViewModel.launchUrl.launcUrlRequestSentHandler result \(result)")
                        self.status = result ? "Sent Launch URL request" : "Failed to send Launch URL request!"
                    })
            }
        }
        else
        {
            Log.debug("ContentLauncherViewModel.launchUrl input(s) missing!")
            self.status = "Missing input parameter(s)!"
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
                        if(contentApp.supportsContentLauncher())
                        {
                            self.contentAppIds.append(String(contentApp.endpointId))
                        }
                    }
                }
            })
        }
    }
}
