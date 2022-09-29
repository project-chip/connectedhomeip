package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

/** An interstitial {@link Fragment} to select one of the supported media actions to perform */
public class SelectClusterFragment extends Fragment {
  private static final String TAG = SelectClusterFragment.class.getSimpleName();

  private View.OnClickListener selectContentLauncherButtonClickListener;
  private View.OnClickListener selectMediaPlaybackButtonClickListener;

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment SelectActionFragment.
   */
  public static SelectClusterFragment newInstance() {
    return new SelectClusterFragment();
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    SelectClusterFragment.Callback callback = (SelectClusterFragment.Callback) this.getActivity();
    this.selectContentLauncherButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Log.d(TAG, "handle() called on selectContentLauncherButtonClickListener");
            callback.handleContentLauncherSelected();
          }
        };

    this.selectMediaPlaybackButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Log.d(TAG, "handle() called on selectMediaPlaybackButtonClickListener");
            callback.handleMediaPlaybackSelected();
          }
        };

    return inflater.inflate(R.layout.fragment_select_cluster, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "SelectActionFragment.onViewCreated called");
    getView()
        .findViewById(R.id.selectContentLauncherButton)
        .setOnClickListener(selectContentLauncherButtonClickListener);
    getView()
        .findViewById(R.id.selectMediaPlaybackButton)
        .setOnClickListener(selectMediaPlaybackButtonClickListener);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on selection of Content Launcher cluster */
    void handleContentLauncherSelected();

    /** Notifies listener to trigger transition on selection of Media Playback cluster */
    void handleMediaPlaybackSelected();
  }
}
