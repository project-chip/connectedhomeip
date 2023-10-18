/*
 *   Copyright (c) 2023 Project CHIP Authors
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

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.R;
import com.chip.casting.SuccessCallback;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.CastingPlayerDiscovery;
import com.matter.casting.core.MatterCastingPlayerDiscovery;
import com.matter.casting.support.MatterError;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

public class DiscoveryExampleFragment extends Fragment {
  private static final String TAG = DiscoveryExampleFragment.class.getSimpleName();
  private boolean discovering = false;
  private TextView matterDiscoveryMessageTextView;
  private int discoveryAttempt = 0;
  private static final ScheduledExecutorService executorService =
      Executors.newSingleThreadScheduledExecutor();
  private ScheduledFuture scheduledFutureTask;
  private static final List<CastingPlayer> castingPlayerList = new ArrayList<>();
  // private FailureCallback failureCallback;
  private static SuccessCallback<CastingPlayer> discoverySuccessCallback;

  // Get a singleton instance of the MatterCastingPlayerDiscovery
  private static final CastingPlayerDiscovery matterCastingPlayerDiscovery =
      MatterCastingPlayerDiscovery.getInstance();

  /**
   * Implementation of a CastingPlayerChangeListener used to listen to changes in the discovered
   * CastingPlayers
   */
  private static final CastingPlayerDiscovery.CastingPlayerChangeListener
      castingPlayerChangeListener =
          new CastingPlayerDiscovery.CastingPlayerChangeListener() {
            private final String TAG =
                CastingPlayerDiscovery.CastingPlayerChangeListener.class.getSimpleName();

            @Override
            public void onAdded(CastingPlayer castingPlayer) {
              if (castingPlayer != null) {
                Log.i(
                    TAG,
                    "onAdded() Discovered CastingPlayer deviceId: " + castingPlayer.getDeviceId());
                // Display CastingPlayer info on the screen
                if (discoverySuccessCallback != null) {
                  discoverySuccessCallback.handle(castingPlayer);
                } else {
                  Log.e(
                      TAG,
                      "onAdded() Warning: DiscoveryExampleFragment UX discoverySuccessCallback not set");
                }
              } else {
                Log.d(TAG, "onAdded()");
                // Attempt to invoke interface method on a null object reference will throw an error
                Log.d(
                    TAG,
                    "onAdded() Discovered CastingPlayer with deviceId: "
                        + castingPlayer.getDeviceId());
              }
            }

            @Override
            public void onChanged(CastingPlayer castingPlayer) {
              Log.i(
                  TAG,
                  "onChanged() Discovered changes to CastingPlayer with deviceId: "
                      + castingPlayer.getDeviceId());
              // TODO: In following PRs. Consume changes to the provided CastingPlayer.
            }

            @Override
            public void onRemoved(CastingPlayer castingPlayer) {
              Log.i(
                  TAG,
                  "onRemoved() Removed CastingPlayer with deviceId: "
                      + castingPlayer.getDeviceId());
              // TODO: In following PRs. Consume CastingPlayer removed or lost from the network.
            }
          };

  public static DiscoveryExampleFragment newInstance() {
    Log.i(TAG, "newInstance() called");
    return new DiscoveryExampleFragment();
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Log.i(TAG, "onCreate() called");
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    Log.i(TAG, "onCreateView() called");
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_matter_discovery_example, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.i(TAG, "onViewCreated() called");

    matterDiscoveryMessageTextView =
        getActivity().findViewById(R.id.matterDiscoveryMessageTextView);
    matterDiscoveryMessageTextView.setText(
        getString(R.string.matter_discovery_message_initializing_text));

    ArrayAdapter<CastingPlayer> arrayAdapter =
        new CastingPlayerCommissionerAdapter(getActivity(), castingPlayerList);
    final ListView list = getActivity().findViewById(R.id.castingPlayerList);
    list.setAdapter(arrayAdapter);

    Log.d(TAG, "onViewCreated() creating callbacks");

    this.discoverySuccessCallback =
        new SuccessCallback<CastingPlayer>() {
          @Override
          public void handle(CastingPlayer castingPlayer) {
            Log.d(
                TAG,
                "SuccessCallback handle() CastingPlayer deviceId: " + castingPlayer.getDeviceId());
            new Handler(Looper.getMainLooper())
                .post(
                    () -> {
                      final Optional<CastingPlayer> playerInList =
                          castingPlayerList
                              .stream()
                              .filter(
                                  node -> castingPlayer.discoveredCastingPlayerHasSameSource(node))
                              .findFirst();
                      if (playerInList.isPresent()) {
                        Log.d(
                            TAG,
                            "Replacing existing CastingPlayer entry "
                                + playerInList.get().getDeviceId()
                                + " in castingPlayerList list");
                        arrayAdapter.remove(playerInList.get());
                      }
                      arrayAdapter.add(castingPlayer);
                    });
          }
        };

    Button startDiscoveryButton = getView().findViewById(R.id.startDiscoveryButton);
    startDiscoveryButton.setOnClickListener(
        v -> {
          Log.i(
              TAG, "onViewCreated() startDiscoveryButton button clicked. Calling startDiscovery()");
          arrayAdapter.clear();
          if (!startDiscovery(15)) {
            Log.e(TAG, "onViewCreated() startDiscovery() call Failed");
          }
        });

    Button stopDiscoveryButton = getView().findViewById(R.id.stopDiscoveryButton);
    stopDiscoveryButton.setOnClickListener(
        v -> {
          Log.i(TAG, "onViewCreated() stopDiscoveryButton button clicked. Calling stopDiscovery()");
          stopDiscovery();
          Log.i(TAG, "onViewCreated() stopDiscoveryButton button clicked. Canceling future task");
          scheduledFutureTask.cancel(true);
        });

    Button clearDiscoveryResultsButton = getView().findViewById(R.id.clearDiscoveryResultsButton);
    clearDiscoveryResultsButton.setOnClickListener(
        v -> {
          Log.i(
              TAG, "onViewCreated() clearDiscoveryResultsButton button clicked. Clearing results");
          arrayAdapter.clear();
        });
  }

  @Override
  public void onResume() {
    super.onResume();
    Log.i(TAG, "onResume() called. Calling startDiscovery()");
    if (!startDiscovery(15)) {
      Log.e(TAG, "onResume() Warning: startDiscovery() call Failed");
    }
  }

  @Override
  public void onPause() {
    super.onPause();
    Log.i(TAG, "onPause() called");
    stopDiscovery();
    scheduledFutureTask.cancel(true);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener of Connection Button click. */
    // TODO: In following PRs. Implement CastingPlayer connection
    void handleConnectionButtonClicked(CastingPlayer castingPlayer);
  }

  private boolean startDiscovery(int discoveryDuration) {
    Log.i(TAG, "startDiscovery() called, discoveryDuration: " + discoveryDuration + " seconds");
    if (discovering) {
      Log.e(TAG, "startDiscovery() Warning: already discovering, stop before starting");
      return false;
    }
    // Add the implemented CastingPlayerChangeListener to listen to changes in the discovered
    // CastingPlayers
    MatterError errAdd =
        matterCastingPlayerDiscovery.addCastingPlayerChangeListener(castingPlayerChangeListener);
    if (errAdd.hasError()) {
      Log.e(TAG, "startDiscovery() addCastingPlayerChangeListener() called, errAdd: " + errAdd);
      return false;
    }
    // Start discovery
    Log.i(TAG, "startDiscovery() calling startDiscovery()");
    MatterError errStart = matterCastingPlayerDiscovery.startDiscovery();
    if (errStart.hasError()) {
      Log.e(TAG, "startDiscovery() startDiscovery() called, errStart: " + errStart);
      return false;
    }

    discovering = true;
    discoveryAttempt++;
    Log.i(TAG, "startDiscovery() started discovery attempt #" + discoveryAttempt);

    matterDiscoveryMessageTextView.setText(
        getString(R.string.matter_discovery_message_discovering_text));
    Log.d(
        TAG,
        "startDiscovery() text set to: "
            + getString(R.string.matter_discovery_message_discovering_text));

    // Schedule a service to stop discovery and remove the CastingPlayerChangeListener
    // Safe to call if discovery is not running
    scheduledFutureTask =
        executorService.schedule(
            () -> {
              Log.i(
                  TAG,
                  "startDiscovery() executorService "
                      + discoveryDuration
                      + " seconds timer expired. Calling stopDiscovery() on attempt #"
                      + discoveryAttempt);
              stopDiscovery();
            },
            discoveryDuration,
            TimeUnit.SECONDS);

    return true;
  }

  private void stopDiscovery() {
    Log.i(TAG, "stopDiscovery() called on attempt #" + discoveryAttempt);
    boolean stopDiscoverySuccess = true;
    if (!discovering) {
      Log.e(TAG, "stopDiscovery() not discovering");
      return;
    }

    // Stop discovery
    MatterError errStop = matterCastingPlayerDiscovery.stopDiscovery();
    if (errStop.hasError()) {
      Log.e(
          TAG,
          "stopDiscovery() MatterCastingPlayerDiscovery.stopDiscovery() called, errStop: "
              + errStop);
      stopDiscoverySuccess = false;
    } else {
      // TODO: In following PRs. Implement stop discovery in the Android core API.
      Log.d(TAG, "stopDiscovery() MatterCastingPlayerDiscovery.stopDiscovery() success");
      discovering = false;
    }

    matterDiscoveryMessageTextView.setText(
        getString(R.string.matter_discovery_message_stopped_text));
    Log.d(
        TAG,
        "stopDiscovery() text set to: "
            + getString(R.string.matter_discovery_message_stopped_text));

    // Remove the CastingPlayerChangeListener
    Log.i(TAG, "stopDiscovery() removing CastingPlayerChangeListener");
    MatterError errRemove =
        matterCastingPlayerDiscovery.removeCastingPlayerChangeListener(castingPlayerChangeListener);
    if (errRemove.hasError()) {
      Log.e(
          TAG,
          "stopDiscovery() matterCastingPlayerDiscovery.removeCastingPlayerChangeListener() called, errRemove: "
              + errRemove);
      stopDiscoverySuccess = false;
    }

    if (!stopDiscoverySuccess) {
      Log.e(TAG, "stopDiscovery() Warning: complete with errors! Discovering: " + discovering);
    }
  }
}

class CastingPlayerCommissionerAdapter extends ArrayAdapter<CastingPlayer> {
  private final List<CastingPlayer> playerList;
  private final Context context;
  private LayoutInflater inflater;
  private static final String TAG = CastingPlayerCommissionerAdapter.class.getSimpleName();

  public CastingPlayerCommissionerAdapter(Context context, List<CastingPlayer> playerList) {
    super(context, 0, playerList);
    Log.i(TAG, "CastingPlayerCommissionerAdapter() constructor called");
    this.context = context;
    this.playerList = playerList;
    inflater = (LayoutInflater.from(context));
  }

  @Override
  public View getView(int i, View view, ViewGroup viewGroup) {
    view = inflater.inflate(R.layout.commissionable_player_list_item, null);
    String buttonText = getCastingPlayerButtonText(playerList.get(i));
    Button playerDescription = view.findViewById(R.id.commissionable_player_description);
    playerDescription.setText(buttonText);

    View.OnClickListener clickListener =
        v -> {
          CastingPlayer castingPlayer = playerList.get(i);
          Log.d(
              TAG,
              "OnItemClickListener.onClick() called for castingPlayer with deviceId: "
                  + castingPlayer.getDeviceId());
          DiscoveryExampleFragment.Callback callback1 = (DiscoveryExampleFragment.Callback) context;
          // TODO: In following PRs. Implement CastingPlayer connection
          // callback1.handleCommissioningButtonClicked(castingPlayer);
        };
    playerDescription.setOnClickListener(clickListener);
    return view;
  }

  private String getCastingPlayerButtonText(CastingPlayer player) {
    String main = player.getDeviceName() != null ? player.getDeviceName() : "";
    String aux = "" + (player.getDeviceId() != null ? "Device ID: " + player.getDeviceId() : "");
    aux +=
        player.getProductId() > 0
            ? (aux.isEmpty() ? "" : ", ") + "Product ID: " + player.getProductId()
            : "";
    aux +=
        player.getVendorId() > 0
            ? (aux.isEmpty() ? "" : ", ") + "Vendor ID: " + player.getVendorId()
            : "";
    aux +=
        player.getDeviceType() > 0
            ? (aux.isEmpty() ? "" : ", ") + "Device Type: " + player.getDeviceType()
            : "";
    aux = aux.isEmpty() ? aux : "\n" + aux;

    // String preCommissioned = commissioner.isPreCommissioned() ? " (Pre-commissioned)" : "";
    // return main + aux + preCommissioned;
    return main + aux;
  }
}
