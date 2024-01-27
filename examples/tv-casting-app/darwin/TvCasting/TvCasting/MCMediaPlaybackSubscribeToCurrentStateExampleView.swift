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

struct MCMediaPlaybackSubscribeToCurrentStateExampleView: View {
    @StateObject var viewModel = MCMediaPlaybackSubscribeToCurrentStateExampleViewModel()
    
    var selectedCastingPlayer: MCCastingPlayer?

    init(_selectedCastingPlayer: MCCastingPlayer?) {
        self.selectedCastingPlayer = _selectedCastingPlayer
    }
    
    var body: some View {
        VStack(alignment: .leading)
        {
            Button("Subscribe to CurrentState!") {
                viewModel.subscribe(castingPlayer: self.selectedCastingPlayer!)
            }
            .background(Color.blue)
            .foregroundColor(Color.white)
            .cornerRadius(4)
            .border(Color.black, width: 1)
            .padding()
                
            Text(viewModel.status ?? "")
        }
        .navigationTitle("MediaPlayback Subscribe to CurrentState")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
    }
}

struct MCMediaPlaybackSubscribeToCurrentStateExampleView_Previews: PreviewProvider {
    static var previews: some View {
        MCMediaPlaybackSubscribeToCurrentStateExampleView(_selectedCastingPlayer: nil)
    }
}
