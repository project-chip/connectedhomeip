/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import "MTRTraceContext_Internal.h"

#include <lib/support/CodeUtils.h>

static NSString * const sTraceIDKey = @"t";

MTR_DIRECT_MEMBERS
@implementation MTRTraceContext

- (instancetype)initWithTraceID:(NSUUID *)traceID
{
    self = [super init];
    VerifyOrReturnValue(traceID != nil, nil);
    _traceID = [traceID copy];
    return self;
}

static thread_local __unsafe_unretained MTRTraceContext * _Nullable sCurrentContext = nil;

+ (nullable MTRTraceContext *)currentContext
{
    return sCurrentContext;
}

+ (instancetype)newTrace
{
    return [[self alloc] initWithTraceID:[NSUUID UUID]];
}

+ (MTRTraceContext *)currentOrNewContext
{
    return sCurrentContext ?: [self newTrace];
}

+ (MTRTraceContext *)contextForReceivedTraceID:(id _Nullable)traceID
{
    NSUUID * uuid = MTR_SAFE_CAST(traceID, NSUUID);
    return uuid ? [[self alloc] initWithTraceID:uuid] : [self newTrace];
}

+ (void)runWithContext:(MTRTraceContext * _Nullable)context block:(void(NS_NOESCAPE ^)(void))block
{
    MTRTraceScope scope(context);
    block();
}

+ (dispatch_block_t)wrapBlockInCurrentContext:(dispatch_block_t)block
{
    MTRTraceContext * context = sCurrentContext;
    if (context == nil) {
        return block;
    }
    return ^{
        MTRTraceScope scope(context);
        block();
    };
}

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    NSUUID * traceID = [coder decodeObjectOfClass:NSUUID.class forKey:sTraceIDKey];
    return [self initWithTraceID:traceID];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_traceID forKey:sTraceIDKey];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self; // immutable
}

- (NSUInteger)hash
{
    return _traceID.hash;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTRTraceContext * other = object;
    return [_traceID isEqual:other->_traceID];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %@>", self.class, _traceID.UUIDString];
}

@end

MTRTraceScope::MTRTraceScope(MTRTraceContext * _Nullable context)
    : mPrevious(sCurrentContext)
    , mContext(context)
{
    sCurrentContext = mContext;
}

MTRTraceScope::~MTRTraceScope() { sCurrentContext = mPrevious; }
