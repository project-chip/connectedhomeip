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

extension DiscoveredNodeData : Identifiable {
    public var id: String {
        instanceName
    }
}

@available(*, deprecated, message: "Refer to MCDiscoveryExampleView")
struct CommissionerDiscoveryView: View {
    @StateObject var viewModel = CommissionerDiscoveryViewModel()
    
    var body: some View {
        VStack(alignment: .leading) {
            Button("Discover commissioners", action: viewModel.discoverAndUpdate)
                .frame(width: 200, height: 30, alignment: .center)
                .border(Color.black, width: 1)
                .background(Color.blue)
                .foregroundColor(Color.white)
                .padding()
            
            if(viewModel.discoveryRequestStatus == false)
            {
                Text("Failed to send discovery request")
            }
            else if(!viewModel.commissioners.isEmpty)
            {
                Text("Select a commissioner video player...")
                ForEach(viewModel.commissioners) { commissioner in
                    NavigationLink(
                        destination: {
                            if(commissioner.isPreCommissioned())
                            {
                                Text("Error: unexpected value for commissioner.isPreCommissioned: true")
                            }
                            else
                            {
                                CommissioningView(_selectedCommissioner: commissioner)
                            }
                        },
                        label: {
                            Text(commissioner.description)
                        }
                    )
                    .frame(width: 350, height: 50, alignment: .center)
                    .border(Color.black, width: 1)
                    .background(Color.blue)
                    .foregroundColor(Color.white)
                    .padding(1)
                }
            }
        }
        .navigationTitle("Video Player Discovery")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
    }
}

struct CommissionerDiscoveryView_Previews: PreviewProvider {
    static var previews: some View {
        CommissionerDiscoveryView()
    }
}
