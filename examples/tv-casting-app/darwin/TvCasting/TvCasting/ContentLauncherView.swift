/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import SwiftUI

extension ContentApp : Identifiable {
    public var id: UInt16 {
        endpointId
    }
}

struct ContentLauncherView: View {
    @StateObject var viewModel = ContentLauncherViewModel()

    @State private var contentUrl: String = ""
    @State private var contentDisplayStr: String = ""
    @State private var targetContentAppId: String = ""
    
    var body: some View {
        VStack(alignment: .leading) {
            if(viewModel.contentAppIds.isEmpty)
            {
                Text("No Content Launcher cluster supporting content apps found on this video player!")
            }
            else
            {
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
                    Text("Content URL")
                
                    TextField(
                        "https://www.test.com/videoid",
                        text: $contentUrl
                    )
                    .textInputAutocapitalization(.never)
                    .disableAutocorrection(true)
                    .border(.secondary)
                }
            
                HStack() {
                    Text("Display string")
                
                    TextField(
                        "Test video",
                        text: $contentDisplayStr
                    )
                    .textInputAutocapitalization(.never)
                    .disableAutocorrection(true)
                    .border(.secondary)
                }
            
                Button("Launch URL!") {
                    viewModel.launchUrl(targetContentAppId: targetContentAppId, contentUrl: contentUrl, contentDisplayStr: contentDisplayStr)
                }
                .background(Color.blue)
                .foregroundColor(Color.white)
                .cornerRadius(4)
                .border(Color.black, width: 1)
                .padding()
            
                Text(viewModel.status ?? "")
            }

        }
        .navigationTitle("Content Launcher")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            viewModel.populateContentApps()
        })
    }
}

struct ContentLauncherView_Previews: PreviewProvider {
    static var previews: some View {
        ContentLauncherView()
    }
}
