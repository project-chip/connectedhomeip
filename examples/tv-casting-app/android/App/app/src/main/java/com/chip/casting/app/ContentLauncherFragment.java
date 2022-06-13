package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.chip.casting.MatterCallbackHandler;
import com.chip.casting.TvCastingApp;

/** A {@link Fragment} to send Content Launcher commands from the TV Casting App. */
public class ContentLauncherFragment extends Fragment {
  private static final String TAG = ContentLauncherFragment.class.getSimpleName();

  private final TvCastingApp tvCastingApp;

  private View.OnClickListener launchUrlButtonClickListener;

  public ContentLauncherFragment(TvCastingApp tvCastingApp) {
    this.tvCastingApp = tvCastingApp;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param tvCastingApp TV Casting App (JNI)
   * @return A new instance of fragment ContentLauncherFragment.
   */
  public static ContentLauncherFragment newInstance(TvCastingApp tvCastingApp) {
    return new ContentLauncherFragment(tvCastingApp);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    this.launchUrlButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            EditText contentUrl = getView().findViewById(R.id.contentUrlEditText);
            EditText contentDisplayString =
                getView().findViewById(R.id.contentDisplayStringEditText);
            tvCastingApp.contentLauncherLaunchURL(
                contentUrl.getText().toString(),
                contentDisplayString.getText().toString(),
                new MatterCallbackHandler() {
                  @Override
                  public void handle(Status status) {
                    Log.d(TAG, "handle() called on LaunchURLResponse with success " + status);
                    TextView launchUrlStatus = getView().findViewById(R.id.launchUrlStatus);
                    launchUrlStatus.setText(status.isSuccess() ? "Success!" : "Failure!");
                  }
                });
          }
        };

    return inflater.inflate(R.layout.fragment_content_launcher, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "ContentLauncherFragment.onViewCreated called");
    getView().findViewById(R.id.launchUrlButton).setOnClickListener(launchUrlButtonClickListener);
  }
}
