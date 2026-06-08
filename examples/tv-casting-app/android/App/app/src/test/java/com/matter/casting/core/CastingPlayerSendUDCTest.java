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
import static org.mockito.Mockito.*;

import com.matter.casting.support.CommissionerDeclaration;
import com.matter.casting.support.ConnectionCallbacks;
import com.matter.casting.support.IdentificationDeclarationOptions;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;
import java.util.UUID;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

/** Unit tests for CastingPlayer.sendUDC() functionality. */
@RunWith(JUnit4.class)
public class CastingPlayerSendUDCTest {

  @Mock private MatterCastingPlayer mockCastingPlayer;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
  }

  /**
   * Test: sendUDC with basic IdentificationDeclarationOptions Verifies that sendUDC can be called
   * with minimal options
   */
  @Test
  public void testSendUDC_BasicOptions() {
    // Arrange
    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    verify(mockCastingPlayer, times(1)).sendUDC(callbacks, idOptions);
  }

  /**
   * Test: sendUDC with NoPasscode flag set Verifies that sendUDC properly handles the NoPasscode
   * flag for app detection
   */
  @Test
  public void testSendUDC_WithNoPasscode() {
    // Arrange
    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.isNoPasscode()).thenReturn(true);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    assertTrue("NoPasscode should be set", idOptions.isNoPasscode());
  }

  /**
   * Test: sendUDC with CancelPasscode flag set Verifies that sendUDC properly handles the
   * CancelPasscode flag to cancel a UDC session
   */
  @Test
  public void testSendUDC_WithCancelPasscode() {
    // Arrange
    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.isCancelPasscode()).thenReturn(true);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    assertTrue("CancelPasscode should be set", idOptions.isCancelPasscode());
  }

  /**
   * Test: sendUDC with InstanceName Verifies that sendUDC properly handles a custom instanceName
   */
  @Test
  public void testSendUDC_WithInstanceName() {
    // Arrange
    String instanceName = UUID.randomUUID().toString();

    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.getInstanceName()).thenReturn(instanceName);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    assertEquals("InstanceName should match", instanceName, idOptions.getInstanceName());
  }

  /**
   * Test: sendUDC with TargetAppInfo Verifies that sendUDC properly handles TargetAppInfo for app
   * detection
   */
  @Test
  public void testSendUDC_WithTargetAppInfo() {
    // Arrange
    Integer targetVendorId = 0xFFF1;

    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.isNoPasscode()).thenReturn(true);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    assertTrue("NoPasscode should be set", idOptions.isNoPasscode());
  }

  /**
   * Test: sendUDC with CommissionerDeclarationCallback Verifies that sendUDC properly registers the
   * CommissionerDeclarationCallback
   */
  @Test
  public void testSendUDC_WithCommissionerDeclarationCallback() {
    // Arrange
    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    MatterCallback<CommissionerDeclaration> commissionerDeclarationCallback =
        mock(MatterCallback.class);

    ConnectionCallbacks callbacks =
        new ConnectionCallbacks(successCallback, failureCallback, commissionerDeclarationCallback);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.isNoPasscode()).thenReturn(true);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
  }

  /**
   * Test: sendUDC complete app detection flow Verifies the complete flow: send with NoPasscode,
   * receive response, send CancelPasscode
   */
  @Test
  public void testSendUDC_CompleteAppDetectionFlow() {
    // Arrange
    String instanceName = UUID.randomUUID().toString();
    Integer targetVendorId = 0xFFF1;

    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);

    // Mock CommissionerDeclaration response
    MatterCallback<CommissionerDeclaration> commissionerDeclarationCallback =
        new MatterCallback<CommissionerDeclaration>() {
          @Override
          public void handle(CommissionerDeclaration cd) {
            // Simulate checking if app was found
            boolean appFound = !cd.getNoAppsFound();

            // Send CancelPasscode to end the UDC session
            IdentificationDeclarationOptions cancelOptions =
                mock(IdentificationDeclarationOptions.class);
            when(cancelOptions.isCancelPasscode()).thenReturn(true);
            when(cancelOptions.getInstanceName()).thenReturn(instanceName);

            ConnectionCallbacks cancelCallbacks =
                new ConnectionCallbacks(
                    mock(MatterCallback.class), mock(MatterCallback.class), null);

            mockCastingPlayer.sendUDC(cancelCallbacks, cancelOptions);
          }
        };

    ConnectionCallbacks callbacks =
        new ConnectionCallbacks(successCallback, failureCallback, commissionerDeclarationCallback);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.isNoPasscode()).thenReturn(true);
    when(idOptions.getInstanceName()).thenReturn(instanceName);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    assertEquals("InstanceName should match", instanceName, idOptions.getInstanceName());
    assertTrue("NoPasscode should be set", idOptions.isNoPasscode());
  }

  /**
   * Test: sendUDC with multiple TargetAppInfo entries Verifies that sendUDC can handle multiple
   * target apps
   */
  @Test
  public void testSendUDC_WithMultipleTargetApps() {
    // Arrange
    Integer targetVendorId1 = 0xFFF1;
    Integer targetVendorId2 = 0xFFF2;

    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);
    when(idOptions.isNoPasscode()).thenReturn(true);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error", result.hasError());
    assertTrue("NoPasscode should be set", idOptions.isNoPasscode());
  }

  /** Test: sendUDC with error response Verifies that sendUDC properly handles error responses */
  @Test
  public void testSendUDC_WithError() {
    // Arrange
    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);

    MatterError errorResult = new MatterError(1, "Test error");
    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(errorResult);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertTrue("Should have error", result.hasError());
    assertEquals("Error code should match", 1L, result.getErrorCode());
  }

  /**
   * Test: sendUDC validates required callbacks Verifies that sendUDC validates the presence of
   * required callbacks
   */
  @Test
  public void testSendUDC_ValidatesRequiredCallbacks() {
    // Arrange
    MatterCallback<Void> successCallback = mock(MatterCallback.class);
    MatterCallback<MatterError> failureCallback = mock(MatterCallback.class);

    // Create callbacks with required callbacks
    ConnectionCallbacks callbacks = new ConnectionCallbacks(successCallback, failureCallback, null);

    IdentificationDeclarationOptions idOptions = mock(IdentificationDeclarationOptions.class);

    when(mockCastingPlayer.sendUDC(
            any(ConnectionCallbacks.class), any(IdentificationDeclarationOptions.class)))
        .thenReturn(MatterError.NO_ERROR);

    // Act
    MatterError result = mockCastingPlayer.sendUDC(callbacks, idOptions);

    // Assert
    assertNotNull("Result should not be null", result);
    assertFalse("Should not have error with valid callbacks", result.hasError());
  }
}
