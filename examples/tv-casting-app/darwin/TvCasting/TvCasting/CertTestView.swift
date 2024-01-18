
/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

struct CertTestView: View {
    @StateObject var viewModel = CertTestViewModel()
    @State private var targetContentAppId: String = ""
    @State private var parallelizeTests = true

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
                    Toggle(isOn: $parallelizeTests) {
                        Text("Parallelize Tests")
                    }
                }

                Button("Launch Test") {
                    viewModel.launchTests(
                        targetContentAppId: targetContentAppId,
                        inParallel: parallelizeTests
                    )
                }
                .background(Color.blue)
                .foregroundColor(Color.white)
                .cornerRadius(4)
                .border(Color.black, width: 1)
                .padding()

                Text(viewModel.status ?? "")
            }

        }
        .navigationTitle("Cert Test Launcher")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            viewModel.populateAndInitializeEndpoints()
        })
    }
}
