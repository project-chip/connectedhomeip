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

class MCDiscoveryExampleViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MCDiscoveryExampleViewModel")
    let kTargetPlayerDeviceType: UInt64 = 35
    
    @Published var displayedCastingPlayers: [MCCastingPlayer] = []
    
    @Published var discoveryHasError: Bool = false;
    
    func startDiscovery() {
        Log.info("startDiscovery() called")
        clearResults()
        
        // add observers
        NotificationCenter.default.addObserver(self, selector: #selector(self.didAddDiscoveredCastingPlayers), name: NSNotification.Name(ADD_CASTING_PLAYER_NOTIFICATION_NAME), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(self.didRemoveDiscoveredCastingPlayers), name: NSNotification.Name(REMOVE_CASTING_PLAYER_NOTIFICATION_NAME), object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(self.didUpdateDiscoveredCastingPlayers), name: NSNotification.Name(UPDATE_CASTING_PLAYER_NOTIFICATION_NAME), object: nil)

        if let err:Error = MCCastingPlayerDiscovery.sharedInstance().start(UInt32(kTargetPlayerDeviceType))
        {
            Log.error("MCCastingPlayerDiscovery.start failed with \(err)")
            self.discoveryHasError = true
        }
        self.discoveryHasError = false
    }
    
    func stopDiscovery() {
        Log.info("stopDiscovery() called")
        if let err:Error = MCCastingPlayerDiscovery.sharedInstance().stop()
        {
            Log.error("MCCastingPlayerDiscovery.stop failed with \(err)")
            self.discoveryHasError = true
        }
        else
        {
            // remove observers
            NotificationCenter.default.removeObserver(self, name: NSNotification.Name(ADD_CASTING_PLAYER_NOTIFICATION_NAME), object: nil)
            NotificationCenter.default.removeObserver(self, name: NSNotification.Name(REMOVE_CASTING_PLAYER_NOTIFICATION_NAME), object: nil)
            NotificationCenter.default.removeObserver(self, name: NSNotification.Name(UPDATE_CASTING_PLAYER_NOTIFICATION_NAME), object: nil)
        }
        self.discoveryHasError = false
    }
    
    func clearResults() {
        Log.info("clearResults() called")
        DispatchQueue.main.async
        {
            self.displayedCastingPlayers.removeAll()
        }
    }
    
    @objc
    func didAddDiscoveredCastingPlayers(notification: Notification)
    {
        Log.info("didAddDiscoveredCastingPlayers() called")
        guard let userInfo = notification.userInfo,
            let castingPlayer     = userInfo["castingPlayer"] as? MCCastingPlayer else {
            self.Log.error("didAddDiscoveredCastingPlayers called with no MCCastingPlayer")
            return
        }
        
        self.Log.info("didAddDiscoveredCastingPlayers notified of a MCCastingPlayer with ID: \(castingPlayer.identifier())")

        DispatchQueue.main.async
        {
            self.displayedCastingPlayers.append(castingPlayer)
        }
    }
    
    @objc
    func didRemoveDiscoveredCastingPlayers(notification: Notification)
    {
        Log.info("didRemoveDiscoveredCastingPlayers() called")
        guard let userInfo = notification.userInfo,
            let castingPlayer     = userInfo["castingPlayer"] as? MCCastingPlayer else {
            self.Log.error("didRemoveDiscoveredCastingPlayers called with no MCCastingPlayer")
            return
        }
        
        self.Log.info("didRemoveDiscoveredCastingPlayers notified of a MCCastingPlayer with ID: \(castingPlayer.identifier())")
        DispatchQueue.main.async
        {
            self.displayedCastingPlayers.removeAll(where: {$0 == castingPlayer})
        }
    }
    
    @objc
    func didUpdateDiscoveredCastingPlayers(notification: Notification)
    {
        Log.info("didUpdateDiscoveredCastingPlayers() called")
        guard let userInfo = notification.userInfo,
            let castingPlayer     = userInfo["castingPlayer"] as? MCCastingPlayer else {
            self.Log.error("didUpdateDiscoveredCastingPlayers called with no MCCastingPlayer")
            return
        }
        
        self.Log.info("didUpdateDiscoveredCastingPlayers notified of a MCCastingPlayer with ID: \(castingPlayer.identifier())")
        if let index = displayedCastingPlayers.firstIndex(where: { castingPlayer.identifier() == $0.identifier() })
        {
            DispatchQueue.main.async
            {
                self.displayedCastingPlayers[index] = castingPlayer
            }
        }
    }
}
