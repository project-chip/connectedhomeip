package com.tcl.chip.chiptvserver;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import chip.setuppayload.DiscoveryCapability;
import chip.setuppayload.SetupPayload;
import chip.setuppayload.SetupPayloadParser;
import com.tcl.chip.chiptvserver.service.MatterServant;
import java.util.HashSet;

public class MainActivity extends AppCompatActivity {

  private ImageView mQrCodeImg;
  private TextView mQrCodeTxt;
  private TextView mManualPairingCodeTxt;

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
  }
}
