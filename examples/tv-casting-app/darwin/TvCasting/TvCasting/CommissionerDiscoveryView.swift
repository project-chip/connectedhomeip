/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import SwiftUI

extension DiscoveredNodeData : Identifiable {
    public var id: String {
        instanceName
    }
}

struct CommissionerDiscoveryView: View {
    @StateObject var viewModel = CommissionerDiscoveryViewModel()
    
    var body: some View {
        VStack(alignment: .leading) {
            NavigationLink(
                destination: CommissioningView(_selectedCommissioner: nil),
                label: {
                    Text("Skip to manual commissioning >>")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()
            
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
                                ConnectionView(_selectedVideoPlayer: commissioner.getConnectableVideoPlayer())
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
