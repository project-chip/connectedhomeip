/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.matter.virtual.device.app;

public class DeviceEventType {
  public static final long EventId_WiFiConnectivityChange = 0x8000;
  public static final long EventId_ThreadConnectivityChange = 0x8001;
  public static final long EventId_InternetConnectivityChange = 0x8002;
  public static final long EventId_ServiceConnectivityChange = 0x8003;
  public static final long EventId_ServiceProvisioningChange = 0x8004;
  public static final long EventId_TimeSyncChange = 0x8005;
  public static final long EventId_ICDPollingIntervalChang = 0x8006;
  public static final long EventId_CHIPoBLEConnectionEstablished = 0x8007;
  public static final long EventId_CHIPoBLEConnectionClosed = 0x8008;
  public static final long EventId_ThreadStateChange = 0x8009;
  public static final long EventId_ThreadInterfaceStateChange = 0x800A;
  public static final long EventId_CHIPoBLEAdvertisingChange = 0x800B;
  public static final long EventId_InterfaceIpAddressChanged = 0x800C;
  public static final long EventId_CommissioningWindowStatusChanged = 0x800D;
  public static final long EventId_CommissioningComplete = 0x800E;
  public static final long EventId_FailSafeTimerExpired = 0x800F;
  public static final long EventId_FailSafeStateChanged = 0x8010;
  public static final long EventId_OperationalNetworkEnabled = 0x8011;
  public static final long EventId_DnssdInitialized = 0x8012;
  public static final long EventId_DnssdRestartNeeded = 0x8013;
  public static final long EventId_BindingsChangedViaCluster = 0x8014;
  public static final long EventId_OtaStateChanged = 0x8015;
  public static final long EventId_ServerReady = 0x8016;
  public static final long EventId_ChipMsgSentEvent = 0x8017;
  public static final long EventId_ChipMsgRxEventHandled = 0x8018;
  public static final long EventId_AppWakeUpEvent = 0x8018;

  // out of public event range (0x8000)
  public static final long EventId_FabricRemoved = 0x9FFF;
}
