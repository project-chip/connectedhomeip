/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MTRPairingStatus) {
    MTRPairingStatusUnknown = 0,
    MTRPairingStatusSuccess = 1,
    MTRPairingStatusFailed = 2,
    MTRPairingStatusDiscoveringMoreDevices = 3
};

/**
 * The protocol definition for the MTRDevicePairingDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol MTRDevicePairingDelegate <NSObject>
@optional
/**
 * Notify the delegate when pairing status gets updated
 *
 */
- (void)onStatusUpdate:(MTRPairingStatus)status;

/**
 * Notify the delegate when pairing is completed
 *
 */
- (void)onPairingComplete:(nullable NSError *)error;

/**
 * Notify the delegate when commissioning is completed
 *
 */
- (void)onCommissioningComplete:(nullable NSError *)error;

/**
 * Notify the delegate when pairing is deleted
 *
 */
- (void)onPairingDeleted:(nullable NSError *)error;

@end

NS_ASSUME_NONNULL_END
