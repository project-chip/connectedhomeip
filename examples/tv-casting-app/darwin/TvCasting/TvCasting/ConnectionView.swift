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
