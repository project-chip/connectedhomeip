package com.matter.tv.server.fragments;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;
import com.matter.tv.server.R;
import com.matter.tv.server.tvapp.AppPlatformShellCommands;

/**
 * A simple {@link Fragment} subclass. Use the {@link TerminalFragment#newInstance} factory method
 * to create an instance of this fragment.
 */
public class TerminalFragment extends Fragment {

  private EditText terminalText;
  private TextView terminalUsageDescription;
  private AppPlatformShellCommands shellCommands;

  private static String TERMINAL_INSTRUCTIONS =
      "add <vid> [<pid>]    Add app with given vendor ID [1, 2, 9050]. Usage: add 9050\r\n"
          + "remove <endpoint>    Remove app at given endpoint [6, 7, etc]. Usage: remove 6\r\n"
          + "setpin <endpoint> <pincode>  Set pincode for app with given endpoint ID. Usage: setpin 6 34567890\r\n"
          + "commission <udc-entry>     Commission given udc-entry using given pincode from corresponding app. Usage:"
          + "commission 0\r\n"
          + "add-admin-vendor <vid> Add vendor ID to list which will receive admin privileges. Usage: "
          + "add-admin-vendor 65521\r\n"
          + "print-app-access     Print all ACLs for app platform fabric. Usage: print-app-access\r\n"
          + "remove-app-access    Remove all ACLs for app platform fabric. Usage: remove-app-access\r\n";

  public TerminalFragment() {
    // Required empty public constructor
    shellCommands = new AppPlatformShellCommands();
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
    terminalUsageDescription = getView().findViewById(R.id.terminalDescriptionTxt);
    terminalUsageDescription.setText(TERMINAL_INSTRUCTIONS);

    getView()
        .findViewById(R.id.OkBtn)
        .setOnClickListener(
            v -> {
              String message = terminalText.getText().toString();

              String response = shellCommands.OnExecuteCommand(message.split(" "));

              AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

              builder.setMessage(response).create().show();
            });
  }
}
