package com.example.contentapp;

import android.content.Intent;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import com.example.contentapp.matter.MatterAgentClient;
import com.matter.tv.app.api.Clusters;
import com.matter.tv.app.api.MatterIntentConstants;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

  private static final String TAG = "ContentAppMainActivity";
  private static final String ATTR_PS_PLAYING = "Playback State : PLAYING";
  private static final String ATTR_PS_PAUSED = "Playback State : PAUSED";
  private static final String ATTR_PS_NOT_PLAYING = "Playback State : NOT_PLAYING";
  private static final String ATTR_PS_BUFFERING = "Playback State : BUFFERING";
  private final ExecutorService executorService = Executors.newSingleThreadExecutor();

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

    Button sendMessageButton = findViewById(R.id.sendMessageButton);

    sendMessageButton.setOnClickListener(
        view -> {
          Spinner dropdown = findViewById(R.id.spinnerAttribute);
          String attribute = (String) dropdown.getSelectedItem();
          switch (attribute) {
            case ATTR_PS_PLAYING:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.PlaybackStateEnum.Playing);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_PS_PAUSED:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.PlaybackStateEnum.Paused);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_PS_BUFFERING:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.PlaybackStateEnum.Buffering);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
            case ATTR_PS_NOT_PLAYING:
              AttributeHolder.getInstance()
                  .setAttributeValue(
                      Clusters.MediaPlayback.Id,
                      Clusters.MediaPlayback.Attributes.CurrentState,
                      Clusters.MediaPlayback.PlaybackStateEnum.NotPlaying);
              reportAttributeChange(
                  Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState);
              break;
          }
        });
    Spinner dropdown = findViewById(R.id.spinnerAttribute);
    String[] items =
        new String[] {ATTR_PS_PLAYING, ATTR_PS_PAUSED, ATTR_PS_NOT_PLAYING, ATTR_PS_BUFFERING};
    ArrayAdapter<String> adapter =
        new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, items);
    dropdown.setAdapter(adapter);

    MatterAgentClient matterAgentClient = MatterAgentClient.getInstance();
    executorService.execute(matterAgentClient::reportClusters);

    // Setting up attribute defaults
    AttributeHolder.getInstance()
        .setAttributeValue(
            Clusters.ContentLauncher.Id,
            Clusters.ContentLauncher.Attributes.AcceptHeader,
            "[\"video/mp4\", \"application/x-mpegURL\", \"application/dash+xml\"]");
    AttributeHolder.getInstance()
        .setAttributeValue(
            Clusters.ContentLauncher.Id,
            Clusters.ContentLauncher.Attributes.SupportedStreamingProtocols,
            3);
    AttributeHolder.getInstance()
        .setAttributeValue(
            Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState, 2);
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
