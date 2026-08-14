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

package com.matter.casting.core;

import static org.junit.Assert.*;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

/**
 * Unit tests for {@link MatterCastingPlayer#getConnectionState()}.
 *
 * <p>Verifies that getConnectionState() always returns a valid {@link
 * CastingPlayer.ConnectionState} and never throws, including when the native layer returns an
 * invalid or null string.
 *
 * <p>Robolectric is used to stub Android APIs (e.g. {@code android.util.Log}) without requiring a
 * device.
 */
@RunWith(RobolectricTestRunner.class)
public class MatterCastingPlayerGetConnectionStateTest {

  /**
   * Test subclass that overrides {@code getConnectionStateNative()} to return a controlled string
   * without loading the native library.
   */
  private static class FakeMatterCastingPlayer extends MatterCastingPlayer {

    private final String nativeStateString;

    FakeMatterCastingPlayer(String nativeStateString) {
      super(
          /* connected= */ false,
          /* deviceId= */ "test-device-id",
          /* hostName= */ "test-host",
          /* deviceName= */ "test-device",
          /* instanceName= */ "test-instance",
          /* ipAddresses= */ null,
          /* port= */ 0,
          /* productId= */ 0,
          /* vendorId= */ 0,
          /* deviceType= */ 0L,
          /* supportsCommissionerGeneratedPasscode= */ false);
      this.nativeStateString = nativeStateString;
    }

    @Override
    public String getConnectionStateNative() {
      return nativeStateString;
    }
  }

  // ---------------------------------------------------------------------------
  // Valid states — native returns a well-formed string
  // ---------------------------------------------------------------------------

  @Test
  public void getConnectionState_whenNativeReturnsNotConnected_returnsNotConnected() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer("NOT_CONNECTED");
    assertEquals(CastingPlayer.ConnectionState.NOT_CONNECTED, player.getConnectionState());
  }

  @Test
  public void getConnectionState_whenNativeReturnsConnecting_returnsConnecting() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer("CONNECTING");
    assertEquals(CastingPlayer.ConnectionState.CONNECTING, player.getConnectionState());
  }

  @Test
  public void getConnectionState_whenNativeReturnsConnected_returnsConnected() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer("CONNECTED");
    assertEquals(CastingPlayer.ConnectionState.CONNECTED, player.getConnectionState());
  }

  // ---------------------------------------------------------------------------
  // Invalid states — native returns a string that is not a valid enum constant.
  // All of these caused a fatal IllegalArgumentException before the fix.
  // ---------------------------------------------------------------------------

  @Test
  public void getConnectionState_whenNativeReturnsNullptrErrorString_returnsNotConnected() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer("Cast Player is nullptr");
    assertEquals(CastingPlayer.ConnectionState.NOT_CONNECTED, player.getConnectionState());
  }

  @Test
  public void getConnectionState_whenNativeReturnsUnsupportedStateString_returnsNotConnected() {
    FakeMatterCastingPlayer player =
        new FakeMatterCastingPlayer("Unsupported Connection State: 99");
    assertEquals(CastingPlayer.ConnectionState.NOT_CONNECTED, player.getConnectionState());
  }

  @Test
  public void getConnectionState_whenNativeReturnsArbitraryString_returnsNotConnected() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer("GARBAGE");
    assertEquals(CastingPlayer.ConnectionState.NOT_CONNECTED, player.getConnectionState());
  }

  @Test
  public void getConnectionState_whenNativeReturnsEmptyString_returnsNotConnected() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer("");
    assertEquals(CastingPlayer.ConnectionState.NOT_CONNECTED, player.getConnectionState());
  }

  // ---------------------------------------------------------------------------
  // Null — native returns null, causing NullPointerException in valueOf(null).
  // ---------------------------------------------------------------------------

  @Test
  public void getConnectionState_whenNativeReturnsNull_returnsNotConnected() {
    FakeMatterCastingPlayer player = new FakeMatterCastingPlayer(null);
    assertEquals(CastingPlayer.ConnectionState.NOT_CONNECTED, player.getConnectionState());
  }

  // ---------------------------------------------------------------------------
  // No-throw guarantee — getConnectionState() must never propagate an exception
  // regardless of what the native layer returns.
  // ---------------------------------------------------------------------------

  @Test
  public void getConnectionState_neverThrowsForAnyNativeString() {
    String[] inputs = {
      null,
      "",
      "Cast Player is nullptr",
      "Unsupported Connection State: 99",
      "GARBAGE",
      "0",
      "-1",
      "not_connected",       // wrong case
      " NOT_CONNECTED ",     // extra whitespace
      "NOT_CONNECTED\n",     // trailing newline
    };

    for (String input : inputs) {
      FakeMatterCastingPlayer player = new FakeMatterCastingPlayer(input);
      try {
        CastingPlayer.ConnectionState state = player.getConnectionState();
        assertNotNull("getConnectionState() must not return null for input: " + input, state);
        assertEquals(
            "getConnectionState() must return NOT_CONNECTED for invalid input: " + input,
            CastingPlayer.ConnectionState.NOT_CONNECTED,
            state);
      } catch (Exception e) {
        fail("getConnectionState() must not throw for input: " + input + ", but threw: " + e);
      }
    }
  }
}
