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

#import "MTRMetrics_Internal.h"

NS_ASSUME_NONNULL_BEGIN

// dispatch_async of a client callback: runs it with the flow ID current at the call, or with a new one.
static inline void MTRDispatchCallbackWithFlowID(dispatch_queue_t queue, dispatch_block_t block)
{
    NSUUID * flowID = [MTRMetrics _currentOrNewFlowID];
    dispatch_async(queue, ^{
        [MTRMetrics _runWithFlowID:flowID block:block];
    });
}

NS_ASSUME_NONNULL_END

#ifdef __cplusplus
// Makes flowID current for the rest of the enclosing scope on this thread and restores the previous one on exit.
class MTRMetricsScope {
public:
    explicit MTRMetricsScope(NSUUID * _Nullable flowID);
    ~MTRMetricsScope();
    MTRMetricsScope(const MTRMetricsScope &) = delete;
    MTRMetricsScope & operator=(const MTRMetricsScope &) = delete;

private:
    NSUUID * _Nullable mPrevious;
    NSUUID * _Nullable mFlowID;
};
#endif
