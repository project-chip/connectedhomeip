package com.google.chip.chiptool.commissioner.thread.internal;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.room.Database;
import androidx.room.Room;
import androidx.room.RoomDatabase;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkCredential;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkInfo;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Database(entities = {BorderAgentRecord.class}, version = 1, exportSchema = false)
abstract class BorderAgentDatabase extends RoomDatabase {

  abstract BorderAgentDao borderAgentDao();

  private static volatile BorderAgentDatabase INSTANCE;
  private static final int NUMBER_OF_THREADS = 2;

  static final ExecutorService executor = Executors.newFixedThreadPool(NUMBER_OF_THREADS);

  public static BorderAgentDatabase getDatabase(final Context context) {
    if (INSTANCE == null) {
      synchronized (BorderAgentDatabase.class) {
        if (INSTANCE == null) {
          INSTANCE = Room.databaseBuilder(context.getApplicationContext(), BorderAgentDatabase.class, "network_credential_database").build();
        }
      }
    }
    return INSTANCE;
  }

  public CompletableFuture<BorderAgentRecord> getBorderAgent(@NonNull String discriminator) {
    CompletableFuture<BorderAgentRecord> future = CompletableFuture.supplyAsync(() ->
        borderAgentDao().getBorderAgent(discriminator));
    return future;
   }

  public CompletableFuture<Void> insertBorderAgent(@NonNull BorderAgentRecord borderAgentRecord) {
    CompletableFuture<Void> future = CompletableFuture.runAsync(() -> {
      borderAgentDao().insert(borderAgentRecord);
    }, executor);
    return future;
  }

  public CompletableFuture<Void> deleteBorderAgent(@NonNull String discriminator) {
    CompletableFuture<Void> future = CompletableFuture.runAsync(() -> {
      borderAgentDao().delete(discriminator);
    }, executor);
    return future;
  }

}
