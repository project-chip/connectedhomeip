package com.matter.tv.server;

import static androidx.core.content.ContextCompat.getSystemService;

import android.annotation.SuppressLint;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.ContentResolver;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.NotificationCompat;
import com.matter.tv.server.databinding.LayoutDialogBinding;
import com.matter.tv.server.databinding.LayoutInputDialogBinding;
import com.matter.tv.server.model.PromptCommissionerPasscode;
import com.matter.tv.server.tvapp.Message;
import com.matter.tv.server.tvapp.UserPrompter;
import com.matter.tv.server.tvapp.UserPrompterResolver;
import com.matter.tv.server.utils.PxConvert;
import java.lang.ref.WeakReference;

public class MatterCommissioningPrompter extends UserPrompterResolver implements UserPrompter {

  private final Context context;
  private NotificationManager notificationManager;
  private final String CHANNEL_ID = "MatterCommissioningPrompter.CHANNEL";
  private final int SUCCESS_ID = 0;
  private final int FAIL_ID = 1;

  private final MsgHandler mHandler = new MsgHandler(this);

  public MatterCommissioningPrompter(Context context) {
    this.context = context;
    this.createNotificationChannel();
    setUserPrompter(this);
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
    ContentResolver contentResolver = context.getContentResolver();
    boolean authorisationDialogDisabled =
        Settings.Secure.getInt(contentResolver, "matter_show_authorisation_dialog", 0) == 0;
    // By default do not show authorisation dialog
    // only do so if customer or OS explicitly ask for it, by updating
    // matter_show_authorisation_dialog flag to 1
    if (authorisationDialogDisabled) {
      OnPromptAccepted();
      return;
    }
    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_CommissionOkPermission;
    obtained.obj = commissioneeName;
    mHandler.sendMessage(obtained);
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
    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_CommissionPinCode;
    obtained.obj = commissioneeName;
    mHandler.sendMessage(obtained);
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
    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_HidePromptsOnCancel;
    obtained.obj = commissioneeName;
    mHandler.sendMessage(obtained);
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
    Bundle bundle = new Bundle();
    PromptCommissionerPasscode promptCommissionerPasscode =
        new PromptCommissionerPasscode(
            vendorId, productId, commissioneeName, passcode, pairingHint, pairingInstruction);
    bundle.putParcelable(MsgHandler.KEY_PROMPT_COMMISSIONER_PASSCODE, promptCommissionerPasscode);
    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_CommissionerPasscode;
    obtained.setData(bundle);
    mHandler.sendMessage(obtained);
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
    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_CommissioningStarted;
    obtained.obj = commissioneeName;
    mHandler.sendMessage(obtained);
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
    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_CommissioningSucceeded;
    obtained.obj = commissioneeName;
    mHandler.sendMessage(obtained);
  }

  public void promptCommissioningFailed(String commissioneeName, String error) {
    Log.d(TAG, "Received prompt for failure Commissionee: " + commissioneeName);

    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_CommissioningFailed;
    obtained.obj = commissioneeName;
    mHandler.sendMessage(obtained);
  }

  public void promptWithMessage(Message message) {
    Log.d(TAG, "Received message prompt for " + message.messageText);

    android.os.Message obtained = android.os.Message.obtain();
    obtained.what = MsgHandler.MSG_PromptWithMessage;
    obtained.obj = message;
    mHandler.sendMessage(obtained);
  }

  @SuppressLint("ClickableViewAccessibility")
  private void handleCommissionOkPermission(android.os.Message msg) {
    String commissioneeName = (String) msg.obj;

    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(false)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 350))
            .setStyle(PromptDialogManager.DIALOG_STYLE_1)
            .build();

    LayoutDialogBinding udcDialogBinding =
        LayoutDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    udcDialogBinding.tvContent.setText(
        commissioneeName + " is requesting permission to cast to this device, approve?");
    udcDialogBinding.tvTitle.setText("Allow access to " + commissioneeName);
    udcDialogBinding.tvLeft.setOnClickListener(
        v -> {
          OnPromptDeclined();
          promptDialogManager.removeViewImmediate(udcDialogBinding.getRoot());
        });
    udcDialogBinding.tvRight.setOnClickListener(
        v -> {
          OnPromptAccepted();
          promptDialogManager.removeViewImmediate(udcDialogBinding.getRoot());
        });
    promptDialogManager.addView(udcDialogBinding.getRoot(), this::OnPromptDeclined);
  }

  private void handleCommissionPinCode(android.os.Message msg) {
    String commissioneeName = (String) msg.obj;

    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(false)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 350))
            .setStyle(PromptDialogManager.DIALOG_STYLE_2)
            .build();

    LayoutInputDialogBinding inputDialogBinding =
        LayoutInputDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    EditText etPin = inputDialogBinding.etPin;
    inputDialogBinding.tvTitle.setText("Allow access to " + commissioneeName);
    inputDialogBinding.tvContent.setText("Please enter PIN displayed in casting app.");
    inputDialogBinding.tvLeft.setOnClickListener(
        view -> {
          OnPinCodeDeclined();
          promptDialogManager.removeViewImmediate(inputDialogBinding.getRoot());
        });
    inputDialogBinding.tvRight.setOnClickListener(
        view -> {
          String pinCode = etPin.getText().toString();
          OnPinCodeEntered(Integer.parseInt(pinCode));
          promptDialogManager.removeViewImmediate(inputDialogBinding.getRoot());
        });
    promptDialogManager.addView(inputDialogBinding.getRoot(), this::OnPinCodeDeclined);
  }

  private void handleHidePromptsOnCancel(android.os.Message msg) {
    String commissioneeName = (String) msg.obj;

    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(true)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 300))
            .setStyle(PromptDialogManager.DIALOG_STYLE_3)
            .build();

    LayoutDialogBinding dialogBinding =
        LayoutDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    dialogBinding.tvContent.setText("Cancelled connection to " + commissioneeName);
    dialogBinding.tvTitle.setText("Connection Cancelled");
    dialogBinding.llAction.setVisibility(View.GONE);
    promptDialogManager.addView(dialogBinding.getRoot(), null);

    NotificationCompat.Builder builder =
        new NotificationCompat.Builder(getContext(), CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_baseline_check_24)
            .setContentTitle("Connection Cancelled")
            .setContentText("Cancelled connection to " + commissioneeName)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT);

    notificationManager.notify(SUCCESS_ID, builder.build());
  }

  private void handleCommissionerPasscode(android.os.Message msg) {
    Bundle bundle = msg.getData();
    PromptCommissionerPasscode parcelable =
        bundle.getParcelable(MsgHandler.KEY_PROMPT_COMMISSIONER_PASSCODE);

    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(false)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 350))
            .setStyle(PromptDialogManager.DIALOG_STYLE_1)
            .build();

    LayoutDialogBinding udcDialogBinding =
        LayoutDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    udcDialogBinding.tvContent.setText(
        "Please enter "
            + parcelable.getPasscode()
            + " in "
            + parcelable.getCommissioneeName()
            + " app. "
            + parcelable.getPairingInstruction()
            + " ["
            + parcelable.getPairingHint()
            + "]");
    udcDialogBinding.tvTitle.setText("Passcode" + parcelable.getPasscode());
    udcDialogBinding.tvLeft.setOnClickListener(
        v -> {
          OnCommissionerPasscodeCancel();
          promptDialogManager.removeViewImmediate(udcDialogBinding.getRoot());
        });
    udcDialogBinding.tvRight.setOnClickListener(
        v -> {
          OnCommissionerPasscodeOK();
          promptDialogManager.removeViewImmediate(udcDialogBinding.getRoot());
        });
    promptDialogManager.addView(udcDialogBinding.getRoot(), this::OnCommissionerPasscodeCancel);
  }

  private void handleCommissioningStarted(android.os.Message msg) {
    String commissioneeName = (String) msg.obj;

    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(true)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 300))
            .setStyle(PromptDialogManager.DIALOG_STYLE_3)
            .build();

    LayoutDialogBinding dialogBinding =
        LayoutDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    dialogBinding.tvContent.setText("Starting connection to " + commissioneeName);
    dialogBinding.tvTitle.setText("Connection Starting");
    dialogBinding.llAction.setVisibility(View.GONE);
    promptDialogManager.addView(dialogBinding.getRoot(), null);

    NotificationCompat.Builder builder =
        new NotificationCompat.Builder(getContext(), CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_baseline_check_24)
            .setContentTitle("Connection Starting")
            .setContentText("Starting connection to " + commissioneeName)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT);

    notificationManager.notify(SUCCESS_ID, builder.build());
  }

  private void handleCommissioningSucceeded(android.os.Message msg) {
    String commissioneeName = (String) msg.obj;
    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(true)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 300))
            .setStyle(PromptDialogManager.DIALOG_STYLE_3)
            .build();

    LayoutDialogBinding dialogBinding =
        LayoutDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    dialogBinding.tvContent.setText(
        "Success. "
            + commissioneeName
            + " can now cast to this device. Visit settings to manage access control for casting.");
    dialogBinding.tvTitle.setText("Connection Complete");
    dialogBinding.llAction.setVisibility(View.GONE);
    promptDialogManager.addView(dialogBinding.getRoot(), null);

    NotificationCompat.Builder builder =
        new NotificationCompat.Builder(getContext(), CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_baseline_check_24)
            .setContentTitle("Connection Complete")
            .setContentText(
                "Success. "
                    + commissioneeName
                    + " can now cast to this device. Visit settings to manage access control for casting.")
            .setPriority(NotificationCompat.PRIORITY_DEFAULT);

    notificationManager.notify(SUCCESS_ID, builder.build());
  }

  private void handleCommissioningFailed(android.os.Message msg) {
    String commissioneeName = (String) msg.obj;
    String error = (String) msg.obj;
    AlertDialog.Builder abuilder = new AlertDialog.Builder(getContext());
    AlertDialog alertDialog =
        abuilder
            .setMessage("Failed. " + commissioneeName + " experienced error: " + error + ".")
            .setTitle("Connection Failed")
            .create();
    alertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_PANEL);
    alertDialog.show();

    NotificationCompat.Builder builder =
        new NotificationCompat.Builder(getContext(), CHANNEL_ID)
            .setSmallIcon(R.drawable.ic_baseline_clear_24)
            .setContentTitle("Connection Failed")
            .setContentText("Failed. " + commissioneeName + " experienced error: " + error + ".")
            .setPriority(NotificationCompat.PRIORITY_DEFAULT);

    notificationManager.notify(FAIL_ID, builder.build());
  }

  private void handlePromptWithMessage(android.os.Message msg) {
    Message message = (Message) msg.obj;

    PromptDialogManager promptDialogManager =
        new PromptDialogManager.Builder()
            .setCancelable(false)
            .setWidth(PxConvert.dp2px(MatterTvServerApplication.getApplication(), 380))
            .setStyle(PromptDialogManager.DIALOG_STYLE_1)
            .build();

    LayoutDialogBinding dialogBinding =
        LayoutDialogBinding.inflate(
            LayoutInflater.from(
                MatterTvServerApplication.getApplication().getApplicationContext()));
    dialogBinding.tvContent.setText("" + message.messageId + ":" + message.messageText);
    dialogBinding.tvTitle.setText("New Message from Test");
    if (message.responseOptions.length != 2) {
      dialogBinding.tvLeft.setText("Ignore");
      dialogBinding.tvRight.setText("Ok");
      dialogBinding.tvLeft.setOnClickListener(
          v -> {
            OnMessageResponse(message.messageId, -1);
            promptDialogManager.removeViewImmediate(dialogBinding.getRoot());
          });
      dialogBinding.tvRight.setOnClickListener(
          v -> {
            OnMessageResponse(message.messageId, 0);
            promptDialogManager.removeViewImmediate(dialogBinding.getRoot());
          });
    } else {
      dialogBinding.tvLeft.setText(message.responseOptions[1].label);
      dialogBinding.tvRight.setText(message.responseOptions[0].label);
      dialogBinding.tvLeft.setOnClickListener(
          v -> {
            OnMessageResponse(message.messageId, message.responseOptions[1].id);
            promptDialogManager.removeViewImmediate(dialogBinding.getRoot());
          });
      dialogBinding.tvRight.setOnClickListener(
          v -> {
            OnMessageResponse(message.messageId, message.responseOptions[0].id);
            promptDialogManager.removeViewImmediate(dialogBinding.getRoot());
          });
    }
    promptDialogManager.addView(
        dialogBinding.getRoot(),
        () -> {
          if (message.responseOptions.length != 2) {
            OnMessageResponse(message.messageId, -1);
            promptDialogManager.removeViewImmediate(dialogBinding.getRoot());
          } else {
            OnMessageResponse(message.messageId, message.responseOptions[1].id);
            promptDialogManager.removeViewImmediate(dialogBinding.getRoot());
          }
        });
  }

  private void createNotificationChannel() {
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
  }

  private Context getContext() {
    return MatterTvServerApplication.getApplication();
  }

  static class MsgHandler extends Handler {
    public static final int MSG_CommissionOkPermission = 101;
    public static final int MSG_CommissionPinCode = 102;
    public static final int MSG_HidePromptsOnCancel = 103;
    public static final int MSG_CommissionerPasscode = 104;
    public static final int MSG_CommissioningStarted = 105;
    public static final int MSG_CommissioningSucceeded = 106;
    public static final int MSG_CommissioningFailed = 107;
    public static final int MSG_PromptWithMessage = 108;
    public static final String KEY_PROMPT_COMMISSIONER_PASSCODE =
        "KEY_PROMPT_COMMISSIONER_PASSCODE";
    private final WeakReference<MatterCommissioningPrompter> mPrompterWeakReference;

    public MsgHandler(MatterCommissioningPrompter commissioningPrompter) {
      super(Looper.getMainLooper());
      mPrompterWeakReference = new WeakReference<>(commissioningPrompter);
    }

    @Override
    public void handleMessage(@NonNull android.os.Message msg) {
      MatterCommissioningPrompter commissioningPrompter = mPrompterWeakReference.get();
      if (commissioningPrompter == null) {
        return;
      }
      if (msg.what == MSG_CommissionOkPermission) {
        commissioningPrompter.handleCommissionOkPermission(msg);
      } else if (msg.what == MSG_CommissionPinCode) {
        commissioningPrompter.handleCommissionPinCode(msg);
      } else if (msg.what == MSG_HidePromptsOnCancel) {
        commissioningPrompter.handleHidePromptsOnCancel(msg);
      } else if (msg.what == MSG_CommissionerPasscode) {
        commissioningPrompter.handleCommissionerPasscode(msg);
      } else if (msg.what == MSG_CommissioningStarted) {
        commissioningPrompter.handleCommissioningStarted(msg);
      } else if (msg.what == MSG_CommissioningSucceeded) {
        commissioningPrompter.handleCommissioningSucceeded(msg);
      } else if (msg.what == MSG_CommissioningFailed) {
        commissioningPrompter.handleCommissioningFailed(msg);
      } else if (msg.what == MSG_PromptWithMessage) {
        commissioningPrompter.handlePromptWithMessage(msg);
      }
    }
  }
}
