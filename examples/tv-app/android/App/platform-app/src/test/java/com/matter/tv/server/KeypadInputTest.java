package com.matter.tv.server;

import static org.junit.Assert.*;

import com.matter.tv.app.api.Clusters;
import org.junit.Test;

/**
 * Unit tests for KeypadInput cluster constants and structures.
 * Tests verify that cluster IDs, command IDs, and field IDs match the Matter specification.
 */
public class KeypadInputTest {

  @Test
  public void testKeypadInputClusterId() {
    // KeypadInput cluster ID should be 0x0509 (1289 in decimal)
    assertEquals("KeypadInput cluster ID should be 0x0509", 0x0509, Clusters.KeypadInput.Id);
  }

  @Test
  public void testSendKeyCommandId() {
    // SendKey command ID should be 0x00
    assertEquals("SendKey command ID should be 0x00", 0x00, Clusters.KeypadInput.Commands.SendKey.ID);
  }

  @Test
  public void testSendKeyResponseCommandId() {
    // SendKeyResponse command ID should be 0x01
    assertEquals("SendKeyResponse command ID should be 0x01", 0x01, Clusters.KeypadInput.Commands.SendKeyResponse.ID);
  }

  @Test
  public void testSendKeyFields() {
    // Verify SendKey command field IDs
    assertEquals("KeyCode field ID should be 0", 0, Clusters.KeypadInput.Commands.SendKey.Fields.KeyCode);
  }

  @Test
  public void testSendKeyResponseFields() {
    // Verify SendKeyResponse command field IDs
    assertEquals("Status field ID should be 0", 0, Clusters.KeypadInput.Commands.SendKeyResponse.Fields.Status);
  }

  @Test
  public void testStatusEnumValues() {
    // Verify Status enum values match Matter spec
    assertEquals("SUCCESS status should be 0", 0, Clusters.KeypadInput.Types.StatusEnum.Success);
    assertEquals("UNSUPPORTED_KEY status should be 1", 1, Clusters.KeypadInput.Types.StatusEnum.UnsupportedKey);
    assertEquals("INVALID_KEY_IN_CURRENT_STATE status should be 2", 2, Clusters.KeypadInput.Types.StatusEnum.InvalidKeyInCurrentState);
  }

  @Test
  public void testStatusEnumCount() {
    // Ensure all expected status values are defined
    // This helps catch if new status values are added to the spec
    int[] expectedStatuses = {
      Clusters.KeypadInput.Types.StatusEnum.Success,
      Clusters.KeypadInput.Types.StatusEnum.UnsupportedKey,
      Clusters.KeypadInput.Types.StatusEnum.InvalidKeyInCurrentState
    };
    
    // Verify no duplicates
    for (int i = 0; i < expectedStatuses.length; i++) {
      for (int j = i + 1; j < expectedStatuses.length; j++) {
        assertNotEquals("Status values should be unique", expectedStatuses[i], expectedStatuses[j]);
      }
    }
  }

  @Test
  public void testClusterIdIsPositive() {
    // Cluster IDs should always be positive
    assertTrue("Cluster ID should be positive", Clusters.KeypadInput.Id > 0);
  }

  @Test
  public void testCommandIdsAreNonNegative() {
    // Command IDs should be non-negative
    assertTrue("SendKey command ID should be non-negative", Clusters.KeypadInput.Commands.SendKey.ID >= 0);
    assertTrue("SendKeyResponse command ID should be non-negative", Clusters.KeypadInput.Commands.SendKeyResponse.ID >= 0);
  }

  @Test
  public void testFieldIdsAreNonNegative() {
    // Field IDs should be non-negative
    assertTrue("KeyCode field ID should be non-negative", Clusters.KeypadInput.Commands.SendKey.Fields.KeyCode >= 0);
    assertTrue("Status field ID should be non-negative", Clusters.KeypadInput.Commands.SendKeyResponse.Fields.Status >= 0);
  }

  @Test
  public void testCommandIdOrderIsCorrect() {
    // Verify SendKey comes before SendKeyResponse
    assertTrue("SendKey should have lower ID than SendKeyResponse", 
        Clusters.KeypadInput.Commands.SendKey.ID < Clusters.KeypadInput.Commands.SendKeyResponse.ID);
  }

  @Test
  public void testStatusEnumOrderIsCorrect() {
    // Verify status values are in expected order
    assertTrue("Success should be 0", Clusters.KeypadInput.Types.StatusEnum.Success == 0);
    assertTrue("UnsupportedKey should be 1", Clusters.KeypadInput.Types.StatusEnum.UnsupportedKey == 1);
    assertTrue("InvalidKeyInCurrentState should be 2", Clusters.KeypadInput.Types.StatusEnum.InvalidKeyInCurrentState == 2);
  }
}
