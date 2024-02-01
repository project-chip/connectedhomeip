package com.matter.tv.server;

import static androidx.core.content.ContextCompat.getSystemService;

import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.NotificationCompat;

import com.matter.tv.server.databinding.LayoutEnterPinDialogBinding;
import com.matter.tv.server.databinding.LayoutUdcDialogBinding;
import com.matter.tv.server.service.MatterServant;
import com.matter.tv.server.tvapp.UserPrompter;
import com.matter.tv.server.tvapp.UserPrompterResolver;
import com.matter.tv.server.utils.ResourceUtils;

import java.lang.ref.WeakReference;

public class MatterCommissioningPrompter extends UserPrompterResolver implements UserPrompter {

  private final Context context;
  private NotificationManager notificationManager;
  private final String CHANNEL_ID = "MatterCommissioningPrompter.CHANNEL";
  private final int SUCCESS_ID = 0;
  private final int FAIL_ID = 1;
  private final VH mHandler;
  public MatterCommissioningPrompter(Context context) {
    this.context = context;
    this.createNotificationChannel();
    mHandler = new VH(this);
  }

  public void promptForCommissionOkPermission(
          int vendorId, int productId, String commissioneeName) {
    // TODO: find app by vendorId and productId
    Log.d(
            TAG,
            "Received prompt for OK permission vendor id:"
                    + vendorId
                    + " productId:"
                    + productId
                    + ". Commissionee: "
                    + commissioneeName);

    Message message = Message.obtain();
    message.arg1 = vendorId;
    message.arg2 = productId;
    message.obj = commissioneeName;
    message.what = 101;
    mHandler.sendMessage(message);
  }

  @Override
  public void promptForCommissionPinCode(int vendorId, int productId, String commissioneeName) {
    // TODO: find app by vendorId and productId
    Log.d(
            TAG,
            "Received prompt for PIN code vendor id:"
                    + vendorId
                    + " productId:"
                    + productId
                    + ". Commissionee: "
                    + commissioneeName);

    WindowManager.LayoutParams params = new WindowManager.LayoutParams();
    params.type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
    params.format = PixelFormat.TRANSLUCENT;
    params.gravity = Gravity.CENTER;
    params.width = ResourceUtils.dp2px(MatterTvServerApplication.getApplication(),300);
    params.height = WindowManager.LayoutParams.WRAP_CONTENT;
    params.windowAnimations = android.R.style.Animation_Dialog;
    params.dimAmount = 0.6f;
    params.flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND;
    LayoutEnterPinDialogBinding enterPinDialogBinding = LayoutEnterPinDialogBinding.inflate(LayoutInflater.from(MatterTvServerApplication.getApplication().getApplicationContext()));
    WindowManager windowManager = (WindowManager) MatterTvServerApplication.getApplication().getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
    enterPinDialogBinding.tvContent.setText("Please enter PIN displayed in casting app.");
    enterPinDialogBinding.tvTitle.setText("Allow access to " + commissioneeName);
    enterPinDialogBinding.tvLeft.setOnClickListener(v -> {
      OnPromptDeclined();
      windowManager.removeViewImmediate(enterPinDialogBinding.getRoot());
    });
    enterPinDialogBinding.tvRight.setOnClickListener(v -> {
      String pinCode = enterPinDialogBinding.etPin.getEditableText().toString();
      if (!TextUtils.isEmpty(pinCode)){
        OnPinCodeEntered(Integer.parseInt(pinCode));
        windowManager.removeViewImmediate(enterPinDialogBinding.getRoot());
      }
    });
    windowManager.addView(enterPinDialogBinding.getRoot(),params);

  }

  public void promptCommissioningSucceeded(int vendorId, int productId, String commissioneeName) {
    Log.d(
            TAG,
            "Received prompt for success vendor id:"
                    + vendorId
                    + " productId:"
                    + productId
                    + ". Commissionee: "
                    + commissioneeName);
    NotificationCompat.Builder builder =
            new NotificationCompat.Builder(MatterTvServerApplication.getApplication(), CHANNEL_ID)
                    .setSmallIcon(R.drawable.ic_baseline_check_24)
                    .setContentTitle("Connection Complete")
                    .setContentText(
                            "Success. "
                                    + commissioneeName
                                    + " can now cast to this device. Visit settings to manage access control for casting.")
                    .setPriority(NotificationCompat.PRIORITY_DEFAULT);

    notificationManager.notify(SUCCESS_ID, builder.build());
  }

  public void promptCommissioningFailed(String commissioneeName, String error) {
    Log.d(TAG, "Received prompt for failure Commissionee: " + commissioneeName);
    NotificationCompat.Builder builder =
            new NotificationCompat.Builder(MatterTvServerApplication.getApplication(), CHANNEL_ID)
                    .setSmallIcon(R.drawable.ic_baseline_clear_24)
                    .setContentTitle("Connection Failed")
                    .setContentText("Failed. " + commissioneeName + " experienced error: " + error + ".")
                    .setPriority(NotificationCompat.PRIORITY_DEFAULT);

    notificationManager.notify(FAIL_ID, builder.build());
  }

  private void createNotificationChannel() {
    // Create the NotificationChannel, but only on API 26+ because
    // the NotificationChannel class is new and not in the support library
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      CharSequence name = "MatterPromptNotificationChannel";
      String description = "Matter Channel for sending notifications";
      int importance = NotificationManager.IMPORTANCE_DEFAULT;
      NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
      channel.setDescription(description);
      // Register the channel with the system; you can't change the importance
      // or other notification behaviors after this
      this.notificationManager = getSystemService(context, NotificationManager.class);
      notificationManager.createNotificationChannel(channel);
    }
  }

  static class VH extends Handler{

    private final WeakReference<MatterCommissioningPrompter> mPrompterWeakReference;
    public VH(MatterCommissioningPrompter commissioningPrompter){
      super(Looper.getMainLooper());
      mPrompterWeakReference = new WeakReference<>(commissioningPrompter);
    }
    @Override
    public void handleMessage(@NonNull Message msg) {
      MatterCommissioningPrompter commissioningPrompter = mPrompterWeakReference.get();
      if (commissioningPrompter == null) {
        return;
      }
      WindowManager.LayoutParams params = new WindowManager.LayoutParams();
      params.type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
      params.format = PixelFormat.TRANSLUCENT;
      params.gravity = Gravity.CENTER;
      params.width = ResourceUtils.dp2px(MatterTvServerApplication.getApplication(),300);
      params.height = WindowManager.LayoutParams.WRAP_CONTENT;
      params.windowAnimations = android.R.style.Animation_Dialog;
      params.dimAmount = 0.6f;
      params.flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
      LayoutUdcDialogBinding udcDialogBinding = LayoutUdcDialogBinding.inflate(LayoutInflater.from(MatterTvServerApplication.getApplication().getApplicationContext()));
      WindowManager windowManager = (WindowManager) MatterTvServerApplication.getApplication().getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
      udcDialogBinding.tvContent.setText(msg.obj + " is requesting permission to cast to this device, approve?");
      udcDialogBinding.tvTitle.setText("Allow access to " + msg.obj);
      udcDialogBinding.tvLeft.setOnClickListener(v -> {
        commissioningPrompter.OnPromptDeclined();
        windowManager.removeViewImmediate(udcDialogBinding.getRoot());
      });
      udcDialogBinding.tvRight.setOnClickListener(v -> {
        commissioningPrompter.OnPromptAccepted();
        windowManager.removeViewImmediate(udcDialogBinding.getRoot());
      });
      windowManager.addView(udcDialogBinding.getRoot(),params);

    }
  }
}
