/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

package com.matter.casting.support;

import android.os.Build;
import androidx.annotation.RequiresApi;
import androidx.annotation.VisibleForTesting;
import java.lang.ref.Cleaner;
import java.lang.ref.PhantomReference;
import java.lang.ref.ReferenceQueue;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Manages native resource cleanup for Matter Java objects that wrap native pointers.
 *
 * <p>Any Matter Java object holding a native resource can register a cleanup action via {@link
 * #register(Object, Runnable)}. The action runs when the Java object becomes phantom-reachable,
 * freeing the associated native memory without requiring explicit {@code close()} calls from
 * callers.
 *
 * <p>On API 33 and above, delegates to {@link java.lang.ref.Cleaner}. On API 26–32, uses a {@link
 * PhantomReference} and {@link ReferenceQueue} with a dedicated daemon thread.
 */
public final class MatterCleaner {

  private static final MatterCleaner INSTANCE = new MatterCleaner();

  private MatterCleaner() {}

  public static MatterCleaner getInstance() {
    return INSTANCE;
  }

  /**
   * Registers a cleanup action to run when {@code obj} becomes phantom-reachable.
   *
   * <p>The {@code action} must not hold a reference to {@code obj}, directly or indirectly, as this
   * would prevent the object from ever becoming phantom-reachable.
   *
   * @param obj the object whose reachability triggers the action
   * @param action the cleanup action; must not capture {@code obj}
   */
  public void register(Object obj, Runnable action) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      CleanerHolder.CLEANER.register(obj, action);
    } else {
      LegacyCleanerHolder.LIVE_REFS.add(new LegacyCleanerHolder.CleanupRef(obj, action));
    }
  }

  /**
   * Holder initialized on first access, which only occurs on API 33+. This prevents
   * NoClassDefFoundError on earlier API levels.
   */
  @RequiresApi(Build.VERSION_CODES.TIRAMISU)
  private static final class CleanerHolder {
    static final Cleaner CLEANER = Cleaner.create();
  }

  /**
   * Holder initialized on first access, which only occurs on API 26–32.
   *
   * <p>A strong reference to each {@link CleanupRef} is kept in {@link #LIVE_REFS} until the
   * referent is collected. Without this, the phantom reference itself could be collected before its
   * referent and never enqueued.
   */
  @VisibleForTesting
  static final class LegacyCleanerHolder {
    static final ReferenceQueue<Object> QUEUE = new ReferenceQueue<>();
    static final Set<CleanupRef> LIVE_REFS = Collections.newSetFromMap(new ConcurrentHashMap<>());

    static final class CleanupRef extends PhantomReference<Object> {
      final Runnable action;

      CleanupRef(Object referent, Runnable action) {
        super(referent, QUEUE);
        this.action = action;
      }
    }

    static {
      Thread t =
          new Thread(
              () -> {
                while (!Thread.currentThread().isInterrupted()) {
                  try {
                    CleanupRef ref = (CleanupRef) QUEUE.remove();
                    LIVE_REFS.remove(ref);
                    ref.action.run();
                  } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                  } catch (Throwable ignored) {
                    // Matches Cleaner convention: exceptions from cleanup actions are ignored.
                  }
                }
              },
              "MatterCleaner");
      t.setDaemon(true);
      t.start();
    }
  }
}
