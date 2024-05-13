package com.matter.tv.server.service;

import android.util.Log;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class ResponseRegistry {

  public enum WaitState {
    TIMED_OUT,
    INTERRUPTED,
    SUCCESS,
    INVALID_COUNTER
  }

  private static final String TAG = "ResponseRegistry";

  private AtomicInteger messageCounter = new AtomicInteger();

  private Map<Integer, String> responses = new ConcurrentHashMap<>();

  private Map<Integer, CountDownLatch> latches = new ConcurrentHashMap<>();

  public int getNextMessageCounter() {
    int counter = messageCounter.incrementAndGet();
    // MAX_VALUE used for error scenarios
    if (counter == Integer.MAX_VALUE) {
      counter = messageCounter.incrementAndGet();
    }
    latches.put(counter, new CountDownLatch(1));
    return counter;
  }

  public WaitState waitForMessage(int counter, long timeout, TimeUnit unit) {
    CountDownLatch latch = latches.get(counter);
    if (latch == null) {
      return WaitState.INVALID_COUNTER;
    }
    try {
      if (!latch.await(timeout, unit)) {
        Log.i(TAG, "Timed out while waiting for response for message " + counter);
        return WaitState.TIMED_OUT;
      }
    } catch (InterruptedException e) {
      Log.i(TAG, "Interrupted while waiting for response for message " + counter);
      return WaitState.INTERRUPTED;
    }
    return WaitState.SUCCESS;
  }

  public void receivedMessageResponse(int counter, String response) {
    CountDownLatch latch = latches.remove(counter);
    if (latch == null) {
      // no point adding response to memory if no one is going to read it.
      return;
    }
    responses.put(counter, response);
    latch.countDown();
  }

  public String readAndRemoveResponse(int counter) {
    // caller should manage null values
    return responses.remove(counter);
  }
}
