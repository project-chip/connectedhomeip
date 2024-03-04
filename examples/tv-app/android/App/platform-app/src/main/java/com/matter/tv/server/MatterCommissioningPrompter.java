package com.matter.tv.server;

import static androidx.core.content.ContextCompat.getSystemService;

import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.widget.EditText;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.NotificationCompat;
import com.matter.tv.server.service.MatterServant;
import com.matter.tv.server.tvapp.Message;
import com.matter.tv.server.tvapp.UserPrompter;
import com.matter.tv.server.tvapp.UserPrompterResolver;

public class MatterCommissioningPrompter extends UserPrompterResolver implements UserPrompter {

  private final Context context;
  private NotificationManager notificationManager;
  private final String CHANNEL_ID = "MatterCommissioningPrompter.CHANNEL";
  private final int SUCCESS_ID = 0;
  private final int FAIL_ID = 1;

  public MatterCommissioningPrompter(Context context) {
    this.context = context;
    this.createNotificationChannel();
    setUserPrompter(this);
  }

  private Activity getActivity() {
    return MatterServant.get().getActivity();
  }

  public void promptForCommissionOkPermission(
      int vendorId, int productId, String commissioneeName) {
    Log.d(
        TAG,
        "Received prompt for OK permission vendor id:"
            + vendorId
            + " productId:"
            + productId
            + ". Commissionee: "
            + commissioneeName);

    getActivity()
        .runOnUiThread(
            () -> {
              AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
              builder
                  .setMessage(
                      commissioneeName
                          + " is requesting permission to cast to this device, approve?")
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
            });
  }

  @Override
  public void promptForCommissionPinCode(int vendorId, int productId, String commissioneeName) {
    Log.d(
        TAG,
        "Received prompt for PIN code vendor id:"
            + vendorId
            + " productId:"
            + productId
            + ". Commissionee: "
            + commissioneeName);

    getActivity()
        .runOnUiThread(
            () -> {
              EditText editText = new EditText(getActivity());
              AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

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
            });
  }

  public void hidePromptsOnCancel(int vendorId, int productId, String commissioneeName) {
    Log.d(
        TAG,
        "Received Cancel from vendor id:"
            + vendorId
            + " productId:"
            + productId
            + ". Commissionee: "
            + commissioneeName);

    getActivity()
        .runOnUiThread(
            () -> {
              AlertDialog.Builder abuilder = new AlertDialog.Builder(getActivity());
              abuilder
                  .setMessage("Cancelled connection to " + commissioneeName)
                  .setTitle("Connection Cancelled")
                  .create()
                  .show();

              NotificationCompat.Builder builder =
                  new NotificationCompat.Builder(getActivity(), CHANNEL_ID)
                      .setSmallIcon(R.drawable.ic_baseline_check_24)
                      .setContentTitle("Connection Cancelled")
                      .setContentText("Cancelled connection to " + commissioneeName)
                      .setPriority(NotificationCompat.PRIORITY_DEFAULT);

              notificationManager.notify(SUCCESS_ID, builder.build());
            });
  }

  public void promptWithCommissionerPasscode(
      int vendorId,
      int productId,
      String commissioneeName,
      long passcode,
      int pairingHint,
      String pairingInstruction) {
    Log.d(
        TAG,
        "Received prompt for Commissioner Passcode:"
            + passcode
            + " vendor id:"
            + vendorId
            + " productId:"
            + productId
            + ". Commissionee: "
            + commissioneeName);

    getActivity()
        .runOnUiThread(
            () -> {
              EditText editText = new EditText(getActivity());
              AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

              builder
                  .setMessage(
                      "Please enter "
                          + passcode
                          + " in "
                          + commissioneeName
                          + " app. "
                          + pairingInstruction
                          + " ["
                          + pairingHint
                          + "]")
                  .setTitle("Passcode" + passcode)
                  .setPositiveButton(
                      "Ok",
                      (dialog, which) -> {
                        OnCommissionerPasscodeOK();
                      })
                  .setNegativeButton(
                      "Cancel",
                      (dialog, which) -> {
                        OnCommissionerPasscodeCancel();
                      })
                  .create()
                  .show();
            });
  }

  public void promptCommissioningStarted(int vendorId, int productId, String commissioneeName) {
    Log.d(
        TAG,
        "Received prompt for started vendor id:"
            + vendorId
            + " productId:"
            + productId
            + ". Commissionee: "
            + commissioneeName);
    getActivity()
        .runOnUiThread(
            () -> {
              AlertDialog.Builder abuilder = new AlertDialog.Builder(getActivity());
              abuilder
                  .setMessage("Starting connection to " + commissioneeName)
                  .setTitle("Connection Starting")
                  .create()
                  .show();

              NotificationCompat.Builder builder =
                  new NotificationCompat.Builder(getActivity(), CHANNEL_ID)
                      .setSmallIcon(R.drawable.ic_baseline_check_24)
                      .setContentTitle("Connection Starting")
                      .setContentText("Starting connection to " + commissioneeName)
                      .setPriority(NotificationCompat.PRIORITY_DEFAULT);

              notificationManager.notify(SUCCESS_ID, builder.build());
            });
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
    getActivity()
        .runOnUiThread(
            () -> {
              AlertDialog.Builder abuilder = new AlertDialog.Builder(getActivity());
              abuilder
                  .setMessage(
                      "Success. "
                          + commissioneeName
                          + " can now cast to this device. Visit settings to manage access control for casting.")
                  .setTitle("Connection Complete")
                  .create()
                  .show();

              NotificationCompat.Builder builder =
                  new NotificationCompat.Builder(getActivity(), CHANNEL_ID)
                      .setSmallIcon(R.drawable.ic_baseline_check_24)
                      .setContentTitle("Connection Complete")
                      .setContentText(
                          "Success. "
                              + commissioneeName
                              + " can now cast to this device. Visit settings to manage access control for casting.")
                      .setPriority(NotificationCompat.PRIORITY_DEFAULT);

              notificationManager.notify(SUCCESS_ID, builder.build());
            });
  }

  public void promptCommissioningFailed(String commissioneeName, String error) {
    Log.d(TAG, "Received prompt for failure Commissionee: " + commissioneeName);
    getActivity()
        .runOnUiThread(
            () -> {
              AlertDialog.Builder abuilder = new AlertDialog.Builder(getActivity());
              abuilder
                  .setMessage("Failed. " + commissioneeName + " experienced error: " + error + ".")
                  .setTitle("Connection Failed")
                  .create()
                  .show();

              NotificationCompat.Builder builder =
                  new NotificationCompat.Builder(getActivity(), CHANNEL_ID)
                      .setSmallIcon(R.drawable.ic_baseline_clear_24)
                      .setContentTitle("Connection Failed")
                      .setContentText(
                          "Failed. " + commissioneeName + " experienced error: " + error + ".")
                      .setPriority(NotificationCompat.PRIORITY_DEFAULT);

              notificationManager.notify(FAIL_ID, builder.build());
            });
  }

  public void promptWithMessage(Message message) {
    Log.d(TAG, "Received message prompt for " + message.messageText);
    getActivity()
        .runOnUiThread(
            () -> {
              if (message.responseOptions.length != 2) {
                AlertDialog.Builder abuilder = new AlertDialog.Builder(getActivity());
                abuilder
                    .setMessage("" + message.messageId + ":" + message.messageText)
                    .setTitle("New Message from Test")
                    .setPositiveButton(
                        "Ok",
                        (dialog, which) -> {
                          OnMessageResponse(message.messageId, 0); // ack
                        })
                    .setNegativeButton(
                        "Ignore",
                        (dialog, which) -> {
                          OnMessageResponse(message.messageId, -1); // ignore
                        })
                    .create()
                    .show();
              } else {
                AlertDialog.Builder abuilder = new AlertDialog.Builder(getActivity());
                abuilder
                    .setMessage("" + message.messageId + ":" + message.messageText)
                    .setTitle("New Message from Test")
                    .setPositiveButton(
                        message.responseOptions[0].label,
                        (dialog, which) -> {
                          OnMessageResponse(message.messageId, message.responseOptions[0].id);
                        })
                    .setNegativeButton(
                        message.responseOptions[1].label,
                        (dialog, which) -> {
                          OnMessageResponse(message.messageId, message.responseOptions[1].id);
                        })
                    .create()
                    .show();
              }
            });
  }

  private void createNotificationChannel() {
    // Create the NotificationChannel, but only on API 26+ because
    // the NotificationChannel class is new and not in the support library
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      Log.d(TAG, "   -------------            createNotificationChannel");
      CharSequence name = "MatterPromptNotificationChannel";
      String description = "Matter Channel for sending notifications";
      int importance = NotificationManager.IMPORTANCE_DEFAULT;
      NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
      channel.setDescription(description);
      // Register the channel with the system; you can't change the importance
      // or other notification behaviors after this
      this.notificationManager = getSystemService(context, NotificationManager.class);
      notificationManager.createNotificationChannel(channel);
    } else {
      Log.d(TAG, "   -------------            NOT createNotificationChannel");
    }
  }
}
