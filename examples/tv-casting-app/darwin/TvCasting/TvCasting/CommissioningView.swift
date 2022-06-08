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


import SwiftUI

struct CommissioningView: View {
    var selectedCommissioner: DiscoveredNodeData?
    
    @StateObject var viewModel = CommissioningViewModel();
    
    init(_selectedCommissioner: DiscoveredNodeData?) {
        self.selectedCommissioner = _selectedCommissioner
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            if(viewModel.commisisoningWindowOpened) {
                Text("Commissioning window opened.")
                
                if(viewModel.udcRequestSent) {
                    Text("Complete commissioning on " + (selectedCommissioner?.deviceName)!)
                }
                else{
                    Text("Complete commissioning with a commissioner manually")
                }
                
                // TBD: actual values
                Text("Onboarding PIN: ")
                Text("Discriminator: ")
            }
            else {
                Text("Failed to open Commissioning window!")
            }
        }
        .navigationTitle("Commissioning...")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            viewModel.prepareForCommissioning(selectedCommissioner: self.selectedCommissioner)
        })
    }
}

struct CommissioningView_Previews: PreviewProvider {
    static var previews: some View {
        CommissioningView(_selectedCommissioner: nil)
    }
}
