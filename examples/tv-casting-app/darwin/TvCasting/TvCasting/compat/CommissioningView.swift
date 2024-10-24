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


import SwiftUI

@available(*, deprecated, message: "Refer to MCConnectionExampleView")
struct CommissioningView: View {
    var selectedCommissioner: DiscoveredNodeData?
    
    @StateObject var viewModel = CommissioningViewModel();
    
    init(_selectedCommissioner: DiscoveredNodeData?) {
        self.selectedCommissioner = _selectedCommissioner
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            if(viewModel.commisisoningWindowOpened == true) {
                Text("Onboarding payload for Commissioning if needed:").padding()
                
                Text("Passcode: " + String((CastingServerBridge.getSharedInstance()?.getOnboardingPayload().setupPasscode)!))
                    .border(Color.blue, width: 1)
                    .padding()
                Text("Discriminator: " + String((CastingServerBridge.getSharedInstance()?.getOnboardingPayload().setupDiscriminator)!))
                    .border(Color.blue, width: 1)
                    .padding()

                if(viewModel.udcRequestSent == true)
                {
                    Text("Complete commissioning on " + (selectedCommissioner?.deviceName)!)
                        .padding()
                }
                else if(viewModel.udcRequestSent == false) {
                    Text("Could not send user directed commissioning request to " + (selectedCommissioner?.deviceName)!)
                        .foregroundColor(Color.red)
                        .padding()
                }
            }
            else if(viewModel.commisisoningWindowOpened == false) {
                Text("Failed to open Commissioning window!")
                    .foregroundColor(Color.red)
                    .padding()
            }
            
            if(viewModel.commisisoningComplete == true)
            {
                Text("Commissioning finished!").padding()
                
                if let connectionSuccess = viewModel.connectionSuccess
                {
                    if let connectionStatus = viewModel.connectionStatus
                    {
                        Text(connectionStatus).padding()
                    }
                    
                    if(connectionSuccess)
                    {
                        NavigationLink(
                            destination: ClusterSelectorView(),
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
            else if(viewModel.commisisoningComplete == false)
            {
                Text("Commissioning failure!")
                    .foregroundColor(Color.red)
                    .padding()
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
