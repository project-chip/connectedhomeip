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

struct ContentLauncherView: View {
    @StateObject var viewModel = ContentLauncherViewModel()

    @State private var contentUrl: String = ""
    @State private var contentDisplayStr: String = ""
    
    var body: some View {
        VStack(alignment: .leading) {
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
                viewModel.launchUrl(contentUrl: contentUrl, contentDisplayStr: contentDisplayStr)
            }
            .background(Color.blue)
            .foregroundColor(Color.white)
            .cornerRadius(4)
            .border(Color.black, width: 1)
            .padding()
            
            Text(viewModel.status ?? "")
        }
        .navigationTitle("Content Launcher")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
    }
}

struct ContentLauncherView_Previews: PreviewProvider {
    static var previews: some View {
        ContentLauncherView()
    }
}
