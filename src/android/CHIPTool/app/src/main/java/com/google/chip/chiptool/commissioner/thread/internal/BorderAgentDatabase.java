/*
 *   Copyright (c) 2020 Project CHIP Authors
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
 *
 */

package com.google.chip.chiptool.commissioner.thread.internal;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.room.Database;
import androidx.room.Room;
import androidx.room.RoomDatabase;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Database(
    entities = {BorderAgentRecord.class},
    version = 1,
    exportSchema = false)
abstract class BorderAgentDatabase extends RoomDatabase {

  abstract BorderAgentDao borderAgentDao();

  private static volatile BorderAgentDatabase INSTANCE;
  private static final int NUMBER_OF_THREADS = 2;

  static final ExecutorService executor = Executors.newFixedThreadPool(NUMBER_OF_THREADS);

  public static BorderAgentDatabase getDatabase(final Context context) {
    if (INSTANCE == null) {
      synchronized (BorderAgentDatabase.class) {
        if (INSTANCE == null) {
          INSTANCE =
              Room.databaseBuilder(
                      context.getApplicationContext(),
                      BorderAgentDatabase.class,
                      "network_credential_database")
                  .build();
        }
      }
    }
    return INSTANCE;
  }

  public CompletableFuture<BorderAgentRecord> getBorderAgent(@NonNull String discriminator) {
    CompletableFuture<BorderAgentRecord> future =
        CompletableFuture.supplyAsync(() -> borderAgentDao().getBorderAgent(discriminator));
    return future;
  }

  public CompletableFuture<Void> insertBorderAgent(@NonNull BorderAgentRecord borderAgentRecord) {
    CompletableFuture<Void> future =
        CompletableFuture.runAsync(
            () -> {
              borderAgentDao().insert(borderAgentRecord);
            },
            executor);
    return future;
  }

  public CompletableFuture<Void> deleteBorderAgent(@NonNull String discriminator) {
    CompletableFuture<Void> future =
        CompletableFuture.runAsync(
            () -> {
              borderAgentDao().delete(discriminator);
            },
            executor);
    return future;
  }
}
