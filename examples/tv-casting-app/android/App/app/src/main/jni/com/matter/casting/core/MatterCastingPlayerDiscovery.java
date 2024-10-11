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

import com.matter.casting.support.MatterError;
import java.util.List;

/**
 * MatterCastingPlayerDiscovery provides an API to control Matter Casting Player discovery over
 * DNS-SD, and to collect discovery results. This class provides methods to add and remove a
 * CastingPlayerChangeListener, which contains the handlers for when Casting Players are discovered,
 * updated, or lost from the network. This class is a singleton.
 */
public final class MatterCastingPlayerDiscovery implements CastingPlayerDiscovery {
  private static final String TAG = MatterCastingPlayerDiscovery.class.getSimpleName();
  private static MatterCastingPlayerDiscovery matterCastingPlayerDiscoveryInstance;

  // Methods:
  public static MatterCastingPlayerDiscovery getInstance() {
    if (matterCastingPlayerDiscoveryInstance == null) {
      matterCastingPlayerDiscoveryInstance = new MatterCastingPlayerDiscovery();
    }
    return matterCastingPlayerDiscoveryInstance;
  };

  /**
   * @return a list of Casting Players discovered during the current discovery session. This list is
   *     cleared when discovery stops.
   */
  @Override
  public native List<CastingPlayer> getCastingPlayers();

  /**
   * Starts Casting Players discovery or returns an error.
   *
   * @param discoveryTargetDeviceType the target device type to be discovered using DNS-SD. For
   *     example: 35 represents device type of Matter Casting Video Player. If "null" is passed in,
   *     discovery will default to all "_matterd._udp" device types.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  @Override
  public native MatterError startDiscovery(Long discoveryTargetDeviceType);

  /**
   * Stops Casting Players discovery or returns an error.
   *
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  @Override
  public native MatterError stopDiscovery();

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
  @Override
  public native MatterError addCastingPlayerChangeListener(CastingPlayerChangeListener listener);

  /**
   * Removes CastingPlayerChangeListener from the native layer.
   *
   * @param listener the specific instance of CastingPlayerChangeListener to be removed.
   * @return a specific MatterError if the the operation failed or NO_ERROR if succeeded.
   */
  @Override
  public native MatterError removeCastingPlayerChangeListener(CastingPlayerChangeListener listener);
}
