package com.tcl.chip.chiptvserver;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;
import chip.setuppayload.DiscoveryCapability;
import chip.setuppayload.SetupPayload;
import chip.setuppayload.SetupPayloadParser;
import com.tcl.chip.tvapp.KeypadInputManagerStub;
import com.tcl.chip.tvapp.TvApp;
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
    TvApp tvApp = new TvApp();
    tvApp.setKeypadInputManager(new KeypadInputManagerStub());

    AndroidChipPlatform chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(this),
            new PreferencesConfigurationManager(this),
            new NsdManagerServiceResolver(this),
            new ChipMdnsCallbackImpl());

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

    ChipAppServer chipAppServer = new ChipAppServer();
    chipAppServer.startApp();
  }
}
