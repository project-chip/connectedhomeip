package com.example.contentapp;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import com.example.contentapp.matter.MatterAgentClient;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

  private static final String TAG = "ContentAppMainActivity";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    MatterAgentClient matterAgentClient = MatterAgentClient.getInstance(getApplicationContext());
    final ExecutorService executorService = Executors.newSingleThreadExecutor();
    executorService.execute(matterAgentClient::reportClusters);
  }
}
