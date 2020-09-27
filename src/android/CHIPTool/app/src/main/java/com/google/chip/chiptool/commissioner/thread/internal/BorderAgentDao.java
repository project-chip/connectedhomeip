package com.google.chip.chiptool.commissioner.thread.internal;

import androidx.annotation.NonNull;
import androidx.lifecycle.LiveData;
import androidx.room.Dao;
import androidx.room.Delete;
import androidx.room.Insert;
import androidx.room.OnConflictStrategy;
import androidx.room.Query;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkCredential;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkInfo;
import java.util.List;

@Dao
interface BorderAgentDao {

  @Query("SELECT * FROM border_agent_table ORDER BY network_name ASC")
  LiveData<List<BorderAgentRecord>> getAll();

  @Query("SELECT * FROM border_agent_table WHERE discriminator=:discriminator LIMIT 1")
  BorderAgentRecord getBorderAgent(@NonNull String discriminator);

  @Query("SELECT * FROM border_agent_table WHERE network_name=:networkName AND extended_pan_id=:extendedPanId")
  List<BorderAgentRecord> getBorderAgents(@NonNull String networkName, @NonNull byte[] extendedPanId);

  @Insert(onConflict = OnConflictStrategy.IGNORE)
  void insert(BorderAgentRecord borderAgentRecord);

  @Query("DELETE FROM border_agent_table WHERE discriminator=:discriminator")
  void delete(@NonNull String discriminator);

  @Query("DELETE FROM border_agent_table")
  void deleteAll();
}
