/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

struct MCConnectionExampleView: View {
    var selectedCastingPlayer: MCCastingPlayer?
    
    @StateObject var viewModel = MCConnectionExampleViewModel();
    
    init(_selectedCastingPlayer: MCCastingPlayer?) {
        self.selectedCastingPlayer = _selectedCastingPlayer
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            Text("Verifying or Establishing Connection to Casting Player: \(self.selectedCastingPlayer!.deviceName())").padding()
            if let connectionSuccess = viewModel.connectionSuccess
            {
                if let connectionStatus = viewModel.connectionStatus
                {
                    Text(connectionStatus).padding()
                }
                
                if(connectionSuccess)
                {
                    NavigationLink(
                        destination: MCActionSelectorView(_selectedCastingPlayer: self.selectedCastingPlayer), 
                        label: {
                            Text("Next")
                                .frame(width: 100, height: 30, alignment: .center)
                                .border(Color.black, width: 1)
                        }
                    ).background(Color.blue)
                        .foregroundColor(Color.white)
                        .frame(maxHeight: .infinity, alignment: .bottom)
                        .padding()
                }
            }
        }
        .navigationTitle("Connecting...")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            viewModel.connect(selectedCastingPlayer: self.selectedCastingPlayer)
        })
    }
}

struct MCConnectionExampleView_Previews: PreviewProvider {
    static var previews: some View {
        MCConnectionExampleView(_selectedCastingPlayer: nil)
    }
}
