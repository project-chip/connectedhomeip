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

import androidx.annotation.NonNull;
import androidx.lifecycle.LiveData;
import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.OnConflictStrategy;
import androidx.room.Query;
import java.util.List;

@Dao
interface BorderAgentDao {

  @Query("SELECT * FROM border_agent_table ORDER BY network_name ASC")
  LiveData<List<BorderAgentRecord>> getAll();

  @Query("SELECT * FROM border_agent_table WHERE discriminator=:discriminator LIMIT 1")
  BorderAgentRecord getBorderAgent(@NonNull String discriminator);

  @Query(
      "SELECT * FROM border_agent_table WHERE network_name=:networkName AND extended_pan_id=:extendedPanId")
  List<BorderAgentRecord> getBorderAgents(
      @NonNull String networkName, @NonNull byte[] extendedPanId);

  @Insert(onConflict = OnConflictStrategy.IGNORE)
  void insert(BorderAgentRecord borderAgentRecord);

  @Query("DELETE FROM border_agent_table WHERE discriminator=:discriminator")
  void delete(@NonNull String discriminator);

  @Query("DELETE FROM border_agent_table")
  void deleteAll();
}
