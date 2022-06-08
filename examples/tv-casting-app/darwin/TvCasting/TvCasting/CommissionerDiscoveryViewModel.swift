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

class CommissionerDiscoveryViewModel: ObservableObject {
    @Published var commissioners: [DiscoveredNodeData] = []
    func discoverAndUpdate() {
        if(commissioners.isEmpty) {
            CastingServerBridge.getSharedInstance().discoverCommissioners()
            Task {
                try? await Task.sleep(nanoseconds: 5_000_000_000)  // Wait for commissioners to respond
                await updateCommissioners()
            }
        }
    }
    
    private func updateCommissioners() async {
        var i: Int32 = 0
        while(CastingServerBridge.getSharedInstance().getDiscoveredCommissioner(i) != nil)
        {
            let commissioner = CastingServerBridge.getSharedInstance().getDiscoveredCommissioner(i)!
            DispatchQueue.main.async {
                self.commissioners.append(commissioner)
            }
            i += 1
        }
    }
}
