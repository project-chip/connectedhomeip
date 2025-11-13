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
import os.log

struct MCConnectionExampleView: View {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MCConnectionExampleView")
    var selectedCastingPlayer: MCCastingPlayer?
    var useCommissionerGeneratedPasscode: Bool
    
    @StateObject var viewModel = MCConnectionExampleViewModel();
    
    init(_selectedCastingPlayer: MCCastingPlayer?, _useCommissionerGeneratedPasscode: Bool) {
        self.selectedCastingPlayer = _selectedCastingPlayer
        self.useCommissionerGeneratedPasscode = _useCommissionerGeneratedPasscode
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            if self.useCommissionerGeneratedPasscode {
                if selectedCastingPlayer?.supportsCommissionerGeneratedPasscode() == true {
                    Text("Verifying or Establishing Connection, using Casting Player/Commissioner-Generated passcode, with Casting Player: \(self.selectedCastingPlayer!.deviceName())\n\nEnter the passcode displayed on the Casting Player when prompted.").padding()
                } else {
                    Text("\(self.selectedCastingPlayer!.deviceName()) does not support Casting Player/Commissioner-Generated passcode commissioning. \n\nSelect a different Casting Player.").padding()
                }
            } else {
                Text("Verifying or Establishing Connection to Casting Player: \(self.selectedCastingPlayer!.deviceName())").padding()
            }
            if let connectionSuccess = viewModel.connectionSuccess
            {
                if let connectionStatus = viewModel.connectionStatus
                {
                    Text(connectionStatus).padding()
                }
                
                if(connectionSuccess)
                {
                    NavigationLink(
                        destination: MCActionSelectorView(_selectedCastingPlayer: self.selectedCastingPlayer), 
                        label: {
                            Text("Next")
                                .frame(width: 100, height: 30, alignment: .center)
                                .border(Color.black, width: 1)
                        }
                    ).background(Color.blue)
                        .foregroundColor(Color.white)
                        .frame(maxHeight: .infinity, alignment: .bottom)
                        .padding()
                }
            }
            Spacer()  // Push the error code to the bottom
            if let errorCode = viewModel.errorCodeDescription {

                Text("\(errorCode)")
                    .foregroundColor(.red)
                    .padding()
            }
        }
        .navigationTitle("Connecting...")
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .top)
        .onAppear(perform: {
            if self.useCommissionerGeneratedPasscode {
                if selectedCastingPlayer?.supportsCommissionerGeneratedPasscode() == true {
                    self.Log.info("MCConnectionExampleView calling MCConnectionExampleViewModel.connect() with useCommissionerGeneratedPasscode: \(String(describing: self.useCommissionerGeneratedPasscode))")
                    viewModel.connect(selectedCastingPlayer: self.selectedCastingPlayer, useCommissionerGeneratedPasscode: self.useCommissionerGeneratedPasscode)
                } else {
                    self.Log.error("MCConnectionExampleView \(self.selectedCastingPlayer!.deviceName()) does not support Casting Player/Commissioner-Generated passcode commissioning. Select a different Casting Player.")
                }
            } else {
                self.Log.info("MCConnectionExampleView calling MCConnectionExampleViewModel.connect() with useCommissionerGeneratedPasscode: \(String(describing: self.useCommissionerGeneratedPasscode))")
                viewModel.connect(selectedCastingPlayer: self.selectedCastingPlayer, useCommissionerGeneratedPasscode: self.useCommissionerGeneratedPasscode)
            }
        })
        .onDisappear(perform: {
            viewModel.cancelConnectionAttempt(selectedCastingPlayer: self.selectedCastingPlayer)
        })
    }
}

struct MCConnectionExampleView_Previews: PreviewProvider {
    static var previews: some View {
        MCConnectionExampleView(_selectedCastingPlayer: nil, _useCommissionerGeneratedPasscode: false)
    }
}
