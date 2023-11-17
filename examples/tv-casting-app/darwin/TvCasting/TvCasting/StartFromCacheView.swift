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

extension VideoPlayer : Identifiable {
    public var id: String {
        deviceName
    }
}

struct StartFromCacheView: View {
    @StateObject var viewModel = StartFromCacheViewModel()
    
    var body: some View {
        VStack(alignment: .leading) {
            Button("Purge cache", action: {
                viewModel.purgeAndReReadCache()
            })
            .frame(width: 200, height: 30, alignment: .center)
            .border(Color.black, width: 1)
            .background(Color.blue)
            .foregroundColor(Color.white)
            .padding()
            
            NavigationLink(
                destination: CommissionerDiscoveryView(),
                label: {
                    Text("Skip to commissioner discovery >>")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()
            
            if(viewModel.videoPlayers.isEmpty)
            {
                Text("No cached video players.")
            }
            else
            {
                Text("Pick a Video player")
                ForEach(viewModel.videoPlayers) { videoPlayer in
                    NavigationLink(
                        destination: ConnectionView(_selectedVideoPlayer: videoPlayer),
                        label: {
                            Text(videoPlayer.description)
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
        .navigationTitle("Starting from Cache")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {viewModel.readFromCache()})
    }
}

struct StartFromCacheView_Previews: PreviewProvider {
    static var previews: some View {
        StartFromCacheView()
    }
}
