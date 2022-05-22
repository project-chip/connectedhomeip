package com.matter.tv.server.fragments;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.matter.tv.server.R;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.service.ContentAppAgentService;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map.Entry;

/**
 * A simple {@link Fragment} subclass. Use the {@link ContentAppFragment#newInstance} factory method
 * to create an instance of this fragment.
 */
public class ContentAppFragment extends Fragment {

  private static final String TAG = "SettingsFragment";
  private BroadcastReceiver broadcastReceiver;
  private ListView pkgUpdatesView;

  private LinkedHashMap<String, String> packages = new LinkedHashMap<>();

  public ContentAppFragment() {
    // Required empty public constructor
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param param1 Parameter 1.
   * @param param2 Parameter 2.
   * @return A new instance of fragment SettingsFragment.
   */
  // TODO: Rename and change types and number of parameters
  public static ContentAppFragment newInstance(String param1, String param2) {
    ContentAppFragment fragment = new ContentAppFragment();
    Bundle args = new Bundle();
    fragment.setArguments(args);
    return fragment;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    if (getArguments() != null) {}
  }

  @Override
  public void onResume() {
    super.onResume();

    ArrayList<Entry<String, String>> lst = new ArrayList<>(packages.entrySet());

    ContentAppListAdapter adapter =
        new ContentAppListAdapter(getContext(), R.layout.applist_item, lst);

    pkgUpdatesView = getView().findViewById(R.id.pkgUpdates);
    pkgUpdatesView.setAdapter(adapter);
    registerReceiver(adapter);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_content_app, container, false);
  }

  private class ContentAppListAdapter extends ArrayAdapter<Entry<String, String>> {

    private int layout;

    public ContentAppListAdapter(
        @NonNull Context context,
        int resource,
        @NonNull ArrayList<Entry<String, String>> packages) {
      super(context, resource, packages);
      layout = resource;
    }

    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
      ViewHolder mainViewHolder;
      if (convertView == null) {
        LayoutInflater inflator = LayoutInflater.from(getContext());
        convertView = inflator.inflate(layout, parent, false);

        ViewHolder viewHolder = new ViewHolder();
        viewHolder.appName = convertView.findViewById(R.id.appNameTextView);
        viewHolder.appDetails = convertView.findViewById(R.id.appDetailsTextView);
        viewHolder.appName.setText(getItem(position).getKey());
        viewHolder.appDetails.setText(getItem(position).getValue());
        viewHolder.sendMessageButton = convertView.findViewById(R.id.sendMessageButton);
        viewHolder.sendMessageButton.setText(R.string.send_command);
        viewHolder.sendMessageButton.setOnClickListener(
            view -> {
              Log.i(TAG, "Button was clicked for " + position);
              ContentAppAgentService.sendCommand(
                  getActivity().getApplicationContext(), getItem(position).getKey());
            });
        convertView.setTag(viewHolder);
      } else {
        mainViewHolder = (ViewHolder) convertView.getTag();
        mainViewHolder.appName.setText(getItem(position).getKey());
        mainViewHolder.appDetails.setText(getItem(position).getValue());
      }
      return convertView;
    }
  }

  private void registerReceiver(ArrayAdapter adapter) {
    broadcastReceiver =
        new BroadcastReceiver() {
          @Override
          public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            String packageName = intent.getStringExtra("com.matter.tv.server.appagent.add.pkg");
            if (action.equals("com.matter.tv.server.appagent.add")) {
              packages.put(
                  packageName, intent.getStringExtra("com.matter.tv.server.appagent.add.clusters"));
              adapter.clear();
              adapter.addAll(packages.entrySet().toArray());
              adapter.notifyDataSetChanged();
            } else if (action.equals("com.matter.tv.server.appagent.remove")) {
              if (packages.remove(packageName) != null) {
                adapter.clear();
                adapter.addAll(packages.entrySet().toArray());
                adapter.notifyDataSetChanged();
              }
            }
          }
        };
    getContext()
        .registerReceiver(broadcastReceiver, new IntentFilter("com.matter.tv.server.appagent.add"));
    getContext()
        .registerReceiver(
            broadcastReceiver, new IntentFilter("com.matter.tv.server.appagent.remove"));

    ContentAppDiscoveryService.getReceiverInstance().registerSelf(getContext());
    ContentAppDiscoveryService.getReceiverInstance().initializeMatterApps(getContext());
  }

  public class ViewHolder {
    TextView appName;
    TextView appDetails;
    Button sendMessageButton;
  }
}
