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
 * The CastingPlayerDiscovery interface defines the client API to control Matter Casting Player
 * discovery over DNS-SD, and to collect discovery results. This interface defines the methods to
 * add and remove a CastingPlayerChangeListener. It also defines the CastingPlayerChangeListener
 * handler class which must be implemented by the API client. The handler contains the methods
 * called when Casting Players are discovered, updated, or lost from the network.
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
   * @param discoveryTargetDeviceType the target device type to be discovered using DNS-SD. For
   *     example: 35 represents device type of Matter Casting Video Player. If "null" is passed in,
   *     discovery will default to all "_matterd._udp" device types.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError startDiscovery(Long discoveryTargetDeviceType);

  /**
   * Stops Casting Players discovery or returns an error.
   *
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError stopDiscovery();

  /**
   * Adds a CastingPlayerChangeListener instance to be used during discovery. The
   * CastingPlayerChangeListener defines the handler methods for when Casting Players are
   * discovered, updated, or lost from the network. Should be called prior to calling
   * MatterCastingPlayerDiscovery.startDiscovery().
   *
   * @param listener an instance of the CastingPlayerChangeListener to be implemented by the APIs
   *     consumer.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError addCastingPlayerChangeListener(CastingPlayerChangeListener listener);

  /**
   * Removes CastingPlayerChangeListener added by addCastingPlayerChangeListener().
   *
   * @param listener the specific instance of CastingPlayerChangeListener to be removed.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  MatterError removeCastingPlayerChangeListener(CastingPlayerChangeListener listener);

  /**
   * The CastingPlayerChangeListener can discover CastingPlayers by implementing the onAdded(),
   * onChanged() and onRemoved() callbacks which are called as CastingPlayers, are discovered,
   * updated, or lost from the network. The onAdded(), onChanged() and onRemoved() handlers must be
   * implemented by the API client.
   */
  abstract class CastingPlayerChangeListener {
    static final String TAG = CastingPlayerChangeListener.class.getSimpleName();

    /**
     * This handler is called when a Casting Player is added to the network.
     *
     * @param castingPlayer the Casting Player added.
     */
    public abstract void onAdded(CastingPlayer castingPlayer);

    /**
     * This handler is called when a Casting Player previously detected on the network is changed.
     *
     * @param castingPlayer the Casting Player changed.
     */
    public abstract void onChanged(CastingPlayer castingPlayer);

    /**
     * This handler is called when a Casting Player previously detected on the network is removed.
     *
     * @param castingPlayer the Casting Player removed.
     */
    public abstract void onRemoved(CastingPlayer castingPlayer);

    /**
     * The following methods are used to catch possible exceptions thrown by the methods above
     * (onAdded(), onChanged() and onRemoved()), when not implemented correctly by the client.
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
