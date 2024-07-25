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

@available(*, deprecated, message: "Refer to MCActionSelectorView")
struct ClusterSelectorView: View {
    var body: some View {
        VStack(alignment: .leading) {
            NavigationLink(
                destination: ContentLauncherView(),
                label: {
                    Text("Content Launcher")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()
            
            NavigationLink(
                destination: MediaPlayerView(),
                label: {
                    Text("Media Playback")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()

            NavigationLink(
                destination: CertTestView(),
                label: {
                    Text("Certification Tests")
                        .frame(width: 300, height: 30, alignment: .center)
                        .border(Color.black, width: 1)
                }
            ).background(Color.blue)
                .foregroundColor(Color.white)
                .padding()
        }
        .navigationTitle("Cluster selection")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
    }
}

struct ClusterSelectorView_Previews: PreviewProvider {
    static var previews: some View {
        ClusterSelectorView()
    }
}
