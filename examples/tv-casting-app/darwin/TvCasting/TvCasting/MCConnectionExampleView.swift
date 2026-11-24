/**
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
    var fixedPasscodeCommissioningPasscode: UInt32?
    var supportsFixedPasscodeCommissioning: Bool
    var regularSetupPasscode: UInt32
    var regularDiscriminator: UInt16
    
    @StateObject var viewModel = MCConnectionExampleViewModel();
    
    init(
        _selectedCastingPlayer: MCCastingPlayer?,
        _useCommissionerGeneratedPasscode: Bool,
        _fixedPasscodeCommissioningPasscode: UInt32?
    ) {
        let supportsFixedPasscodeCommissioning =
            MCConnectionExampleViewModel.canRequestFixedPasscodeCommissioning(
                selectedCastingPlayer: _selectedCastingPlayer,
                fixedPasscodeCommissioningPasscode: _fixedPasscodeCommissioningPasscode
            )
        let commissionableData =
            MCInitializationExample.shared.getAppParametersDataSource()?.getCommissionableData()

        self.selectedCastingPlayer = _selectedCastingPlayer
        self.useCommissionerGeneratedPasscode = _useCommissionerGeneratedPasscode
        self.fixedPasscodeCommissioningPasscode =
            supportsFixedPasscodeCommissioning ? _fixedPasscodeCommissioningPasscode : nil
        self.supportsFixedPasscodeCommissioning = supportsFixedPasscodeCommissioning
        self.regularSetupPasscode = commissionableData?.passcode ?? 0
        self.regularDiscriminator = commissionableData?.discriminator ?? 0
    }
    
    var body: some View {
        VStack(alignment: .leading) {
            if supportsFixedPasscodeCommissioning && self.useCommissionerGeneratedPasscode {
                Text("Requesting Fixed Passcode Commissioning from Casting Player with device name: \(self.selectedCastingPlayer!.deviceName())\n\nIf Fixed Passcode Commissioning is not selected, requesting CastingPlayer/Commissioner-Generated passcode commissioning.").padding()
            } else if self.useCommissionerGeneratedPasscode {
                if selectedCastingPlayer?.supportsCommissionerGeneratedPasscode() == true {
                    Text("Verifying or Establishing Connection, using Casting Player/Commissioner-Generated passcode, with Casting Player: \(self.selectedCastingPlayer!.deviceName())\n\nEnter the passcode displayed on the Casting Player when prompted.").padding()
                } else {
                    Text("\(self.selectedCastingPlayer!.deviceName()) does not support Casting Player/Commissioner-Generated passcode commissioning. \n\nSelect a different Casting Player.").padding()
                }
            } else if supportsFixedPasscodeCommissioning {
                Text("Verifying or establishing a connection with Fixed Passcode Commissioning support: \(self.selectedCastingPlayer!.deviceName())\n\nIf Fixed Passcode Commissioning is not selected, use this setup Passcode: \(regularSetupPasscode)\nDiscriminator: \(regularDiscriminator)").padding()
            } else {
                Text("Verifying or establishing connection with Casting Player with device name: \(self.selectedCastingPlayer!.deviceName())\nClient/Commissionee-Generated Setup Passcode: \(regularSetupPasscode)\nDiscriminator: \(regularDiscriminator)").padding()
            }
            if let connectionStatus = viewModel.connectionStatus
            {
                Text(connectionStatus).padding()
            }

            if viewModel.connectionSuccess == true
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
            if self.useCommissionerGeneratedPasscode && !supportsFixedPasscodeCommissioning {
                if selectedCastingPlayer?.supportsCommissionerGeneratedPasscode() == true {
                    self.Log.info("MCConnectionExampleView calling MCConnectionExampleViewModel.connect() with useCommissionerGeneratedPasscode: \(String(describing: self.useCommissionerGeneratedPasscode))")
                    viewModel.connect(
                        selectedCastingPlayer: self.selectedCastingPlayer,
                        useCommissionerGeneratedPasscode: self.useCommissionerGeneratedPasscode,
                        fixedPasscodeCommissioningPasscode: self.fixedPasscodeCommissioningPasscode
                    )
                } else {
                    self.Log.error("MCConnectionExampleView \(self.selectedCastingPlayer!.deviceName()) does not support Casting Player/Commissioner-Generated passcode commissioning. Select a different Casting Player.")
                }
            } else {
                self.Log.info("MCConnectionExampleView calling MCConnectionExampleViewModel.connect() with useCommissionerGeneratedPasscode: \(String(describing: self.useCommissionerGeneratedPasscode))")
                viewModel.connect(
                    selectedCastingPlayer: self.selectedCastingPlayer,
                    useCommissionerGeneratedPasscode: self.useCommissionerGeneratedPasscode,
                    fixedPasscodeCommissioningPasscode: self.fixedPasscodeCommissioningPasscode
                )
            }
        })
        .onDisappear(perform: {
            viewModel.cancelConnectionAttempt(selectedCastingPlayer: self.selectedCastingPlayer)
        })
        .alert("Fixed Passcode Commissioning Failed", isPresented: $viewModel.showFixedPasscodeAuthenticationFailure) {
            Button("OK", role: .cancel) {}
        } message: {
            Text("The stored fixed setup Passcode does not match this Casting Player. Update the Fixed Passcode or use another Passcode flow.")
        }
    }
}

struct MCConnectionExampleView_Previews: PreviewProvider {
    static var previews: some View {
        MCConnectionExampleView(
            _selectedCastingPlayer: nil,
            _useCommissionerGeneratedPasscode: false,
            _fixedPasscodeCommissioningPasscode: nil
        )
    }
}
