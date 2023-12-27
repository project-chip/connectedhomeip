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
import com.matter.casting.support.MatterError;
import java.util.List;

/**
 * The CastingPlayerDiscovery interface defines the API to control Matter Casting Player discovery
 * over DNS-SD, and to collect discovery results. Discovery is centrally managed by the native C++
 * layer in the Matter SDK. This class exposes native functions to add and remove a
 * CastingPlayerChangeListener, which contains the C++ to Java callbacks for when Casting Players
 * are discovered, updated, or lost from the network. This class is a singleton.
 */
public interface CastingPlayerDiscovery {

  /**
   * @return a list of Casting Players discovered during the current discovery session. This list is
   *     cleared when discovery stops.
   */
  List<CastingPlayer> getCastingPlayers();

  /**
   * Starts Casting Players discovery or returns an error.
   *
   * @param discoveryTargetDeviceType the target device type to be discovered using DNS-SD. 35
   *     represents device type of Matter Casting Player.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError startDiscovery(int discoveryTargetDeviceType);

  /**
   * Stops Casting Players discovery or returns an error.
   *
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError stopDiscovery();

  /**
   * Adds a CastingPlayerChangeListener instance to be used during discovery. The
   * CastingPlayerChangeListener contains the C++ to Java callbacks for when Casting Players are
   * discovered, updated, or lost from the network. Should be called prior to calling
   * MatterCastingPlayerDiscovery.startDiscovery().
   *
   * @param listener an instance of the CastingPlayerChangeListener to be implemented by the APIs
   *     consumer.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError addCastingPlayerChangeListener(CastingPlayerChangeListener listener);

  /**
   * Removes CastingPlayerChangeListener from the native layer.
   *
   * @param listener the specific instance of CastingPlayerChangeListener to be removed.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError removeCastingPlayerChangeListener(CastingPlayerChangeListener listener);

  /**
   * The CastingPlayerChangeListener can discover CastingPlayers by implementing the onAdded,
   * onChanged and onRemoved callbacks which are called as CastingPlayers, are discovered, updated,
   * or lost from the network. The onAdded(), onChanged() and onRemoved() callbacks must be
   * implemented by the APIs client.
   */
  abstract class CastingPlayerChangeListener {
    static final String TAG = CastingPlayerChangeListener.class.getSimpleName();

    /**
     * Called by the native C++ layer when a Casting Player is added to the local network.
     *
     * @param castingPlayer the Casting Player added.
     */
    public abstract void onAdded(CastingPlayer castingPlayer);

    /**
     * Called by the native C++ layer when a Casting Player on the local network is changed.
     *
     * @param castingPlayer the Casting Player changed.
     */
    public abstract void onChanged(CastingPlayer castingPlayer);

    /**
     * Called by the native C++ layer when a Casting Player is removed from the local network.
     *
     * @param castingPlayer the Casting Player removed.
     */
    public abstract void onRemoved(CastingPlayer castingPlayer);

    /**
     * The following methods are used to catch possible exceptions thrown by the methods above, when
     * not implemented correctly.
     */
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
