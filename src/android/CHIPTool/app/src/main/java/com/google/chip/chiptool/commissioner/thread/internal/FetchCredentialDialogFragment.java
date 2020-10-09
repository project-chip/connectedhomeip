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
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.ThreadCommissionerException;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkCredential;

public class FetchCredentialDialogFragment extends DialogFragment
    implements DialogInterface.OnClickListener {

  private CredentialListener credentialListener;
  private TextView statusText;

  private AlertDialog dialog;

  NetworkCredentialFetcher fetcher = new NetworkCredentialFetcher();
  BorderAgentInfo borderAgentInfo;
  byte[] pskc;
  private ThreadNetworkCredential credential;

  public interface CredentialListener {
    void onCancelClick(FetchCredentialDialogFragment fragment);

    void onConfirmClick(FetchCredentialDialogFragment fragment, ThreadNetworkCredential credential);
  }

  public FetchCredentialDialogFragment(
      @NonNull BorderAgentInfo borderAgentInfo,
      @NonNull byte[] pskc,
      @NonNull CredentialListener credentialListener) {
    this.borderAgentInfo = borderAgentInfo;
    this.pskc = pskc;
    this.credentialListener = credentialListener;
  }

  @Override
  public Dialog onCreateDialog(Bundle savedInstanceState) {
    AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
    LayoutInflater inflater = requireActivity().getLayoutInflater();
    View view = inflater.inflate(R.layout.commissioner_fetch_credential_dialog, null);

    statusText = view.findViewById(R.id.fetch_credential_status);

    builder.setTitle("Fetching Credential");
    builder.setView(view);
    builder.setPositiveButton(R.string.commissioner_fetch_credential_done, this);
    builder.setNegativeButton(R.string.commissioner_fetch_credential_cancel, this);

    dialog = builder.create();
    return dialog;
  }

  @Override
  public void onStart() {
    super.onStart();

    dialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
    startFetching();
  }

  @Override
  public void onStop() {
    super.onStop();

    stopFetching();
  }

  @Override
  public void onClick(DialogInterface dialogInterface, int which) {
    if (which == DialogInterface.BUTTON_POSITIVE) {
      credentialListener.onConfirmClick(this, credential);
    } else {
      stopFetching();
      credentialListener.onCancelClick(this);
    }
  }

  private void startFetching() {
    Thread thread =
        new Thread(
            () -> {
              new Handler(Looper.getMainLooper())
                  .post(
                      () -> {
                        statusText.setText("petitioning...");
                      });

              String status;
              try {
                credential = fetcher.fetchNetworkCredential(borderAgentInfo, pskc);
                status = "success";
              } catch (ThreadCommissionerException e) {
                status = e.getMessage();
              }

              final String statusCopy = status;

              new Handler(Looper.getMainLooper())
                  .post(
                      () -> {
                        statusText.setText(statusCopy);
                        dialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(true);
                      });
            });
    thread.start();
  }

  private void stopFetching() {
    fetcher.cancel();
  }
}
