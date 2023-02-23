package com.chip.casting.app;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.chip.casting.DiscoveredNodeData;
import com.chip.casting.FailureCallback;
import com.chip.casting.MatterError;
import com.chip.casting.SuccessCallback;
import com.chip.casting.TvCastingApp;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

/** A {@link Fragment} to discover commissioners on the network */
public class CommissionerDiscoveryFragment extends Fragment {
  private static final String TAG = CommissionerDiscoveryFragment.class.getSimpleName();
  private static final long DISCOVERY_POLL_INTERVAL_MS = 15000;
  private static final List<DiscoveredNodeData> commissionerVideoPlayerList = new ArrayList<>();
  private FailureCallback failureCallback;
  private SuccessCallback<DiscoveredNodeData> successCallback;
  private ScheduledFuture poller;
  private final TvCastingApp tvCastingApp;
  private ScheduledExecutorService executor;

  public CommissionerDiscoveryFragment(TvCastingApp tvCastingApp) {
    this.tvCastingApp = tvCastingApp;
    this.executor = Executors.newSingleThreadScheduledExecutor();
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment CommissionerDiscoveryFragment.
   */
  public static CommissionerDiscoveryFragment newInstance(TvCastingApp tvCastingApp) {
    return new CommissionerDiscoveryFragment(tvCastingApp);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_commissioner_discovery, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    Button manualCommissioningButton = getView().findViewById(R.id.manualCommissioningButton);
    // In the ideal case we wouldn't rely on the host activity to maintain this object since
    // the lifecycle of the context isn't tied to this callback. Since this is an example app
    // this should work fine.
    Callback callback = (Callback) this.getActivity();
    View.OnClickListener manualCommissioningButtonOnClickListener =
        v -> callback.handleCommissioningButtonClicked(null);
    manualCommissioningButton.setOnClickListener(manualCommissioningButtonOnClickListener);
    ArrayAdapter<DiscoveredNodeData> arrayAdapter =
        new VideoPlayerCommissionerAdapter(getActivity(), commissionerVideoPlayerList);
    final ListView list = getActivity().findViewById(R.id.commissionerList);
    list.setAdapter(arrayAdapter);
    list.setOnItemClickListener(
        (parent, view1, position, id) -> {
          DiscoveredNodeData discoveredNodeData =
              (DiscoveredNodeData) parent.getItemAtPosition(position);
          Log.d(TAG, "OnItemClickListener.onClick called for " + discoveredNodeData);
          Callback callback1 = (Callback) getActivity();
          callback1.handleCommissioningButtonClicked(discoveredNodeData);
        });

    this.successCallback =
        new SuccessCallback<DiscoveredNodeData>() {
          @Override
          public void handle(DiscoveredNodeData discoveredNodeData) {
            Log.d(TAG, "Discovered a Video Player Commissioner: " + discoveredNodeData);
            new Handler(Looper.getMainLooper())
                .post(
                    () -> {
                      if (commissionerVideoPlayerList.contains(discoveredNodeData)) {
                        Log.d(TAG, "Replacing existing entry in players list");
                        arrayAdapter.remove(discoveredNodeData);
                      }
                      arrayAdapter.add(discoveredNodeData);
                    });
          }
        };

    this.failureCallback =
        new FailureCallback() {
          @Override
          public void handle(MatterError matterError) {
            Log.e(TAG, "Error occurred during video player commissioner discovery: " + matterError);
            if (MatterError.DISCOVERY_SERVICE_LOST == matterError) {
              Log.d(TAG, "Attempting to restart service");
              tvCastingApp.discoverVideoPlayerCommissioners(successCallback, this);
            }
          }
        };

    Button discoverButton = getView().findViewById(R.id.discoverButton);
    discoverButton.setOnClickListener(
        v -> {
          Log.d(TAG, "Discovering on button click");
          tvCastingApp.discoverVideoPlayerCommissioners(successCallback, failureCallback);
        });
  }

  @Override
  public void onResume() {
    super.onResume();
    Log.d(TAG, "Auto discovering");

    poller =
        executor.scheduleAtFixedRate(
            () -> {
              tvCastingApp.discoverVideoPlayerCommissioners(successCallback, failureCallback);
            },
            0,
            DISCOVERY_POLL_INTERVAL_MS,
            TimeUnit.MILLISECONDS);
  }

  @Override
  public void onPause() {
    super.onPause();
    tvCastingApp.stopVideoPlayerDiscovery();
    poller.cancel(true);
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener of Commissioning Button click. */
    void handleCommissioningButtonClicked(DiscoveredNodeData selectedCommissioner);
  }
}

class VideoPlayerCommissionerAdapter extends ArrayAdapter<DiscoveredNodeData> {
  private final List<DiscoveredNodeData> playerList;
  private LayoutInflater inflater;

  public VideoPlayerCommissionerAdapter(
      Context applicationContext, List<DiscoveredNodeData> playerList) {
    super(applicationContext, 0, playerList);
    this.playerList = playerList;
    inflater = (LayoutInflater.from(applicationContext));
  }

  @Override
  public View getView(int i, View view, ViewGroup viewGroup) {
    view = inflater.inflate(R.layout.commissionable_player_list_item, null);
    String buttonText = getCommissionerButtonText(playerList.get(i));
    TextView playerDescription = view.findViewById(R.id.commissionable_player_description);
    playerDescription.setText(buttonText);
    return view;
  }

  private String getCommissionerButtonText(DiscoveredNodeData commissioner) {
    String main = commissioner.getDeviceName() != null ? commissioner.getDeviceName() : "";
    String aux =
        "" + (commissioner.getProductId() > 0 ? "Product ID: " + commissioner.getProductId() : "");
    aux +=
        commissioner.getDeviceType() > 0
            ? (aux.isEmpty() ? "" : " ") + "Device Type: " + commissioner.getDeviceType()
            : "";
    aux +=
        commissioner.getVendorId() > 0
            ? (aux.isEmpty() ? "" : " from ") + "Vendor ID: " + commissioner.getVendorId()
            : "";
    aux = aux.isEmpty() ? aux : "\n[" + aux + "]";

    String preCommissioned = commissioner.isPreCommissioned() ? " (Pre-commissioned)" : "";
    return main + aux + preCommissioned;
  }
}
