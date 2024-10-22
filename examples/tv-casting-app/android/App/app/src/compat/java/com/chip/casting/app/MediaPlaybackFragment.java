package com.chip.casting.app;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import com.R;
import com.chip.casting.ContentApp;
import com.chip.casting.FailureCallback;
import com.chip.casting.MatterError;
import com.chip.casting.MediaPlaybackTypes;
import com.chip.casting.SubscriptionEstablishedCallback;
import com.chip.casting.SuccessCallback;
import com.chip.casting.TvCastingApp;

/**
 * @deprecated Refer to com.matter.casting.MediaPlaybackSubscribeToCurrentStateExampleFragment.
 *     <p>A {@link Fragment} for the Media Playback cluster
 */
@Deprecated
public class MediaPlaybackFragment extends Fragment {
  private static final String TAG = MediaPlaybackFragment.class.getSimpleName();

  private final TvCastingApp tvCastingApp;

  private View.OnClickListener subscribeToCurrentStateButtonClickListener;

  private View.OnClickListener shutdownALlSubscriptionsButtonClickListener;

  private static final ContentApp kContentApp = new ContentApp((short) 4, null);

  public MediaPlaybackFragment(TvCastingApp tvCastingApp) {
    this.tvCastingApp = tvCastingApp;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param tvCastingApp TV Casting App (JNI)
   * @return A new instance of fragment MediaPlaybackFragment.
   */
  public static MediaPlaybackFragment newInstance(TvCastingApp tvCastingApp) {
    return new MediaPlaybackFragment(tvCastingApp);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    this.subscribeToCurrentStateButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Log.d(TAG, "SubscribeToCurrentStateButtonClickListener called");
            TextView minInterval = getView().findViewById(R.id.minIntervalEditText);
            TextView maxInterval = getView().findViewById(R.id.maxIntervalEditText);
            TextView subscriptionStatus =
                getView().findViewById(R.id.currentStateSubscriptionEstablishedStatus);
            TextView currentStateValue = getView().findViewById(R.id.currentStateValue);

            SuccessCallback<MediaPlaybackTypes.PlaybackStateEnum> successCallback =
                new SuccessCallback<MediaPlaybackTypes.PlaybackStateEnum>() {
                  @Override
                  public void handle(MediaPlaybackTypes.PlaybackStateEnum playbackStateEnum) {
                    Log.d(
                        TAG,
                        "handle() called on SuccessCallback<MediaPlaybackResponseTypes.PlaybackStateEnum> with "
                            + playbackStateEnum);
                    FragmentActivity fragmentActivity = getActivity();
                    if (fragmentActivity != null) {
                      fragmentActivity.runOnUiThread(
                          new Runnable() {
                            @Override
                            public void run() {
                              if (playbackStateEnum != null) {
                                currentStateValue.setText(playbackStateEnum.toString());
                              }
                            }
                          });
                    }
                  }
                };

            FailureCallback failureCallback =
                new FailureCallback() {
                  @Override
                  public void handle(MatterError matterError) {
                    Log.d(TAG, "handle() called on FailureCallback with " + matterError);
                    getActivity()
                        .runOnUiThread(
                            new Runnable() {
                              @Override
                              public void run() {
                                currentStateValue.setText("Error!");
                              }
                            });
                  }
                };

            SubscriptionEstablishedCallback subscriptionEstablishedCallback =
                new SubscriptionEstablishedCallback() {
                  @Override
                  public void handle() {
                    Log.d(TAG, "handle() called on SubscriptionEstablishedCallback");
                    FragmentActivity fragmentActivity = getActivity();
                    if (fragmentActivity != null) {
                      fragmentActivity.runOnUiThread(
                          new Runnable() {
                            @Override
                            public void run() {
                              subscriptionStatus.setText("Subscription established!");
                            }
                          });
                    }
                  }
                };

            boolean retVal =
                tvCastingApp.mediaPlayback_subscribeToCurrentState(
                    kContentApp,
                    successCallback,
                    failureCallback,
                    Integer.parseInt(minInterval.getText().toString()),
                    Integer.parseInt(maxInterval.getText().toString()),
                    subscriptionEstablishedCallback);
            Log.d(TAG, "tvCastingApp.mediaPlayback_subscribeToCurrentState returned " + retVal);
            if (retVal == false) {
              subscriptionStatus.setText("Subscribe call failed!");
            }
          }
        };

    this.shutdownALlSubscriptionsButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Log.d(TAG, "Shutting down all subscriptions");
            tvCastingApp.shutdownAllSubscriptions();
          }
        };

    return inflater.inflate(R.layout.fragment_media_playback, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "MediaPlaybackFragment.onViewCreated called");
    getView()
        .findViewById(R.id.subscribeToCurrentStateButton)
        .setOnClickListener(subscribeToCurrentStateButtonClickListener);

    getView()
        .findViewById(R.id.shutdownAllSubscriptionsButton)
        .setOnClickListener(shutdownALlSubscriptionsButtonClickListener);
  }
}
