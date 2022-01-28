package com.chip.casting;

import android.os.Bundle;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

  private TextView mHelloCastingTxt;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    mHelloCastingTxt = findViewById(R.id.helloCastingTxt);
    mHelloCastingTxt.setText("Hello, TV Casting app!");
  }
}
