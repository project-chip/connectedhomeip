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

extension ContentApp : Identifiable {
    public var id: UInt16 {
        endpointId
    }
}

@available(*, deprecated, message: "Refer to MCContentLauncherLaunchURLExampleView")
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
