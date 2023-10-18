/*
 *   Copyright (c) 2023 Project CHIP Authors
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
package com.matter.casting.core;

import android.util.Log;
import java.util.List;

public interface CastingPlayerDiscovery {

  List<CastingPlayer> getCastingPlayers();

  com.matter.casting.support.MatterError startDiscovery();

  com.matter.casting.support.MatterError stopDiscovery();

  com.matter.casting.support.MatterError addCastingPlayerChangeListener(
      CastingPlayerChangeListener listener);

  com.matter.casting.support.MatterError removeCastingPlayerChangeListener(
      CastingPlayerChangeListener listener);

  /**
   * The Casting Client discovers CastingPlayers using Matter Commissioner discovery over DNS-SD by
   * listening for CastingPlayer events as they are discovered, updated, or lost from the network.
   */
  abstract class CastingPlayerChangeListener {
    static final String TAG = CastingPlayerChangeListener.class.getSimpleName();

    public abstract void onAdded(CastingPlayer castingPlayer);

    public abstract void onChanged(CastingPlayer castingPlayer);

    public abstract void onRemoved(CastingPlayer castingPlayer);

    protected final void _onAdded(CastingPlayer castingPlayer) {
      try {
        onAdded(castingPlayer);
      } catch (Throwable t) {
        Log.e(TAG, "_onAdded() Caught an unhandled Throwable from the client: " + t);
      }
    };

    protected final void _onChanged(CastingPlayer castingPlayer) {
      try {
        onChanged(castingPlayer);
      } catch (Throwable t) {
        Log.e(TAG, "_onChanged() Caught an unhandled Throwable from the client: " + t);
      }
    };

    protected final void _onRemoved(CastingPlayer castingPlayer) {
      try {
        onRemoved(castingPlayer);
      } catch (Throwable t) {
        Log.e(TAG, "_onRemoved() Caught an unhandled Throwable from the client: " + t);
      }
    };
  }
}
