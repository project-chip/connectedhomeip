package com.google.chip.chiptool;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;

public class OnOffActivity extends AppCompatActivity implements View.OnClickListener {
  private static final String TAG = OnOffActivity.class.getSimpleName();

  public static int ON_OFF_ACTIVITY_REQUEST = 1;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.on_off_activity);
    getSupportFragmentManager()
        .beginTransaction()
        .replace(R.id.on_off_frame, new OnOffFragment(this))
        .commit();
  }

  public static class OnOffFragment extends Fragment {
    OnOffActivity activity;

    OnOffFragment(OnOffActivity activity) {
      this.activity = activity;
    }

    @Override
    public View onCreateView(
        LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
      View inflated = inflater.inflate(R.xml.on_off_fragment, container, false);

      inflated
              .findViewById(R.id.send_on_request)
              .setOnClickListener(activity);

      inflated
              .findViewById(R.id.send_off_request)
              .setOnClickListener(activity);

      inflated
              .findViewById(R.id.send_toggle_request)
              .setOnClickListener(activity);

      inflated
              .findViewById(R.id.back)
              .setOnClickListener(
                      new View.OnClickListener() {
                        public void onClick(View v) {
                          activity.finish();
                        }
                      });

      ConnectionStatusFragment.updateStatus(activity);
      return inflated;
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
      super.onViewCreated(view, savedInstanceState);
    }
  }

  @Override
  public void onClick(View view) {
    if (view.getId() == R.id.send_echo_request) {
      ConnectionStatusFragment.updateStatus(this);
    }
  }

}
