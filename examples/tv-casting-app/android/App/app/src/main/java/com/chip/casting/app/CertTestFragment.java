package com.chip.casting.app;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.R;
import com.chip.casting.ContentApp;
import com.chip.casting.ContentLauncherTypes;
import com.chip.casting.FailureCallback;
import com.chip.casting.MatterCallbackHandler;
import com.chip.casting.MatterError;
import com.chip.casting.MediaPlaybackTypes;
import com.chip.casting.SubscriptionEstablishedCallback;
import com.chip.casting.SuccessCallback;
import com.chip.casting.TvCastingApp;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/** A {@link Fragment} to send Content Launcher commands from the TV Casting App. */
public class CertTestFragment extends Fragment {
  private static final String TAG = CertTestFragment.class.getSimpleName();

  private final TvCastingApp tvCastingApp;

  private View.OnClickListener launchTestButtonClickListener;
  private List<String> certTestStatuses = new ArrayList<String>();

  private static final ContentApp kTVApp = new ContentApp((short) 1, null);
  private static final ContentApp kTVAppSpeaker = new ContentApp((short) 2, null);

  private static final ContentApp kContentApp = new ContentApp((short) 4, null);
  private ArrayAdapter<String> certTestStatusesAdapter;

  public CertTestFragment(TvCastingApp tvCastingApp) {
    this.tvCastingApp = tvCastingApp;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param tvCastingApp TV Casting App (JNI)
   * @return A new instance of fragment ContentLauncherFragment.
   */
  public static CertTestFragment newInstance(TvCastingApp tvCastingApp) {
    return new CertTestFragment(tvCastingApp);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    this.launchTestButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Activity activity = getActivity();
            certTestStatuses.clear();
            certTestStatusesAdapter.notifyDataSetChanged();

            runCertTests(activity);
          }
        };

    return inflater.inflate(R.layout.fragment_cert_test_launcher, container, false);
  }

  private void runCertTests(Activity activity) {
    CertTestMatterSuccessFailureCallback successFailureCallback =
        new CertTestMatterSuccessFailureCallback(activity);
    CertTestMatterSuccessCallback successCallback =
        new CertTestMatterSuccessCallback(successFailureCallback);
    CertTestMatterFailureCallback failureCallback =
        new CertTestMatterFailureCallback(successFailureCallback);
    CertTestMatterSuccessCallbackInteger successCallbackInteger =
        new CertTestMatterSuccessCallbackInteger(successFailureCallback);
    CertTestMatterCallbackHandler callback =
        new CertTestMatterCallbackHandler(successFailureCallback);

    runAndWait(
        "keypadInput_sendKey",
        successFailureCallback,
        () -> {
          // 3.3.1. [TC-KEYPADINPUT-3.1] Send Key Command Verification (DUT as Client)
          tvCastingApp.keypadInput_sendKey(kTVApp, (byte) 10, callback);
        });

    runAndWait(
        "applicationLauncher_launchApp",
        successFailureCallback,
        () -> {
          // 3.4.4. [TC-APPLAUNCHER-3.7.1] Launch Application Verification (DUT as Client)
          tvCastingApp.applicationLauncher_launchApp(
              kTVApp, (short) 123, "exampleid", new byte[] {}, callback);
        });

    runAndWait(
        "applicationLauncher_stopApp",
        successFailureCallback,
        () -> {
          // [TC-APPLAUNCHER-3.8.1] Stop Application Verification (DUT as Client)
          tvCastingApp.applicationLauncher_stopApp(kTVApp, (short) 123, "exampleid", callback);
        });

    runAndWait(
        "applicationLauncher_hideApp",
        successFailureCallback,
        () -> {
          // [TC-APPLAUNCHER-3.9.1] Hide Application Verification (DUT as Client)
          tvCastingApp.applicationLauncher_hideApp(kTVApp, (short) 123, "exampleid", callback);
        });

    runAndWait(
        "targetNavigator_navigateTarget",
        successFailureCallback,
        () -> {
          // 3.10.2. [TC-TGTNAV-8.2] Navigate Target Verification (DUT as Client)
          tvCastingApp.targetNavigator_navigateTarget(kTVApp, (byte) 1, "", callback);
        });

    ArrayList<ContentLauncherTypes.AdditionalInfo> list = new ArrayList<>();
    list.add(new ContentLauncherTypes.AdditionalInfo("imdb", "dummyId"));

    ArrayList<ContentLauncherTypes.Parameter> paramList = new ArrayList<>();
    paramList.add(new ContentLauncherTypes.Parameter(13, "Dummy Video", Optional.of(list)));

    runAndWait(
        "contentLauncher_launchContent",
        successFailureCallback,
        () -> {
          // 3.12.3. [TC-CONTENTLAUNCHER-10.4] Launch Content Verification (DUT as Client)
          tvCastingApp.contentLauncher_launchContent(
              kTVApp, new ContentLauncherTypes.ContentSearch(paramList), true, "", callback);
        });

    runAndWait(
        "contentLauncherLaunchURL",
        successFailureCallback,
        () -> {
          // 3.12.5. [TC-CONTENTLAUNCHER-10.6] Launch URL Verification (DUT as Client)
          tvCastingApp.contentLauncherLaunchURL(
              kTVApp, "https://dummyurl", "Dummy Content", callback);
        });

    runAndWait(
        "mediaPlayback_play",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_play(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_next",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_next(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_skipForward",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_skipForward(kContentApp, 10000, callback);
        });

    runAndWait(
        "mediaPlayback_skipBackward",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_skipBackward(kContentApp, 10000, callback);
        });

    runAndWait(
        "mediaPlayback_pause",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_pause(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_stopPlayback",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_stopPlayback(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_previous",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_previous(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_rewind",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_rewind(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_fastForward",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_fastForward(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_startOver",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_startOver(kContentApp, callback);
        });

    runAndWait(
        "mediaPlayback_seek",
        successFailureCallback,
        () -> {
          // 3.8.5. [TC-MEDIAPLAYBACK-6.5] Mandatory Media Playback Verification (DUT as Client)
          tvCastingApp.mediaPlayback_seek(kContentApp, 10000, callback);
        });

    // Additional Tests
    // Mandatory
    // OnOff cluster
    runAndWait(
        "onOff_on",
        successFailureCallback,
        () -> {
          tvCastingApp.onOff_on(kTVApp, callback);
        });

    runAndWait(
        "onOff_off",
        successFailureCallback,
        () -> {
          tvCastingApp.onOff_off(kTVApp, callback);
        });

    runAndWait(
        "onOff_toggle",
        successFailureCallback,
        () -> {
          tvCastingApp.onOff_toggle(kTVApp, callback);
        });

    // Application Basic
    runAndWait(
        "applicationBasic_readApplicationVersion",
        successFailureCallback,
        () -> {
          tvCastingApp.applicationBasic_readApplicationVersion(
              kContentApp, successCallback, failureCallback);
        });

    runAndWait(
        "applicationBasic_readVendorName",
        successFailureCallback,
        () -> {
          tvCastingApp.applicationBasic_readVendorName(
              kContentApp, successCallback, failureCallback);
        });

    runAndWait(
        "applicationBasic_readApplicationName",
        successFailureCallback,
        () -> {
          tvCastingApp.applicationBasic_readApplicationName(
              kContentApp, successCallback, failureCallback);
        });

    runAndWait(
        "applicationBasic_readVendorID",
        successFailureCallback,
        () -> {
          tvCastingApp.applicationBasic_readVendorID(
              kContentApp, successCallbackInteger, failureCallback);
        });

    runAndWait(
        "applicationBasic_readProductID",
        successFailureCallback,
        () -> {
          tvCastingApp.applicationBasic_readProductID(
              kContentApp, successCallbackInteger, failureCallback);
        });

    runAndWait(
        "messages_presentMessages",
        successFailureCallback,
        () -> {
          tvCastingApp.messages_presentMessages(kTVApp, "CastingAppTestMessage", callback);
        });

    runAndWait(
        "mediaPlayback_subscribeToCurrentState",
        successFailureCallback,
        () -> {
          tvCastingApp.mediaPlayback_subscribeToCurrentState(
              kContentApp,
              new SuccessCallback<MediaPlaybackTypes.PlaybackStateEnum>() {
                @Override
                public void handle(MediaPlaybackTypes.PlaybackStateEnum response) {
                  // Lets wait for the timeout to avoid the race condition issue in the SDK with
                  // ReadClient::Close()
                  // successFailureCallback.handle(MatterError.NO_ERROR);
                  addCertTestStatus(
                      activity, MatterError.NO_ERROR, "mediaPlayback_subscribeToCurrentState");
                }
              },
              failureCallback,
              0,
              20,
              new SubscriptionEstablishedCallback() {
                @Override
                public void handle() {
                  // Lets wait for the timeout to avoid the race condition issue in the SDK with
                  // ReadClient::Close()
                  // successFailureCallback.handle(MatterError.NO_ERROR);
                  addCertTestStatus(
                      activity, MatterError.NO_ERROR, "mediaPlayback_subscribeToCurrentState");
                }
              });
        });

    runAndWait(
        "shutdownAllSubscriptions",
        successFailureCallback,
        () -> {
          tvCastingApp.shutdownAllSubscriptions();
          successFailureCallback.handle(MatterError.NO_ERROR);
        });

    // Unsupported & Optional
    // 3.2.2. [TC-LOWPOWER-2.2] Low Power Mode Verification (DUT as Client)
    // 3.5.5. [TC-MEDIAINPUT-3.14] Select Input Verification (DUT as Client)
    // 3.5.6. [TC-MEDIAINPUT-3.15] Show Input Status Verification (DUT as Client)
    // 3.5.7. [TC-MEDIAINPUT-3.16] Hide Input Status Verification (DUT as Client)
    // 3.5.8. [TC-MEDIAINPUT-3.17] Rename Input Verification (DUT as Client)

    // 3.7.4. [TC-CHANNEL-5.4] Change Channel Verification (DUT as Client)
    // 3.7.5. [TC-CHANNEL-5.5] Change Channel By Number Verification (DUT as Client)
    // 3.7.6. [TC-CHANNEL-5.6] Skip Channel Verification (DUT as Client)

    // 19.9.3. [TC-AUDIOOUTPUT-7.3] Select Output Verification (DUT as Client)
    // 19.9.4. [TC-AUDIOOUTPUT-7.4] Rename Output Verification (DUT as Client)

    // 3.14.2. [TC-ALOGIN-12.2] Account Login Verification (DUT as Client)
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "ContentLauncherFragment.onViewCreated called");
    certTestStatusesAdapter =
        new ArrayAdapter<String>(
            this.getContext(),
            R.layout.cert_test_status_item,
            R.id.launchCertTestStatus,
            certTestStatuses);
    ListView launchCertTestStatusUpdatesView =
        getView().findViewById(R.id.launchCertTestStatusUpdates);
    launchCertTestStatusUpdatesView.setAdapter(certTestStatusesAdapter);
    getView()
        .findViewById(R.id.launchCertTestButton)
        .setOnClickListener(launchTestButtonClickListener);
  }

  private void runAndWait(
      String testMethod, CertTestMatterSuccessFailureCallback callback, Runnable runnable) {
    CountDownLatch cdl = new CountDownLatch(1);
    callback.setTestMethod(testMethod);
    callback.setCountDownLatch(cdl);
    runnable.run();
    try {
      if (!cdl.await(5, TimeUnit.SECONDS)) {
        Log.d(TAG, "Timed out for test to finish : " + testMethod);
      }
    } catch (InterruptedException e) {
      Log.d(TAG, "Interrupted waiting for test to finish : " + testMethod);
    }
  }

  private void addCertTestStatus(Activity activity, MatterError error, String testMethod) {
    Log.d(TAG, "Cert Test Status for " + testMethod + " : " + error);
    certTestStatuses.add(testMethod + ":" + (error.isNoError() ? "Success!" : "Failure!"));
    activity.runOnUiThread(
        () -> {
          certTestStatusesAdapter.notifyDataSetChanged();
        });
  }

  class CertTestMatterCallbackHandler extends MatterCallbackHandler {
    private CertTestMatterSuccessFailureCallback delegate;

    CertTestMatterCallbackHandler(CertTestMatterSuccessFailureCallback delegate) {
      this.delegate = delegate;
    }

    @Override
    public void handle(MatterError error) {
      delegate.handle(error);
    }
  }

  class CertTestMatterSuccessFailureCallback {
    private Activity activity;
    private String testMethod;
    private CountDownLatch cdl;

    CertTestMatterSuccessFailureCallback(Activity activity) {
      this.activity = activity;
    }

    void setTestMethod(String testMethod) {
      this.testMethod = testMethod;
    }

    public void setCountDownLatch(CountDownLatch cdl) {
      this.cdl = cdl;
    }

    public void handle(MatterError error) {
      try {
        cdl.countDown();
        addCertTestStatus(activity, error, testMethod);
      } catch (Exception e) {
        Log.e(
            TAG,
            "Exception when handling matter callback when running test method " + testMethod,
            e);
      }
    }

    public void handle(String response) {
      try {
        cdl.countDown();
        addCertTestStatus(activity, MatterError.NO_ERROR, testMethod);
      } catch (Exception e) {
        Log.e(
            TAG,
            "Exception when handling matter callback when running test method " + testMethod,
            e);
      }
    }
  }

  class CertTestMatterSuccessCallback extends SuccessCallback<String> {
    private CertTestMatterSuccessFailureCallback delegate;

    CertTestMatterSuccessCallback(CertTestMatterSuccessFailureCallback delegate) {
      this.delegate = delegate;
    }

    @Override
    public void handle(String response) {
      delegate.handle(response);
    }
  }

  class CertTestMatterFailureCallback extends FailureCallback {
    private CertTestMatterSuccessFailureCallback delegate;

    CertTestMatterFailureCallback(CertTestMatterSuccessFailureCallback delegate) {
      this.delegate = delegate;
    }

    @Override
    public void handle(MatterError err) {
      delegate.handle(err);
    }
  }

  class CertTestMatterSuccessCallbackInteger extends SuccessCallback<Integer> {

    private CertTestMatterSuccessFailureCallback delegate;

    CertTestMatterSuccessCallbackInteger(CertTestMatterSuccessFailureCallback delegate) {
      this.delegate = delegate;
    }

    @Override
    public void handle(Integer response) {
      delegate.handle(response.toString());
    }
  }
}
