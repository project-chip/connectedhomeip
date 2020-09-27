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

package com.google.chip.chiptool.commissioner.thread.internal;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import androidx.fragment.app.DialogFragment;
import com.google.chip.chiptool.R;

public class InputNetworkPasswordDialogFragment extends DialogFragment implements DialogInterface.OnClickListener {

  private PasswordDialogListener passwordListener;
  private EditText passwordText;

  public interface PasswordDialogListener {
    public void onPositiveClick(InputNetworkPasswordDialogFragment fragment, String password);
    public void onNegativeClick(InputNetworkPasswordDialogFragment fragment);
  }

  public InputNetworkPasswordDialogFragment(PasswordDialogListener passwordListener) {
    this.passwordListener = passwordListener;
  }

  @Override
  public Dialog onCreateDialog(Bundle savedInstanceState) {
    AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
    LayoutInflater inflater = requireActivity().getLayoutInflater();
    View view = inflater.inflate(R.layout.commissioner_network_password_dialog, null);

    passwordText = view.findViewById(R.id.network_password);

    builder.setTitle("Enter Password");
    builder.setView(view);
    builder.setPositiveButton(R.string.commissioner_password_connect, this);
    builder.setNegativeButton(R.string.commissioner_password_cancel, this);

    return builder.create();
  }

  @Override
  public void onClick(DialogInterface dialogInterface, int which) {
    if (which == DialogInterface.BUTTON_POSITIVE) {
      passwordListener.onPositiveClick(this, passwordText.getText().toString());
    } else {
      passwordListener.onNegativeClick(this);
    }
  }
}
