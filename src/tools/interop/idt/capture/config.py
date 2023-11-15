#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""
The timeout time used by Orchestrator in capture/controller

Used when calling:
- Platform.connect() - if timeout, then halt
- Ecosystem.start(), .stop(), .probe() - if timeout, then continue execution and log error

This is an async timeout, so dependent on event loop being released to work.
To illustrate, consider this example where no timeout is thrown despite the awaited task running for twice the timeout:
----
sleep_time = 2

async def not_actually_async():
    time.sleep(sleep_time * 2)  # Blocking the EL!

async def try_timeout():
    async with asyncio.timeout(sleep_time):
        await not_actually_async()
    print("Timeout was NOT thrown!")

asyncio.run(try_timeout())
----
Result: Timeout was NOT thrown!

Update the example
----
async def not_actually_async():  # Now it is_actually_async because we
    await asyncio.sleep(sleep_time * 2)  # change to something that isn't blocking the EL
----
Result: The timeout error will be raised.

"""
orchestrator_async_step_timeout_seconds = 240
