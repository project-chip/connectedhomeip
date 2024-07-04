package com.matter.tv.server;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.matter.tv.server.fragments.ContentAppFragment;
import com.matter.tv.server.fragments.QrCodeFragment;
import com.matter.tv.server.fragments.TerminalFragment;
import java.util.LinkedHashMap;

public class MainActivity extends AppCompatActivity {

  private LinkedHashMap<String, String> packages = new LinkedHashMap<>();

  private BottomNavigationView.OnNavigationItemSelectedListener navListener =
      item -> {
        Fragment selectedFragment = null;
        switch (item.getItemId()) {
          case R.id.content_app:
            selectedFragment = ContentAppFragment.newInstance();
            break;
          case R.id.qr_code:
            selectedFragment = QrCodeFragment.newInstance();
            break;
          case R.id.terminal:
            selectedFragment = TerminalFragment.newInstance();
            break;
        }

        getSupportFragmentManager()
            .beginTransaction()
            .replace(R.id.fragment_container_view, selectedFragment)
            .commit();
        return true;
      };

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation);
    bottomNavigationView.setOnItemSelectedListener(navListener);

    getSupportFragmentManager()
        .beginTransaction()
        .replace(R.id.fragment_container_view, new QrCodeFragment())
        .commit();
    checkOverlayPermission();
  }

  private void checkOverlayPermission() {
    if (!Settings.canDrawOverlays(this)) {
      AlertDialog.Builder builder = new AlertDialog.Builder(this);
      builder
          .setMessage("Allow permission to display over other apps")
          .setTitle("Request overlay permission")
          .setPositiveButton(
              "Ok",
              (dialog, which) -> {
                dialog.dismiss();
                Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION);
                intent.setData(Uri.parse("package:" + getPackageName()));
                startActivity(intent);
              })
          .create()
          .show();
    }
  }
}
