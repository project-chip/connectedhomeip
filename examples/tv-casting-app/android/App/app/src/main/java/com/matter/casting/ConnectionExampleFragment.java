/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
package com.matter.casting;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.R;
import com.matter.casting.core.CastingApp;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.support.CommissionerDeclaration;
import com.matter.casting.support.ConnectionCallbacks;
import com.matter.casting.support.DataProvider;
import com.matter.casting.support.IdentificationDeclarationOptions;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;
import com.matter.casting.support.TargetAppInfo;
import java.util.concurrent.Executors;

/** A {@link Fragment} to Verify or establish a connection with a selected Casting Player. */
public class ConnectionExampleFragment extends Fragment {
  private static final String TAG = ConnectionExampleFragment.class.getSimpleName();
  // Time (in sec) to keep the commissioning window open, if commissioning is required.
  // Must be >= 3 minutes.
  private static final short MIN_CONNECTION_TIMEOUT_SEC = 3 * 60;
  private static final Integer DESIRED_TARGET_APP_VENDOR_ID = 65521;
  // Use this Target Content Application Vendor ID, configured on the tv-app, to demonstrate the
  // Commissioner-Generated passcode commissioning flow.
  private static final Integer DESIRED_TARGET_APP_VENDOR_ID_FOR_CGP_FLOW = 1111;
  private static final String DEFAULT_COMMISSIONER_GENERATED_PASSCODE = "12345678";
  private static final int DEFAULT_DISCRIMINATOR_FOR_CGP_FLOW = 0;
  private final CastingPlayer targetCastingPlayer;
  private final Boolean useCommissionerGeneratedPasscode;
  private TextView connectionFragmentStatusTextView;
  private Button connectionFragmentNextButton;

  public ConnectionExampleFragment(
      CastingPlayer targetCastingPlayer, Boolean useCommissionerGeneratedPasscode) {
    Log.i(
        TAG,
        "ConnectionExampleFragment() Target CastingPlayer ID: "
            + targetCastingPlayer.getDeviceId()
            + ", useCommissionerGeneratedPasscode: "
            + useCommissionerGeneratedPasscode);
    this.targetCastingPlayer = targetCastingPlayer;
    this.useCommissionerGeneratedPasscode = useCommissionerGeneratedPasscode;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment ConnectionExampleFragment.
   */
  public static ConnectionExampleFragment newInstance(
      CastingPlayer castingPlayer, Boolean useCommissionerGeneratedPasscode) {
    Log.i(TAG, "newInstance()");
    return new ConnectionExampleFragment(castingPlayer, useCommissionerGeneratedPasscode);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Log.i(TAG, "onCreate()");
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    Log.i(TAG, "onCreateView()");
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_matter_connection_example, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.i(TAG, "onViewCreated()");

    connectionFragmentStatusTextView = getView().findViewById(R.id.connectionFragmentStatusText);
    if (useCommissionerGeneratedPasscode) {
      connectionFragmentStatusTextView.setText(
          "Verifying or establishing connection with Casting Player with device name: "
              + targetCastingPlayer.getDeviceName()
              + "\n\nAttempting Commissioner-Generated passcode commissioning.");
    } else {
      connectionFragmentStatusTextView.setText(
          "Verifying or establishing connection with Casting Player with device name: "
              + targetCastingPlayer.getDeviceName()
              + "\nCommissionee-Generated Setup Passcode: "
              + InitializationExample.commissionableDataProvider.get().getSetupPasscode()
              + "\nDiscriminator: "
              + InitializationExample.commissionableDataProvider.get().getDiscriminator());
    }

    connectionFragmentNextButton = getView().findViewById(R.id.connectionFragmentNextButton);
    Callback callback = (ConnectionExampleFragment.Callback) this.getActivity();
    connectionFragmentNextButton.setOnClickListener(
        v -> {
          Log.i(TAG, "onViewCreated() NEXT clicked. Calling handleConnectionComplete()");
          callback.handleConnectionComplete(targetCastingPlayer, useCommissionerGeneratedPasscode);
        });

    Executors.newSingleThreadExecutor()
        .submit(
            () -> {
              Log.d(TAG, "onViewCreated() calling CastingPlayer.verifyOrEstablishConnection()");

              IdentificationDeclarationOptions idOptions = new IdentificationDeclarationOptions();
              TargetAppInfo targetAppInfo = new TargetAppInfo();
              targetAppInfo.vendorId = DESIRED_TARGET_APP_VENDOR_ID;

              if (useCommissionerGeneratedPasscode) {
                idOptions.mCommissionerPasscode = true;
                targetAppInfo.vendorId = DESIRED_TARGET_APP_VENDOR_ID_FOR_CGP_FLOW;
                Log.d(
                    TAG,
                    "onViewCreated() calling CastingPlayer.verifyOrEstablishConnection() Target Content Application Vendor ID: "
                        + targetAppInfo.vendorId
                        + ", useCommissionerGeneratedPasscode: "
                        + useCommissionerGeneratedPasscode);
              } else {
                Log.d(
                    TAG,
                    "onViewCreated() calling CastingPlayer.verifyOrEstablishConnection() Target Content Application Vendor ID: "
                        + targetAppInfo.vendorId);
              }

              idOptions.addTargetAppInfo(targetAppInfo);

              ConnectionCallbacks connectionCallbacks =
                  new ConnectionCallbacks(
                      new MatterCallback<Void>() {
                        @Override
                        public void handle(Void v) {
                          Log.i(
                              TAG,
                              "Successfully connected to CastingPlayer with deviceId: "
                                  + targetCastingPlayer.getDeviceId());
                          getActivity()
                              .runOnUiThread(
                                  () -> {
                                    connectionFragmentStatusTextView.setText(
                                        "Successfully connected to Casting Player with device name: "
                                            + targetCastingPlayer.getDeviceName()
                                            + "\n\n");
                                    connectionFragmentNextButton.setEnabled(true);
                                  });
                        }
                      },
                      new MatterCallback<MatterError>() {
                        @Override
                        public void handle(MatterError err) {
                          Log.e(TAG, "CastingPlayer connection failed: " + err);
                          getActivity()
                              .runOnUiThread(
                                  () -> {
                                    connectionFragmentStatusTextView.setText(
                                        "Casting Player connection failed due to: " + err + "\n\n");
                                  });
                        }
                      },
                      null);

              // CommissionerDeclaration is only needed for the Commissioner-Generated passcode
              // commissioning flow.
              if (useCommissionerGeneratedPasscode) {
                connectionCallbacks.onCommissionerDeclaration =
                    new MatterCallback<CommissionerDeclaration>() {
                      @Override
                      public void handle(CommissionerDeclaration cd) {
                        Log.i(TAG, "CastingPlayer CommissionerDeclaration message received: ");
                        cd.logDetail();

                        getActivity()
                            .runOnUiThread(
                                () -> {
                                  connectionFragmentStatusTextView.setText(
                                      "CommissionerDeclaration message received from Casting Player: \n\n");
                                  if (cd.getCommissionerPasscode()) {

                                    displayPasscodeInputDialog(getActivity());

                                    connectionFragmentStatusTextView.setText(
                                        "CommissionerDeclaration message received from Casting Player: A passcode is now displayed for the user by the Casting Player. \n\n");
                                  }
                                });
                      }
                    };
              }

              MatterError err =
                  targetCastingPlayer.verifyOrEstablishConnection(
                      connectionCallbacks, MIN_CONNECTION_TIMEOUT_SEC, idOptions);

              if (err.hasError()) {
                getActivity()
                    .runOnUiThread(
                        () -> {
                          connectionFragmentStatusTextView.setText(
                              "Casting Player connection failed due to: " + err + "\n\n");
                        });
              }
            });
  }

  private void displayPasscodeInputDialog(Context context) {
    AlertDialog.Builder builder = new AlertDialog.Builder(context);

    String title = "Enter the Commissioner-Generated Passcode";
    String instructions =
        "Input the Commissioner-Generated passcode displayed on the CastingPlayer UX, or use the default provided (12345678).";

    LayoutInflater inflater = LayoutInflater.from(context);
    View dialogView = inflater.inflate(R.layout.custom_passcode_dialog, null);

    TextView titleTextView = dialogView.findViewById(R.id.dialog_title);
    TextView instructionsTextView = dialogView.findViewById(R.id.dialog_instructions);
    titleTextView.setText(title);
    instructionsTextView.setText(instructions);

    // Set up the input dialog with the default passcode
    final EditText input = dialogView.findViewById(R.id.passcode_input);
    input.setText(DEFAULT_COMMISSIONER_GENERATED_PASSCODE);

    // Set up the buttons
    builder.setPositiveButton(
        "Continue Connecting",
        new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int which) {
            String passcode = input.getText().toString();
            Log.i(
                TAG,
                "displayPasscodeInputDialog() User entered Commissioner-Generated passcode: "
                    + passcode);

            // Display the user entered passcode on the screen
            connectionFragmentStatusTextView.setText(
                "Continue Connecting with user entered Commissioner-Generated passcode: "
                    + passcode
                    + "\n\n");

            long passcodeLongValue = 12345678;
            try {
              passcodeLongValue = Long.parseLong(passcode);
            } catch (NumberFormatException nfe) {
            }

            // Update the CommissionableData DataProvider and AndroidChipPlatform with the user
            // entered Commissioner-Generated setup passcode. This is mandatory for
            // Commissioner-Generated passcode commissioning.
            ((DataProvider) InitializationExample.commissionableDataProvider)
                .updateCommissionableDataSetupPasscode(
                    passcodeLongValue, DEFAULT_DISCRIMINATOR_FOR_CGP_FLOW);
            MatterError err =
                CastingApp.getInstance().updateAndroidChipPlatformWithCommissionableData();

            if (err.hasError()) {
              connectionFragmentStatusTextView.setText(
                  "displayPasscodeInputDialog() Casting Player connection failed due to: "
                      + err
                      + "\n\n");
              Log.e(TAG, "displayPasscodeInputDialog() calling stopConnecting() due to: " + err);
              targetCastingPlayer.stopConnecting();
            } else {
              Log.i(TAG, "displayPasscodeInputDialog() calling continueConnecting()");
              connectionFragmentStatusTextView =
                  getView().findViewById(R.id.connectionFragmentStatusText);
              connectionFragmentStatusTextView.setText(
                  "Continuing connection with Casting Player with device name: "
                      + targetCastingPlayer.getDeviceName()
                      + "\nCommissioner-Generated Setup Passcode: "
                      + InitializationExample.commissionableDataProvider.get().getSetupPasscode()
                      + "\nDiscriminator: "
                      + InitializationExample.commissionableDataProvider.get().getDiscriminator());

              targetCastingPlayer.continueConnecting();
            }
          }
        });

    builder.setNegativeButton(
        "Cancel",
        new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int which) {
            Log.i(
                TAG,
                "displayPasscodeInputDialog() user cancelled the Commissioner-Generated Passcode input dialog.");
            connectionFragmentStatusTextView.setText(
                "Connection attempt with Casting Player cancelled by the user, route back to exit. \n\n");
            targetCastingPlayer.stopConnecting();
            dialog.cancel();
          }
        });

    builder.setView(dialogView);
    AlertDialog alertDialog = builder.create();
    alertDialog.show();
    alertDialog
        .getWindow()
        .setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of connection */
    void handleConnectionComplete(
        CastingPlayer castingPlayer, Boolean useCommissionerGeneratedPasscode);
  }
}
