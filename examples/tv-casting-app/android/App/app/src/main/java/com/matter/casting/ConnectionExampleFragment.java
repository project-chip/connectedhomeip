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
import androidx.fragment.app.FragmentActivity;
import com.R;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.support.CommissionerDeclaration;
import com.matter.casting.support.ConnectionCallbacks;
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
  // CastingPlayer/Commissioner-Generated passcode commissioning flow.
  private static final Integer DESIRED_TARGET_APP_VENDOR_ID_FOR_CGP_FLOW = 1111;
  private static final long DEFAULT_COMMISSIONER_GENERATED_PASSCODE = 12345678;
  private static final int DEFAULT_DISCRIMINATOR_FOR_CGP_FLOW = 0;
  private final CastingPlayer targetCastingPlayer;
  private final boolean useCommissionerGeneratedPasscode;
  private TextView connectionFragmentStatusTextView;
  private TextView commissionerDeclarationErrorTextView;
  private Button connectionFragmentNextButton;
  private AlertDialog passcodeDialog;

  public ConnectionExampleFragment(
      CastingPlayer targetCastingPlayer, boolean useCommissionerGeneratedPasscode) {
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
    commissionerDeclarationErrorTextView =
        getView().findViewById(R.id.commissionerDeclarationErrorTextView);
    if (useCommissionerGeneratedPasscode) {
      connectionFragmentStatusTextView.setText(
          "Verifying or establishing connection with Casting Player with device name: "
              + targetCastingPlayer.getDeviceName()
              + "\n\nAttempting CastingPlayer/Commissioner-Generated passcode commissioning.");
    } else {
      connectionFragmentStatusTextView.setText(
          "Verifying or establishing connection with Casting Player with device name: "
              + targetCastingPlayer.getDeviceName()
              + "\nClient/Commissionee-Generated Setup Passcode: "
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

              IdentificationDeclarationOptions idOptions;
              TargetAppInfo targetAppInfo = new TargetAppInfo(DESIRED_TARGET_APP_VENDOR_ID);

              if (useCommissionerGeneratedPasscode) {
                // Set commissionerPasscode to true for CastingPlayer/Commissioner-Generated
                // passcode commissioning.
                idOptions = new IdentificationDeclarationOptions(false, false, true, false, false);
                targetAppInfo = new TargetAppInfo(DESIRED_TARGET_APP_VENDOR_ID_FOR_CGP_FLOW);
                Log.d(
                    TAG,
                    "onViewCreated() calling CastingPlayer.verifyOrEstablishConnection() Target Content Application Vendor ID: "
                        + targetAppInfo.getVendorId()
                        + ", useCommissionerGeneratedPasscode: "
                        + useCommissionerGeneratedPasscode);
              } else {
                idOptions = new IdentificationDeclarationOptions();
                Log.d(
                    TAG,
                    "onViewCreated() calling CastingPlayer.verifyOrEstablishConnection() Target Content Application Vendor ID: "
                        + targetAppInfo.getVendorId());
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
                                  + targetCastingPlayer.getDeviceId()
                                  + ", useCommissionerGeneratedPasscode: "
                                  + useCommissionerGeneratedPasscode);
                          getActivity()
                              .runOnUiThread(
                                  () -> {
                                    connectionFragmentStatusTextView.setText(
                                        "Successfully connected to Casting Player with device name: "
                                            + targetCastingPlayer.getDeviceName()
                                            + "\n\nUsed CastingPlayer Passcode: "
                                            + useCommissionerGeneratedPasscode
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

              // The CommissionerDeclaration callback is optional and only needed for the
              // CastingPlayer/Commissioner-Generated
              // passcode commissioning flow. However, if we want to know when the
              // CastingPlayer/Commissioner user
              // has cancelled the connection attempt when using the Client/Commissionee generated
              // passcode flow, then we
              // need to implement this callback.
              connectionCallbacks.onCommissionerDeclaration =
                  new MatterCallback<CommissionerDeclaration>() {
                    @Override
                    public void handle(CommissionerDeclaration cd) {
                      Log.i(TAG, "CastingPlayer CommissionerDeclaration message received: ");
                      cd.logDetail();

                      FragmentActivity activity = getActivity();
                      // Prevent possible NullPointerException. This callback could be called when
                      // this fragment is not attached to its host activity or when the fragment's
                      // lifecycle is not in a valid state for interacting with the activity.
                      if (activity != null && !activity.isFinishing()) {
                        activity.runOnUiThread(
                            () -> {
                              connectionFragmentStatusTextView.setText(
                                  "CommissionerDeclaration message received from Casting Player: \n\n");
                              if (cd.getCommissionerPasscode()) {

                                displayPasscodeInputDialog(activity);

                                connectionFragmentStatusTextView.setText(
                                    "CommissionerDeclaration message received from Casting Player: A passcode is now displayed for the user by the Casting Player. \n\n");
                              }
                              if (cd.getCancelPasscode()) {
                                if (useCommissionerGeneratedPasscode) {
                                  connectionFragmentStatusTextView.setText(
                                      "CastingPlayer/Commissioner-Generated passcode connection attempt cancelled by the CastingPlayer/Commissioner user. \n\nRoute back to exit. \n\n");
                                } else {
                                  connectionFragmentStatusTextView.setText(
                                      "Connection attempt cancelled by the CastingPlayer/Commissioner user. \n\nRoute back to exit. \n\n");
                                }
                                if (passcodeDialog != null && passcodeDialog.isShowing()) {
                                  passcodeDialog.dismiss();
                                }
                              }
                              if (cd.getErrorCode() != CommissionerDeclaration.CdError.noError) {
                                commissionerDeclarationErrorTextView.setText(
                                    "CommissionerDeclaration error from CastingPlayer: "
                                        + cd.getErrorCode().getDescription());
                              }
                            });
                      }
                    }
                  };

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

  @Override
  public void onStop() {
    super.onStop();
    // Only stop connection if we are still connecting to the device
    if (targetCastingPlayer.getConnectionState() == CastingPlayer.ConnectionState.CONNECTING) {
      // NOTE, once stopConnecting() is called, the targetCastingPlayer's native object is freed
      MatterError err = targetCastingPlayer.stopConnecting();
      if (err.hasError()) {
        Log.e(
            TAG,
            "Going back before connection finishes but stopConnecting() failed due to: " + err);
      }
    }
  }

  private void displayPasscodeInputDialog(Context context) {
    AlertDialog.Builder builder = new AlertDialog.Builder(context);

    LayoutInflater inflater = LayoutInflater.from(context);
    View dialogView = inflater.inflate(R.layout.custom_passcode_dialog, null);

    // Set up the input dialog with the default passcode
    final EditText input = dialogView.findViewById(R.id.passcode_input);
    input.setText("" + DEFAULT_COMMISSIONER_GENERATED_PASSCODE);

    // Set up the buttons
    builder.setPositiveButton(
        "Continue Connecting",
        new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int which) {
            String userEnteredPasscode = input.getText().toString();
            Log.i(
                TAG,
                "displayPasscodeInputDialog() user-entered CastingPlayer/Commissioner-Generated passcode: "
                    + userEnteredPasscode);

            // Display the user entered passcode on the screen
            connectionFragmentStatusTextView.setText(
                "Continue Connecting with user-entered CastingPlayer/Commissioner-Generated passcode: "
                    + userEnteredPasscode
                    + "\n\n");

            long passcodeLongValue = DEFAULT_COMMISSIONER_GENERATED_PASSCODE;
            try {
              passcodeLongValue = Long.parseLong(userEnteredPasscode);
              Log.i(
                  TAG,
                  "displayPasscodeInputDialog() User entered CastingPlayer/Commissioner-Generated passcode: "
                      + passcodeLongValue);
            } catch (NumberFormatException nfe) {
              Log.e(
                  TAG,
                  "displayPasscodeInputDialog()User entered CastingPlayer/Commissioner-Generated passcode is not a valid integer. NumberFormatException: "
                      + nfe);
              connectionFragmentStatusTextView.setText(
                  "User entered CastingPlayer/Commissioner-Generated passcode is not a valid integer: "
                      + userEnteredPasscode
                      + "\n\n");
            }

            // Update the CommissionableData DataProvider with the user-entered
            // CastingPlayer / Commissioner-Generated setup passcode. This is mandatory for
            // Commissioner-Generated passcode commissioning since the commissioning session's PAKE
            // verifier needs to be updated with the entered passcode.
            InitializationExample.commissionableDataProvider.updateCommissionableDataSetupPasscode(
                passcodeLongValue, DEFAULT_DISCRIMINATOR_FOR_CGP_FLOW);

            Log.i(TAG, "displayPasscodeInputDialog() calling continueConnecting()");
            connectionFragmentStatusTextView =
                getView().findViewById(R.id.connectionFragmentStatusText);
            connectionFragmentStatusTextView.setText(
                "Continuing to connect with Casting Player with device name: "
                    + targetCastingPlayer.getDeviceName()
                    + "\nCastingPlayer/Commissioner-Generated Setup Passcode: "
                    + InitializationExample.commissionableDataProvider.get().getSetupPasscode()
                    + "\nDiscriminator: "
                    + InitializationExample.commissionableDataProvider.get().getDiscriminator());

            MatterError err = targetCastingPlayer.continueConnecting();

            if (err.hasError()) {
              MatterError finalErr = err;
              getActivity()
                  .runOnUiThread(
                      () -> {
                        connectionFragmentStatusTextView.setText(
                            "Casting Player CONTINUE CONNECTING failed due to: "
                                + finalErr
                                + ". Route back to disconnect & try again. \n\n");
                      });
              Log.e(TAG, "displayPasscodeInputDialog() continueConnecting() failed due to: " + err);
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
                "displayPasscodeInputDialog() user cancelled the CastingPlayer/Commissioner-Generated Passcode input dialog");
            connectionFragmentStatusTextView.setText(
                "Connection attempt with Casting Player cancelled by the Casting Client/Commissionee user. \n\nRoute back to disconnect & exit. \n\n");
            dialog.cancel();
          }
        });

    builder.setView(dialogView);
    passcodeDialog = builder.create(); // Store the dialog instance in the passcodeDialog variable
    passcodeDialog.show();
    passcodeDialog
        .getWindow()
        .setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of connection */
    void handleConnectionComplete(
        CastingPlayer castingPlayer, boolean useCommissionerGeneratedPasscode);
  }
}
