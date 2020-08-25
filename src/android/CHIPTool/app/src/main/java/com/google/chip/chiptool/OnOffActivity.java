/*
 *   Copyright (c) 2020 Project CHIP Authors
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
 *
 */

package com.google.chip.chiptool;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;

public class OnOffActivity extends AppCompatActivity implements View.OnClickListener {
  private static final String TAG = OnOffActivity.class.getSimpleName();

  public static int ON_OFF_ACTIVITY_REQUEST = 1;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.on_off_activity);
    getSupportFragmentManager()
        .beginTransaction()
        .replace(R.id.on_off_frame, new OnOffFragment(this))
        .commit();
  }

  public static class OnOffFragment extends Fragment {
    OnOffActivity activity;

    OnOffFragment(OnOffActivity activity) {
      this.activity = activity;
    }

    @Override
    public View onCreateView(
        LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
      View inflated = inflater.inflate(R.layout.on_off_fragment, container, false);

      inflated.findViewById(R.id.send_on_request).setOnClickListener(activity);

      inflated.findViewById(R.id.send_off_request).setOnClickListener(activity);

      inflated.findViewById(R.id.send_toggle_request).setOnClickListener(activity);

      inflated
          .findViewById(R.id.back)
          .setOnClickListener(
              new View.OnClickListener() {
                public void onClick(View v) {
                  activity.finish();
                }
              });
      return inflated;
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
      super.onViewCreated(view, savedInstanceState);
    }
  }

  @Override
  public void onClick(View view) {}
}
