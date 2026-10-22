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

#import <Matter/MTRTraceContext.h>

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRTraceContext ()

- (instancetype)initWithTraceID:(NSUUID *)traceID;

// A new context with a fresh random ID.
+ (instancetype)newTrace;

// The current context, or a new one if there is none.
+ (MTRTraceContext *)currentOrNewContext;

// The context for a trace ID received from another process, or a new one if it is missing or not an NSUUID.
+ (MTRTraceContext *)contextForReceivedTraceID:(id _Nullable)traceID;

// Runs block with context current on this thread, then restores the previous context.
+ (void)runWithContext:(MTRTraceContext * _Nullable)context block:(void(NS_NOESCAPE ^)(void))block;

// Returns a block that runs block with the context current when this is called.
+ (dispatch_block_t)wrapBlockInCurrentContext:(dispatch_block_t)block;

@end

// A trace ID for log lines, or "none".
static inline NSString * MTRTraceIDLogString(MTRTraceContext * _Nullable context)
{
    return context.traceID.UUIDString ?: @"none";
}

static inline NSString * MTRCurrentTraceIDString(void)
{
    return MTRTraceIDLogString(MTRTraceContext.currentContext);
}

// dispatch_async that runs block in the trace current at the call, if any.
static inline void MTRTracedDispatchAsync(dispatch_queue_t queue, dispatch_block_t block)
{
    dispatch_async(queue, [MTRTraceContext wrapBlockInCurrentContext:block]);
}

// dispatch_async of a client callback: runs it in the trace current at the call, or in a new one.
static inline void MTRDispatchTracedCallback(dispatch_queue_t queue, dispatch_block_t block)
{
    MTRTraceContext * context = [MTRTraceContext currentOrNewContext];
    dispatch_async(queue, ^{
        [MTRTraceContext runWithContext:context block:block];
    });
}

NS_ASSUME_NONNULL_END

#ifdef __cplusplus
// Sets a context for the rest of the enclosing scope on this thread and restores the previous one on exit.
class MTRTraceScope {
public:
    explicit MTRTraceScope(MTRTraceContext * _Nullable context);
    ~MTRTraceScope();
    MTRTraceScope(const MTRTraceScope &) = delete;
    MTRTraceScope & operator=(const MTRTraceScope &) = delete;

private:
    MTRTraceContext * _Nullable mPrevious;
    MTRTraceContext * _Nullable mContext;
};
#endif
