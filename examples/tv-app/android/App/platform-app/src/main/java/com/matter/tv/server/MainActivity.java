package com.matter.tv.server;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import chip.setuppayload.DiscoveryCapability;
import chip.setuppayload.SetupPayload;
import chip.setuppayload.SetupPayloadParser;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.service.ContentAppAgentService;
import com.matter.tv.server.service.MatterServant;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map.Entry;

public class MainActivity extends AppCompatActivity {

  private static final String TAG = "MatterMainActivity";

  private ImageView mQrCodeImg;
  private TextView mQrCodeTxt;
  private TextView mManualPairingCodeTxt;
  private BroadcastReceiver broadcastReceiver;
  private ListView pkgUpdatesView;

  private LinkedHashMap<String, String> packages = new LinkedHashMap<>();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    mQrCodeImg = findViewById(R.id.qrCodeImg);
    mQrCodeTxt = findViewById(R.id.qrCodeTxt);
    mManualPairingCodeTxt = findViewById(R.id.manualPairingCodeTxt);
    findViewById(R.id.resetBtn)
        .setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View view) {
                MatterServant.get().restart();
              }
            });
    findViewById(R.id.OnOffBtn)
        .setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View view) {
                MatterServant.get().toggleOnOff();
              }
            });

    SeekBar sb = findViewById(R.id.seekBar);
    sb.setOnSeekBarChangeListener(
        new SeekBar.OnSeekBarChangeListener() {
          @Override
          public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
            MatterServant.get().updateLevel(i);
          }

          @Override
          public void onStartTrackingTouch(SeekBar seekBar) {}

          @Override
          public void onStopTrackingTouch(SeekBar seekBar) {}
        });

    // TODO: Get these parameters from PreferencesConfigurationManager
    HashSet<DiscoveryCapability> discoveryCapabilities = new HashSet<>();
    discoveryCapabilities.add(DiscoveryCapability.ON_NETWORK);
    SetupPayload payload =
        new SetupPayload(0, 9050, 65279, 0, discoveryCapabilities, 3840, 20202021);

    SetupPayloadParser parser = new SetupPayloadParser();
    try {
      String qrCode = parser.getQrCodeFromPayload(payload);
      mQrCodeTxt.setText(qrCode);

      Bitmap qrCodeBitmap = QRUtils.createQRCodeBitmap(qrCode, 800, 800);
      mQrCodeImg.setImageBitmap(qrCodeBitmap);
    } catch (SetupPayloadParser.SetupPayloadException e) {
      e.printStackTrace();
    }

    try {
      String manualPairingCode = parser.getManualEntryCodeFromPayload(payload);
      mManualPairingCodeTxt.setText("ManualPairingCode:" + manualPairingCode);
    } catch (SetupPayloadParser.SetupPayloadException e) {
      e.printStackTrace();
    }

    ArrayList<Entry<String, String>> lst =
        new ArrayList<Entry<String, String>>(packages.entrySet());

    ContentAppListAdapter adapter = new ContentAppListAdapter(this, R.layout.applist_item, lst);

    pkgUpdatesView = findViewById(R.id.pkgUpdates);
    pkgUpdatesView.setAdapter(adapter);
    registerReceiver(adapter);
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
    registerReceiver(broadcastReceiver, new IntentFilter("com.matter.tv.server.appagent.add"));
    registerReceiver(broadcastReceiver, new IntentFilter("com.matter.tv.server.appagent.remove"));

    ContentAppDiscoveryService.getRecieverInstance().registerSelf(this.getApplicationContext());
    ContentAppDiscoveryService.getRecieverInstance()
        .initializeMatterApps(this.getApplicationContext());
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
      ViewHolder mainViewHolder = null;
      if (convertView == null) {
        LayoutInflater inflator = LayoutInflater.from(getContext());
        convertView = inflator.inflate(layout, parent, false);
        ViewHolder viewHolder = new ViewHolder();
        viewHolder.appName = (TextView) convertView.findViewById(R.id.appNameTextView);
        viewHolder.appDetails = (TextView) convertView.findViewById(R.id.appDetailsTextView);
        viewHolder.appName.setText(getItem(position).getKey());
        viewHolder.appDetails.setText(getItem(position).getValue());
        viewHolder.sendMessageButton = (Button) convertView.findViewById(R.id.sendMessageButton);
        viewHolder.sendMessageButton.setText(R.string.send_command);
        viewHolder.sendMessageButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View view) {
                Log.i(TAG, "Button was clicked for " + position);
                ContentAppAgentService.sendCommand(
                    getApplicationContext(), getItem(position).getKey());
              }
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

  public class ViewHolder {
    TextView appName;
    TextView appDetails;
    Button sendMessageButton;
  }
}
