/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
