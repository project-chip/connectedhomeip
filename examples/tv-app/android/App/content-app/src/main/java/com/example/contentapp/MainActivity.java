package com.example.contentapp;

import android.content.Intent;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import com.example.contentapp.matter.MatterAgentClient;
import com.matter.tv.app.api.Clusters;
import com.matter.tv.app.api.MatterIntentConstants;
import com.matter.tv.app.api.SetSupportedClustersRequest;
import com.matter.tv.app.api.SupportedCluster;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

  private static final String TAG = "ContentAppMainActivity";
  private static final String ATTR_PS_PLAYING = "Playback State : PLAYING";
  private static final String ATTR_PS_PAUSED = "Playback State : PAUSED";
  private static final String ATTR_PS_NOT_PLAYING = "Playback State : NOT_PLAYING";
  private static final String ATTR_PS_BUFFERING = "Playback State : BUFFERING";
  private static final String ATTR_TL_LONG_BAD = "Target List : LONG BAD";
  private static final String ATTR_TL_LONG = "Target List : LONG";
  private static final String ATTR_TL_SHORT = "Target List : SHORT";
  private final ExecutorService executorService = Executors.newSingleThreadExecutor();
  private String setupPIN = "";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    MatterAgentClient.initialize(getApplicationContext());

    setContentView(R.layout.activity_main);

    Intent intent = getIntent();
    String command = intent.getStringExtra(MatterIntentConstants.EXTRA_COMMAND_PAYLOAD);

    // use the text in a TextView
    TextView textView = (TextView) findViewById(R.id.commandTextView);
    textView.setText("Command Payload : " + command);

    Button setupPINButton = findViewById(R.id.setupPINButton);
    if (!setupPIN.isEmpty()) {
      EditText pinText = findViewById(R.id.setupPINText);
      pinText.setText(setupPIN);
    }
    setupPINButton.setOnClickListener(
        view -> {
          EditText pinText = findViewById(R.id.setupPINText);
          String pinStr = pinText.getText().toString();
          setupPIN = pinStr;
          CommandResponseHolder.getInstance()
              .setResponseValue(
                  Clusters.AccountLogin.Id,
                  Clusters.AccountLogin.Commands.GetSetupPIN.ID,
                  "{\""
                      + Clusters.AccountLogin.Commands.GetSetupPINResponse.Fields.SetupPIN
                      + "\":\""
                      + pinStr
                      + "\"}");
        });

    Button attributeUpdateButton = findViewById(R.id.updateAttributeButton);

    attributeUpdateButton.setOnClickListener(
        view -> {
          Spinner dropdown = findViewById(R.id.spinnerAttribute);
          String attribute = (String) dropdown.getSelectedItem();
          switch (attribute) {
            case ATTR_PS_PLAYING:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.Types.PlaybackStateEnum.Playing);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_PS_PAUSED:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.Types.PlaybackStateEnum.Paused);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_PS_BUFFERING:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.Types.PlaybackStateEnum.Buffering);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_PS_NOT_PLAYING:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.Types.PlaybackStateEnum.NotPlaying);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_TL_LONG_BAD:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.TargetNavigator.Id,
                      Clusters.TargetNavigator.Attributes.TargetList,
                      AttributeHolder.TL_LONG_BAD);
              reportAttributeChange(
                  Clusters.TargetNavigator.Id, Clusters.TargetNavigator.Attributes.TargetList);
              break;
            case ATTR_TL_LONG:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.TargetNavigator.Id,
                      Clusters.TargetNavigator.Attributes.TargetList,
                      AttributeHolder.TL_LONG);
              reportAttributeChange(
                  Clusters.TargetNavigator.Id, Clusters.TargetNavigator.Attributes.TargetList);
              break;
            case ATTR_TL_SHORT:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.TargetNavigator.Id,
                      Clusters.TargetNavigator.Attributes.TargetList,
                      AttributeHolder.TL_SHORT);
              reportAttributeChange(
                  Clusters.TargetNavigator.Id, Clusters.TargetNavigator.Attributes.TargetList);
              break;
          }
        });
    Spinner dropdown = findViewById(R.id.spinnerAttribute);
    String[] items =
        new String[] {
          ATTR_PS_PLAYING,
          ATTR_PS_PAUSED,
          ATTR_PS_NOT_PLAYING,
          ATTR_PS_BUFFERING,
          ATTR_TL_LONG,
          ATTR_TL_SHORT,
          ATTR_TL_LONG_BAD
        };
    ArrayAdapter<String> adapter =
        new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, items);
    dropdown.setAdapter(adapter);

    MatterAgentClient matterAgentClient = MatterAgentClient.getInstance();
    if (matterAgentClient != null) {
      SetSupportedClustersRequest supportedClustersRequest = new SetSupportedClustersRequest();
      supportedClustersRequest.supportedClusters = new ArrayList<SupportedCluster>();
      SupportedCluster supportedCluster = new SupportedCluster();
      supportedCluster.clusterIdentifier = 1;
      supportedClustersRequest.supportedClusters.add(supportedCluster);
      executorService.execute(() -> matterAgentClient.reportClusters(supportedClustersRequest));
    }
  }

  private void reportAttributeChange(final int clusterId, final int attributeId) {
    executorService.execute(
        new Runnable() {
          @Override
          public void run() {
            MatterAgentClient client = MatterAgentClient.getInstance();
            client.reportAttributeChange(clusterId, attributeId);
          }
        });
  }
}
