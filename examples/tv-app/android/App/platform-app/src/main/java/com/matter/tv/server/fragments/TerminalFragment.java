package com.matter.tv.server.fragments;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;
import com.matter.tv.server.R;

/**
 * A simple {@link Fragment} subclass. Use the {@link TerminalFragment#newInstance} factory method
 * to create an instance of this fragment.
 */
public class TerminalFragment extends Fragment {

  private EditText terminalText;

  public TerminalFragment() {
    // Required empty public constructor
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment CommssionerFragment.
   */
  public static TerminalFragment newInstance() {
    TerminalFragment fragment = new TerminalFragment();
    Bundle args = new Bundle();
    fragment.setArguments(args);
    return fragment;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_terminal, container, false);
  }

  @Override
  public void onResume() {
    super.onResume();

    terminalText = getView().findViewById(R.id.terminalTxt);

    getView()
        .findViewById(R.id.OkBtn)
        .setOnClickListener(
            v -> {
              String message = terminalText.getText().toString();

              AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

              builder.setMessage(message).setTitle("Response").create().show();
            });
  }
}
