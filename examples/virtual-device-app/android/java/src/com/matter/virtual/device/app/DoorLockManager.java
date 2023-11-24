package com.matter.virtual.device.app;

public interface DoorLockManager {

  int DlLockState_kNotFullyLocked = 0;
  int DlLockState_kLocked = 1;
  int DlLockState_kUnlocked = 2;
  int DlLockState_kUnlatched = 3;
  int DlLockState_kUnknownEnumValue = 4;

  int DlLockType_kDeadBolt = 0;
  int DlLockType_kMagnetic = 1;
  int DlLockType_kOther = 2;
  int DlLockType_kMortise = 3;
  int DlLockType_kRim = 4;
  int DlLockType_kLatchBolt = 5;
  int DlLockType_kCylindricalLock = 6;
  int DlLockType_kTubularLock = 7;
  int DlLockType_kInterconnectedLock = 8;
  int DlLockType_kDeadLatch = 9;
  int DlLockType_kDoorFurniture = 10;
  int DlLockType_kEurocylinder = 11;

  int OperatingModeEnum_kNormal = 0;
  int OperatingModeEnum_kVacation = 1;
  int OperatingModeEnum_kPrivacy = 2;
  int OperatingModeEnum_kNoRemoteLockUnlock = 3;
  int OperatingModeEnum_kPassage = 4;

  int DlSupportedOperatingModes_kNormal = 0x1;
  int DlSupportedOperatingModes_kVacation = 0x2;
  int DlSupportedOperatingModes_kPrivacy = 0x4;
  int DlSupportedOperatingModes_kNoRemoteLockUnlock = 0x8;
  int DlSupportedOperatingModes_kPassage = 0x10;

  /** initialize attribute by DeviceApp */
  void initAttributeValue();

  void handleLockStateChanged(int value);
}
