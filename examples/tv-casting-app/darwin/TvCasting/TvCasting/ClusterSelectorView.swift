/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import SwiftUI

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
