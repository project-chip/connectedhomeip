/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import SwiftUI

struct MediaPlayerView: View {
    @StateObject var viewModel = MediaPlaybackViewModel()

    @State private var minIntervalStr: String = ""
    @State private var maxIntervalStr: String = ""
    @State private var targetContentAppId: String = ""
    
    var body: some View {
        VStack(alignment: .leading)
        {
            if(viewModel.contentAppIds.isEmpty)
            {
                Text("No Media Playback cluster supporting content apps found on this video player!")
            }
            else
            {
                Text("Subscribe to Current State")
                
                HStack() {
                    Text("Content App Endpoint Id")
                    
                    VStack()
                    {
                        Picker("Select", selection: $targetContentAppId) {
                            Text("Select").tag(nil as String?)
                            ForEach(viewModel.contentAppIds, id: \.self) { contentAppId in
                                Text(String(contentAppId))
                            }
                        }
                        .pickerStyle(.menu)
                        .padding(2)
                    }
                    .border(.secondary)
                }
                
                HStack() {
                    Text("Min Interval")
                    
                    TextField(
                        "0",
                        text: $minIntervalStr
                    )
                    .textInputAutocapitalization(.never)
                    .disableAutocorrection(true)
                    .border(.secondary)
                }
                
                HStack() {
                    Text("Max Interval")
                    
                    TextField(
                        "1",
                        text: $maxIntervalStr
                    )
                    .textInputAutocapitalization(.never)
                    .disableAutocorrection(true)
                    .border(.secondary)
                }
                
                Button("Subscribe!") {
                    viewModel.subscribeCurrentState(targetContentAppId: targetContentAppId, minInterval: minIntervalStr, maxInterval: maxIntervalStr)
                }
                .background(Color.blue)
                .foregroundColor(Color.white)
                .cornerRadius(4)
                .border(Color.black, width: 1)
                .padding()
                
                Text(viewModel.requestStatus ?? "")
                Text(viewModel.subscriptionStatus ?? "")
                Text(viewModel.readResponse ?? "")
            }
        }
        .navigationTitle("Media Playback")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            viewModel.populateContentApps()
        })
    }
}

struct MediaPlayerView_Previews: PreviewProvider {
    static var previews: some View {
        MediaPlayerView()
    }
}
