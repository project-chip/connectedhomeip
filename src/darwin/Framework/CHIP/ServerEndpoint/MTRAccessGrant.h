/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Foundation/Foundation.h>
#import <Matter/MTRBaseClusters.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * An access grant, which can be represented as an entry in the Matter Access
 * Control cluster.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRAccessGrant : NSObject <NSCopying>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Grant access at the provided level to a specific node on the fabric.  The
 * provided nodeID must be an operational node identifier.
 */
+ (nullable MTRAccessGrant *)accessGrantForNodeID:(NSNumber *)nodeID privilege:(MTRAccessControlEntryPrivilege)privilege;

/**
 * Grant access to any node on the fabric that has a matching CASE Authenticated
 * Tag in its operational certificate.  The provided caseAuthenticatedTag must
 * be a 32-bit unsigned integer with lower 16 bits not 0, per the Matter
 * specification.
 */
+ (nullable MTRAccessGrant *)accessGrantForCASEAuthenticatedTag:(NSNumber *)caseAuthenticatedTag privilege:(MTRAccessControlEntryPrivilege)privilege;

/**
 * Grant access to any node on the fabric that is communicating with us via
 * group messages sent to the given group.  The provided groupID must be a valid
 * group identifier in the range 1-65535.
 */
+ (nullable MTRAccessGrant *)accessGrantForGroupID:(NSNumber *)groupID privilege:(MTRAccessControlEntryPrivilege)privilege;

/**
 * Grant access to any node on the fabric, as long as it's communicating with us
 * over a unicast authenticated channel.
 */
+ (MTRAccessGrant *)accessGrantForAllNodesWithPrivilege:(MTRAccessControlEntryPrivilege)privilege;

/**
 * The matter access control subject ID that access has been granted for.  Nil
 * when access has been granted for all subjects (e.g. via initForAllNodesWithPrivilege).
 */
@property (nonatomic, copy, readonly, nullable) NSNumber * subjectID;

/**
 * The privilege that has been granted
 */
@property (nonatomic, assign, readonly) MTRAccessControlEntryPrivilege grantedPrivilege;

/**
 * The type of authentication mode the access grant is
 * for. MTRAccessControlEntryAuthModeCASE for unicast messages and
 * MTRAccessControlEntryAuthModeGroup for groupcast ones.
 */
@property (nonatomic, assign, readonly) MTRAccessControlEntryAuthMode authenticationMode;

@end

NS_ASSUME_NONNULL_END
