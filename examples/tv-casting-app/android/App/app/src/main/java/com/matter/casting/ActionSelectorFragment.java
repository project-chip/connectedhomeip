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

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.R;
import com.matter.casting.core.CastingPlayer;

/** An interstitial {@link Fragment} to select one of the supported media actions to perform */
public class ActionSelectorFragment extends Fragment {
  private static final String TAG = ActionSelectorFragment.class.getSimpleName();

  private final CastingPlayer selectedCastingPlayer;

  private View.OnClickListener selectContentLauncherButtonClickListener;
  private View.OnClickListener disconnectButtonClickListener;

  public ActionSelectorFragment(CastingPlayer selectedCastingPlayer) {
    this.selectedCastingPlayer = selectedCastingPlayer;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param selectedCastingPlayer CastingPlayer that the casting app connected to
   * @return A new instance of fragment SelectActionFragment.
   */
  public static ActionSelectorFragment newInstance(CastingPlayer selectedCastingPlayer) {
    return new ActionSelectorFragment(selectedCastingPlayer);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    ActionSelectorFragment.Callback callback = (ActionSelectorFragment.Callback) this.getActivity();
    this.selectContentLauncherButtonClickListener =
        v -> {
          Log.d(TAG, "handle() called on selectContentLauncherButtonClickListener");
          callback.handleContentLauncherLaunchURLSelected(selectedCastingPlayer);
        };

    this.disconnectButtonClickListener =
        v -> {
          Log.d(TAG, "Disconnecting from current casting player");
          selectedCastingPlayer.disconnect();
          callback.handleDisconnect();
        };

    return inflater.inflate(R.layout.fragment_matter_action_selector, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "ActionSelectorFragment.onViewCreated called");
    getView()
        .findViewById(R.id.selectContentLauncherLaunchURLButton)
        .setOnClickListener(selectContentLauncherButtonClickListener);

    getView().findViewById(R.id.disconnectButton).setOnClickListener(disconnectButtonClickListener);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on selection of Content Launcher cluster */
    void handleContentLauncherLaunchURLSelected(CastingPlayer selectedCastingPlayer);

    /** Notifies listener to trigger transition on click of the Disconnect button */
    void handleDisconnect();
  }
}
