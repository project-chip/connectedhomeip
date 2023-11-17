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

class StartFromCacheViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "StartFromCacheViewModel")
    
    @Published var videoPlayers: [VideoPlayer] = []
        
    func readFromCache() {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.readCachedVideoPlayers(DispatchQueue.main, readCachedVideoPlayersHandler: { (cachedVideoPlayers: NSMutableArray?) -> () in
                if(cachedVideoPlayers != nil)
                {
                    self.videoPlayers = cachedVideoPlayers! as! [VideoPlayer]
                }
            })
        }
    }
    
    func purgeAndReReadCache() {
        if let castingServerBridge = CastingServerBridge.getSharedInstance()
        {
            castingServerBridge.purgeCache(DispatchQueue.main, responseHandler: { (error: MatterError) -> () in
                DispatchQueue.main.async {
                    self.Log.info("purgeCache returned \(error)")
                    self.readFromCache()
                }
            })
        }
    }
}
