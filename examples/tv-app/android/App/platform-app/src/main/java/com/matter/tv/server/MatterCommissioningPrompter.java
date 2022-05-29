package com.matter.tv.server;

import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;
import android.widget.EditText;

import androidx.appcompat.app.AlertDialog;
import androidx.core.app.NotificationCompat;

import com.tcl.chip.tvapp.UserPrompter;
import com.tcl.chip.tvapp.UserPrompterResolver;

import static androidx.core.content.ContextCompat.getSystemService;

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

    public void promptForCommissionOkPermission(int vendorId,
                                                int productId,
                                                String commissioneeName) {
        // TODO: find app by vendorId and productId
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);

        builder.setMessage("Will you allow access to " + commissioneeName + "?")
                .setTitle("Allow access to " + commissioneeName)
                .setPositiveButton("Ok", (dialog, which) -> {
                    OnPromptAccepted();
                })
                .setNegativeButton("Cancel", (dialog, which) -> {
                    OnPromptDeclined();
                })
                .create()
                .show();
    }

    @Override
    public void promptForCommissionPinCode(int vendorId,
                                           int productId,
                                           String commissioneeName) {
        // TODO: find app by vendorId and productId

        EditText editText = new EditText(activity);
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);

        builder.setMessage("Enter PIN code sent to your other device")
                .setTitle("Allow access to " + commissioneeName)
                .setView(editText)
                .setPositiveButton("Ok", (dialog, which) -> {
                    String pinCode = editText.getText().toString();
                    OnPinCodeEntered(Integer.parseInt(pinCode));
                })
                .setNegativeButton("Cancel", (dialog, which) -> {
                    OnPinCodeDeclined();
                })
                .create()
                .show();
    }

    public void promptCommissioningSucceeded(int vendorId, int productId, String commissioneeName) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_baseline_check_24)
                .setContentTitle("Device connected")
                .setContentText("Your device " + commissioneeName + " is ready for use.")
                .setPriority(NotificationCompat.PRIORITY_DEFAULT);

        notificationManager.notify(SUCCESS_ID, builder.build());
    }

    public void promptCommissioningFailed(String commissioneeName, String error) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_baseline_clear_24)
                .setContentTitle("Error occurred")
                .setContentText("Device " + commissioneeName + " experienced error: " + error)
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
}
