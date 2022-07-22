package com.matter.tv.server.fragments;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.fragment.app.Fragment;
import chip.setuppayload.DiscoveryCapability;
import chip.setuppayload.SetupPayload;
import chip.setuppayload.SetupPayloadParser;
import com.matter.tv.server.QRUtils;
import com.matter.tv.server.R;
import com.matter.tv.server.service.MatterServant;
import java.util.HashSet;

/**
 * A simple {@link Fragment} subclass. Use the {@link QrCodeFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class QrCodeFragment extends Fragment {

  private ImageView mQrCodeImg;
  private TextView mQrCodeTxt;
  private TextView mManualPairingCodeTxt;

  public QrCodeFragment() {
    // Required empty public constructor
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment QrCodeFragment.
   */
  public static QrCodeFragment newInstance() {
    QrCodeFragment fragment = new QrCodeFragment();
    Bundle args = new Bundle();
    fragment.setArguments(args);
    return fragment;
  }

  @Override
  public void onResume() {
    super.onResume();

    mQrCodeImg = getView().findViewById(R.id.qrCodeImg);
    mQrCodeTxt = getView().findViewById(R.id.qrCodeTxt);
    mManualPairingCodeTxt = getView().findViewById(R.id.manualPairingCodeTxt);
    getView().findViewById(R.id.resetBtn).setOnClickListener(view -> MatterServant.get().restart());
    getView()
        .findViewById(R.id.OnOffBtn)
        .setOnClickListener(view -> MatterServant.get().toggleOnOff());
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

    SeekBar sb = getView().findViewById(R.id.seekBar);
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
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_qr_code, container, false);
  }
}
