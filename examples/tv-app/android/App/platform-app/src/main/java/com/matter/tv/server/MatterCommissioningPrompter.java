package com.matter.tv.server;

import static androidx.core.content.ContextCompat.getSystemService;

import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.os.Build;
import android.util.Log;
import android.widget.EditText;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.NotificationCompat;
import com.matter.tv.server.tvapp.UserPrompter;
import com.matter.tv.server.tvapp.UserPrompterResolver;

public class MatterCommissioningPrompter extends UserPrompterResolver implements UserPrompter {

  private Activity activity;
  private NotificationManager notificationManager;
  private final String CHANNEL_ID = "MatterCommissioningPrompter.CHANNEL";
  private final int SUCCESS_ID = 0;
  private final int FAIL_ID = 1;

  public MatterCommissioningPrompter(Activity activity) {
    this.activity = activity;
    this.createNotificationChannel();
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
    AlertDialog.Builder builder = new AlertDialog.Builder(activity);

    builder
        .setMessage(commissioneeName + " is requesting permission to cast to this device, approve?")
        .setTitle("Allow access to " + commissioneeName)
        .setPositiveButton(
            "Ok",
            (dialog, which) -> {
              OnPromptAccepted();
            })
        .setNegativeButton(
            "Cancel",
            (dialog, which) -> {
              OnPromptDeclined();
            })
        .create()
        .show();
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
    EditText editText = new EditText(activity);
    AlertDialog.Builder builder = new AlertDialog.Builder(activity);

    builder
        .setMessage("Please enter PIN displayed in casting app.")
        .setTitle("Allow access to " + commissioneeName)
        .setView(editText)
        .setPositiveButton(
            "Ok",
            (dialog, which) -> {
              String pinCode = editText.getText().toString();
              OnPinCodeEntered(Integer.parseInt(pinCode));
            })
        .setNegativeButton(
            "Cancel",
            (dialog, which) -> {
              OnPinCodeDeclined();
            })
        .create()
        .show();
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
        new NotificationCompat.Builder(activity, CHANNEL_ID)
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
        new NotificationCompat.Builder(activity, CHANNEL_ID)
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
      this.notificationManager = getSystemService(activity, NotificationManager.class);
      notificationManager.createNotificationChannel(channel);
    }
  }
}
