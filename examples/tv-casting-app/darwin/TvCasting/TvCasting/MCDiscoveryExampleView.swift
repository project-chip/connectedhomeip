/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

extension MCCastingPlayer : Identifiable {
    public var id: String {
        identifier()
    }
}

struct MCDiscoveryExampleView: View {
    @StateObject var viewModel = MCDiscoveryExampleViewModel()
    @State private var selectedCastingPlayer: MCCastingPlayer?
    // Enable navigating to the MCConnectionExampleView with or without the use
    // Commissioner-Generated Passcode (CGP) flag.
    @State private var navigateWithUseCGP = false
    
    var body: some View {
        VStack(alignment: .center, spacing: 16) {
            Button("Start Discovery", action: viewModel.startDiscovery)
                .frame(width: 350, height: 30)
                .border(Color.black, width: 1)
                .background(Color.blue)
                .foregroundColor(Color.white)
                .padding(1)

            Button("Stop Discovery", action: viewModel.stopDiscovery)
                .frame(width: 350, height: 30)
                .border(Color.black, width: 1)
                .background(Color.blue)
                .foregroundColor(Color.white)
                .padding(1)

            Button("Clear Results", action: viewModel.clearResults)
                .frame(width: 350, height: 30)
                .border(Color.black, width: 1)
                .background(Color.blue)
                .foregroundColor(Color.white)
                .padding(1)

            if(viewModel.discoveryHasError)
            {
                Text("Discovery request failed. Check logs for details")
            }
            else if(!viewModel.displayedCastingPlayers.isEmpty)
            {
                Text("Select a Casting player:")
                ForEach(viewModel.displayedCastingPlayers) { castingPlayer in
                    ZStack {
                        NavigationLink(
                            destination: MCConnectionExampleView(
                                _selectedCastingPlayer: selectedCastingPlayer,
                                _useCommissionerGeneratedPasscode: navigateWithUseCGP
                            ),
                            isActive: Binding(
                                get: { selectedCastingPlayer?.id == castingPlayer.id },
                                set: { newValue in
                                    if newValue {
                                        selectedCastingPlayer = castingPlayer
                                        navigateWithUseCGP = false
                                    } else {
                                        selectedCastingPlayer = nil
                                        navigateWithUseCGP = false
                                    }
                                }
                            ),
                            label: { EmptyView() }
                        )
                        Text(castingPlayer.description)
                            .frame(minHeight: 50)
                            .multilineTextAlignment(.center)
                            .padding(10)
                            .background(Color.blue)
                            .foregroundColor(Color.white)
                            .border(Color.black, width: 1)
                            .onTapGesture {
                                selectedCastingPlayer = castingPlayer
                                navigateWithUseCGP = false
                            }
                            .gesture(
                                LongPressGesture()
                                    .onEnded { _ in
                                        selectedCastingPlayer = castingPlayer
                                        navigateWithUseCGP = true
                                    }
                            )
                    }
                    .frame(width: 350)
                    .padding(1)
                }
            }
            Spacer()
            Text("Long click on a Casting Player to connect using CastingPlayer/Commissioner-Generated passcode commissioning flow (if supported).").font(.system(size: 12)).padding(1)
        }
        .navigationTitle("Casting Player Discovery")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .multilineTextAlignment(.center)
    }        
}

struct MCDiscoveryExampleView_Previews: PreviewProvider {
    static var previews: some View {
        MCDiscoveryExampleView()
    }
}
