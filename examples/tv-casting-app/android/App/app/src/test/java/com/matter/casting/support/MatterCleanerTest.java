/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package com.matter.casting.support;

import static org.junit.Assert.*;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

/**
 * Unit tests for {@link MatterCleaner}.
 *
 * <p>The legacy path (API 26–32) is tested by manually enqueuing phantom references via {@link
 * java.lang.ref.PhantomReference#enqueue()}, simulating GC-driven cleanup without relying on the
 * garbage collector.
 */
@RunWith(RobolectricTestRunner.class)
@Config(sdk = 28)
public class MatterCleanerTest {

  @Before
  public void setUp() {
    MatterCleaner.LegacyCleanerHolder.LIVE_REFS.clear();
  }

  @Test
  public void register_addsToLiveRefs() {
    MatterCleaner.getInstance().register(new Object(), () -> {});
    assertEquals(1, MatterCleaner.LegacyCleanerHolder.LIVE_REFS.size());
  }

  @Test
  public void register_actionCalledWhenRefEnqueued() throws Exception {
    CountDownLatch latch = new CountDownLatch(1);
    MatterCleaner.getInstance().register(new Object(), latch::countDown);

    enqueueFirst();

    assertTrue("action was not called within timeout", latch.await(2, TimeUnit.SECONDS));
  }

  @Test
  public void register_liveRefsEmptiedAfterCleanup() throws Exception {
    CountDownLatch latch = new CountDownLatch(1);
    MatterCleaner.getInstance().register(new Object(), latch::countDown);
    assertEquals(1, MatterCleaner.LegacyCleanerHolder.LIVE_REFS.size());

    enqueueFirst();

    assertTrue("action was not called within timeout", latch.await(2, TimeUnit.SECONDS));
    assertEquals(0, MatterCleaner.LegacyCleanerHolder.LIVE_REFS.size());
  }

  @Test
  public void register_cleanupThreadSurvivesThrowingAction() throws Exception {
    CountDownLatch firstProcessed = new CountDownLatch(1);
    MatterCleaner.getInstance()
        .register(
            new Object(),
            () -> {
              firstProcessed.countDown();
              throw new RuntimeException("test exception");
            });
    enqueueFirst();
    assertTrue("first action was not processed within timeout", firstProcessed.await(2, TimeUnit.SECONDS));

    // Thread must still be alive — a second action should fire normally.
    CountDownLatch secondProcessed = new CountDownLatch(1);
    MatterCleaner.getInstance().register(new Object(), secondProcessed::countDown);
    enqueueFirst();
    assertTrue("second action was not called within timeout", secondProcessed.await(2, TimeUnit.SECONDS));
  }

  // ---------------------------------------------------------------------------
  // Helpers
  // ---------------------------------------------------------------------------

  private static void enqueueFirst() {
    MatterCleaner.LegacyCleanerHolder.LIVE_REFS.iterator().next().enqueue();
  }
}
