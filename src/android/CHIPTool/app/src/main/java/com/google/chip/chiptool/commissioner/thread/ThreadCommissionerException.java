package com.google.chip.chiptool.commissioner.thread;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;

public class ThreadCommissionerException extends Exception {

  @NonNull
  private int code;

  @NonNull
  private String message;

  public ThreadCommissionerException(int code, String message) {
    this.code = code;
    this.message = message;
  }

  public int getCode() {
    return code;
  }

  public String getMessage() {
    return message;
  }

  @Override
  public String toString() {
    return String.format("[ %d ]: %s", code, message);
  }
}
