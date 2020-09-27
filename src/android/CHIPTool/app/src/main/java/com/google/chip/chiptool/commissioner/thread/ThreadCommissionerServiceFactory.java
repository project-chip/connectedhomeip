package com.google.chip.chiptool.commissioner.thread;

import android.content.Context;
import com.google.chip.chiptool.commissioner.thread.internal.ThreadCommissionerServiceImpl;

class ThreadCommissionerServiceFactory {
  public ThreadCommissionerService getCommissionerService(Context context) {
    return new ThreadCommissionerServiceImpl(context);
  }
}
