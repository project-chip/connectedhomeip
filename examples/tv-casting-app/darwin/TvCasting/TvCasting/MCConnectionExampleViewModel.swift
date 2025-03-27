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

import Foundation
import os.log
import UIKit

class MCConnectionExampleViewModel: ObservableObject {
    let Log = Logger(subsystem: "com.matter.casting",
                     category: "MCConnectionExampleViewModel")

    var passcodeAlertController: UIAlertController?
    
    // VendorId of the MCEndpoint on the MCCastingPlayer that the MCCastingApp desires to interact with after connection
    let kDesiredEndpointVendorId: UInt16 = 65521;

    // VendorId of the MCEndpoint on the MCCastingPlayer that the MCCastingApp desires to interact with after connecting
    // using the MCCastingPlayer/Commissioner-Generated passcode (CGP) commissioning flow.  Use this Target Content
    // Application Vendor ID, which is configured on the tv-app. This Target Content Application Vendor ID (1111), does
    // not implement the AccountLogin cluster, which would otherwise auto commission using the Commissionee-Generated
    // passcode upon recieving the IdentificationDeclaration Message. See
    // connectedhomeip/examples/tv-app/tv-common/include/AppTv.h.
    let kDesiredEndpointVendorIdCGP: UInt16 = 1111;
    
    @Published var connectionSuccess: Bool?;

    @Published var connectionStatus: String?;

    @Published var errorCodeDescription: String?

    func cancelConnectionAttempt(selectedCastingPlayer: MCCastingPlayer?) {
        DispatchQueue.main.async {
            // Only stop connection if we are pending passcode confirmation
            var connectionState = MC_CASTING_PLAYER_NOT_CONNECTED;
            let err = selectedCastingPlayer?.getConnectionState(&connectionState)
            if err != nil {
                self.Log.error("MCConnectionExampleViewModel cancelConnect() MCCastingPlayer.getConnectionState() failed due to: \(err)")
            }

            if connectionState == MC_CASTING_PLAYER_CONNECTING {
                self.Log.info("MCConnectionExampleViewModel cancelConnect(). User navigating back from ConnectionView")
                let err = selectedCastingPlayer?.stopConnecting()
                if err == nil {
                    self.connectionStatus = "User cancelled the connection attempt with CastingPlayer.stopConnecting()."
                    self.Log.info("MCConnectionExampleViewModel cancelConnect() MCCastingPlayer.stopConnecting() succeeded.")
                } else {
                    self.connectionStatus = "Cancel connection failed due to: \(String(describing: err))."
                    self.Log.error("MCConnectionExampleViewModel cancelConnect() MCCastingPlayer.stopConnecting() failed due to: \(err)")
                }
            }
        }
    }

    func connect(selectedCastingPlayer: MCCastingPlayer?, useCommissionerGeneratedPasscode: Bool) {
        self.Log.info("MCConnectionExampleViewModel.connect() useCommissionerGeneratedPasscode: \(String(describing: useCommissionerGeneratedPasscode))")

        let connectionCompleteCallback: (Swift.Error?) -> Void = { err in
            self.Log.error("MCConnectionExampleViewModel connect() completed with: \(err)")
            DispatchQueue.main.async {
                if err == nil {
                    self.connectionSuccess = true
                    if useCommissionerGeneratedPasscode {
                        self.connectionStatus = "Successfully connected to Casting Player using the Casting Player/Commissioner-Generated passcode!"
                        self.Log.info("MCConnectionExampleViewModel connect() Successfully connected to Casting Player using the Casting Player/CommissioneR-Generated passcode!")
                    } else {
                        self.connectionStatus = "Successfully connected to Casting Player!"
                        self.Log.info("MCConnectionExampleViewModel connect() Successfully connected to Casting Player using the Casting App/CommissioneE-Generated passcode!")
                    }
                } else {
                    self.connectionSuccess = false
                    self.connectionStatus = "Connection to Casting Player failed with: \(String(describing: err))"
                }
            }
        }

        let commissionerDeclarationCallback: (MCCommissionerDeclaration) -> Void = { commissionerDeclarationMessage in
            DispatchQueue.main.async {
                self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, recived a message form the MCCastingPlayer:\n\(commissionerDeclarationMessage)")

                // Display CommissionerDeclaration error code if `errorCode` is not `kNoError`
                if commissionerDeclarationMessage.errorCode != CdError.noError {
                    self.errorCodeDescription = "CommissionerDeclaration error from CastingPlayer: \(commissionerDeclarationMessage.getErrorCodeString())"
                    self.Log.error("MCConnectionExampleViewModel connect() Casting Player/Commissioner Error: \(self.errorCodeDescription ?? "Unknown Error")")
                }

                // Check if the passcode dialog should be cancelled
                if commissionerDeclarationMessage.cancelPasscode {
                    self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback. Cancel passcode received. Dismissing the PasscodeInputDialog.")
                    self.passcodeAlertController?.dismiss(animated: true, completion: nil)
                    self.connectionStatus = "Connection attempt cancelled by the CastingPlayer/Commissioner user. \n\nRoute back to exit."
                    return
                }

                // Continue with passcode dialog if needed
                if commissionerDeclarationMessage.commissionerPasscode {
                    self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, calling getTopMostViewController()")
                    if let topViewController = self.getTopMostViewController() {
                        self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, calling displayPasscodeInputDialog()")
                        self.displayPasscodeInputDialog(on: topViewController, continueConnecting: { userEnteredPasscode in
                            self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, Continuing to connect with user entered MCCastingPlayer/Commissioner-Generated passcode: \(userEnteredPasscode)")

                            // Update the CommissionableData in the client defined MCAppParametersDataSource with the user
                            // entered CastingPlayer/Commissioner-Generated setup passcode. This is mandatory for the
                            // Commissioner-Generated passcode commissioning flow since the commissioning session's PAKE
                            // verifier needs to be updated with the entered passcode. Get the singleton instane of the
                            // MCInitializationExample.
                            let initializationExample = MCInitializationExample.shared
                            self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback calling MCInitializationExample.getAppParametersDataSource()")
                            if let dataSource = initializationExample.getAppParametersDataSource() {
                                let newCommissionableData = MCCommissionableData(
                                    passcode: UInt32(userEnteredPasscode) ?? 0,
                                    discriminator: 0,
                                    spake2pIterationCount: 1000,
                                    spake2pVerifier: nil,
                                    spake2pSalt: nil
                                )
                                dataSource.update(newCommissionableData)
                                self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, Updated MCAppParametersDataSource instance with new MCCommissionableData.")
                            } else {
                                self.Log.error("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, InitializationExample.getAppParametersDataSource() failed")
                                self.connectionStatus = "Failed to update the MCAppParametersDataSource with the user entered passcode: \n\nRoute back to disconnect and try again."
                                self.connectionSuccess = false
                                return
                            }

                            self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, calling MCCastingPlayer.continueConnecting()")
                            let errContinue = selectedCastingPlayer?.continueConnecting()
                            if errContinue == nil {
                                self.connectionStatus = "Continuing to connect with user entered passcode: \(userEnteredPasscode)"
                            } else {
                                self.connectionStatus = "Continue Connecting to Casting Player failed with: \(String(describing: errContinue)) \n\nRoute back to disconnect and try again."
                                self.Log.error("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, MCCastingPlayer.continueConnecting() failed due to: \(errContinue)")
                            }
                        }, cancelConnecting: {
                            self.Log.info("MCConnectionExampleViewModel connect() commissionerDeclarationCallback, Connection attempt cancelled by the user")
                            self.connectionSuccess = false
                            self.connectionStatus = "User cancelled the connection attempt with CastingPlayer. \n\nRoute back to disconnect & exit."
                        })
                    }
                }
            }
        }

        let identificationDeclarationOptions: MCIdentificationDeclarationOptions
        let targetAppInfo: MCTargetAppInfo
        let connectionCallbacks: MCConnectionCallbacks

        if useCommissionerGeneratedPasscode {
            identificationDeclarationOptions = MCIdentificationDeclarationOptions(commissionerPasscodeOnly: true)
            targetAppInfo = MCTargetAppInfo(vendorId: kDesiredEndpointVendorIdCGP)
            connectionCallbacks = MCConnectionCallbacks(
                callbacks: connectionCompleteCallback,
                commissionerDeclarationCallback: commissionerDeclarationCallback
            )
        } else {
            identificationDeclarationOptions = MCIdentificationDeclarationOptions()
            targetAppInfo = MCTargetAppInfo(vendorId: kDesiredEndpointVendorId)
            connectionCallbacks = MCConnectionCallbacks(
                callbacks: connectionCompleteCallback,
                commissionerDeclarationCallback: commissionerDeclarationCallback
            )
        }

        identificationDeclarationOptions.addTargetAppInfo(targetAppInfo)
        self.Log.info("MCConnectionExampleViewModel.connect() MCIdentificationDeclarationOptions description: \n\(identificationDeclarationOptions.description)")

        self.Log.info("MCConnectionExampleViewModel.connect() calling MCCastingPlayer.verifyOrEstablishConnection()")
        let err = selectedCastingPlayer?.verifyOrEstablishConnection(with: connectionCallbacks, identificationDeclarationOptions: identificationDeclarationOptions)
        if err != nil {
            self.Log.error("MCConnectionExampleViewModel connect(), MCCastingPlayer.verifyOrEstablishConnection() failed due to: \(err)")
        }
    }

    // Function to display the passcode input dialog
    func displayPasscodeInputDialog(on viewController: UIViewController, continueConnecting: @escaping (String) -> Void, cancelConnecting: @escaping () -> Void) {
        self.Log.info("MCConnectionExampleViewModel displayPasscodeInputDialog()")

        // Create the alert controller
        let alertController = UIAlertController(title: "Enter Passcode", message: nil, preferredStyle: .alert)
        self.passcodeAlertController = alertController

        // Add the text field with the default passcode
        alertController.addTextField { textField in
            textField.placeholder = "Enter Passcode"
            textField.text = "12345678"
            // textField.isSecureTextEntry = true // Makes the passcode invisible
        }

        // Add the "Continue Connecting" button
        let continueAction = UIAlertAction(title: "Continue Connecting", style: .default) { _ in
            if let passcode = alertController.textFields?.first?.text {
                self.Log.info("MCConnectionExampleViewModel displayPasscodeInputDialog() User entered passcode: \(passcode)")
                continueConnecting(passcode)
            }
        }
        alertController.addAction(continueAction)

        // Add the "Cancel" button
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel) { _ in
            self.Log.info("MCConnectionExampleViewModel displayPasscodeInputDialog() User cancelled the passcode input dialog.")
            cancelConnecting()
        }
        alertController.addAction(cancelAction)
        
        // Present the alert controller
        viewController.present(alertController, animated: true, completion: nil)
    }

    // Function to get the top-most view controller
    func getTopMostViewController() -> UIViewController? {
        self.Log.info("MCConnectionExampleViewModel getTopMostViewController()")
        guard let windowScene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
            let window = windowScene.windows.first(where: { $0.isKeyWindow }) else {
            return nil
        }

        var topController = window.rootViewController
        while let presentedController = topController?.presentedViewController {
            topController = presentedController
        }

        return topController
    }
}
