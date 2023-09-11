
/**
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
