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

import SwiftUI

struct MCActionSelectorView: View {
    var selectedCastingPlayer: MCCastingPlayer?

    init(_selectedCastingPlayer: MCCastingPlayer?) {
        self.selectedCastingPlayer = _selectedCastingPlayer
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            NavigationLink(
                destination: MCContentLauncherLaunchURLExampleView(_selectedCastingPlayer: self.selectedCastingPlayer),
                label: {
                    Text("ContentLauncher Launch URL")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()
            
            NavigationLink(
                destination: MCApplicationBasicReadVendorIDExampleView(_selectedCastingPlayer: self.selectedCastingPlayer),
                label: {
                    Text("ApplicationBasic Read VendorID")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()

            NavigationLink(
                destination: MCMediaPlaybackSubscribeToCurrentStateExampleView(_selectedCastingPlayer: self.selectedCastingPlayer),
                label: {
                    Text("MediaPlayback Subscribe to CurrentState")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()

        }
        .navigationTitle("Select an action")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
    }
}

struct MCActionSelectorView_Previews: PreviewProvider {
    static var previews: some View {
        MCActionSelectorView(_selectedCastingPlayer: nil)
    }
}
