/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */


import SwiftUI

struct ConnectionView: View {
    var selectedVideoPlayer: VideoPlayer?
    
    @StateObject var viewModel = ConnectionViewModel();
    
    init(_selectedVideoPlayer: VideoPlayer?) {
        self.selectedVideoPlayer = _selectedVideoPlayer
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            if let requestSent = viewModel.requestSent {
                if(requestSent)
                {
                    Text("Sent request to verify or connect to video player").padding()
                }
                else
                {
                    Text("Failed in sending request to verify or connect to video player!").foregroundColor(Color.red).padding()
                }

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
        }
        .navigationTitle("Connecting...")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            viewModel.verifyOrEstablishConnection(selectedVideoPlayer: self.selectedVideoPlayer)
        })
    }
}

struct ConnectionView_Previews: PreviewProvider {
    static var previews: some View {
        ConnectionView(_selectedVideoPlayer: nil)
    }
}
