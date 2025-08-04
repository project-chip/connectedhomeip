#
#    Copyright (c) 2024 Project CHIP Authors
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

# Matter git info
execute_process(
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE MATTER_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git diff --quiet
    RESULT_VARIABLE MATTER_LOCAL_STATUS
)

if(MATTER_LOCAL_STATUS)
  set(MATTER_LOCAL_STATUS "-dirty")
else()
  set(MATTER_LOCAL_STATUS "")
endif()

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE MATTER_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND python3 -c "from datetime import datetime; print(datetime.now().strftime('%Y-%m-%d %H:%M:%S'))"
    OUTPUT_VARIABLE BUILD_TIMESTAMP
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git show -s --format=%cd --date=format:%Y-%m-%d
    OUTPUT_VARIABLE MATTER_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git remote get-url origin
    OUTPUT_VARIABLE MATTER_REMOTE_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Zephyr git info
execute_process(
    COMMAND git -C ${ZEPHYR_BASE} rev-parse HEAD
    OUTPUT_VARIABLE ZEPHYR_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git -C ${ZEPHYR_BASE} rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE ZEPHYR_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git -C ${ZEPHYR_BASE} show -s --format=%cd --date=format:%Y-%m-%d HEAD
    OUTPUT_VARIABLE ZEPHYR_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND bash -c "git -C ${ZEPHYR_BASE} remote get-url \$(git -C ${ZEPHYR_BASE} remote | head -n 1)"
    OUTPUT_VARIABLE ZEPHYR_REMOTE_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git -C ${ZEPHYR_BASE} diff --quiet
    RESULT_VARIABLE ZEPHYR_LOCAL_STATUS
)

if(ZEPHYR_LOCAL_STATUS)
    set(ZEPHYR_LOCAL_STATUS "-dirty")
else()
    set(ZEPHYR_LOCAL_STATUS "")
endif()

# Telink HAL info
execute_process(
    COMMAND git -C ${ZEPHYR_BASE}/../modules/hal/telink rev-parse HEAD
    OUTPUT_VARIABLE TELINK_HAL_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git -C ${ZEPHYR_BASE}/../modules/hal/telink diff --quiet
    RESULT_VARIABLE HAL_LOCAL_STATUS
)

if(TELINK_HAL_LOCAL_STATUS)
    set(TELINK_HAL_LOCAL_STATUS "-dirty")
else()
    set(TELINK_HAL_LOCAL_STATUS "")
endif()

execute_process(
    COMMAND git -C ${ZEPHYR_BASE}/../modules/hal/telink show -s --format=%cd --date=format:%Y-%m-%d
    OUTPUT_VARIABLE TELINK_HAL_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(app PRIVATE
  MATTER_COMMIT_HASH="${MATTER_COMMIT_HASH}"
  MATTER_BRANCH="${MATTER_BRANCH_NAME}"
  BUILD_TIMESTAMP="${BUILD_TIMESTAMP}"
  MATTER_COMMIT_DATE="${MATTER_COMMIT_DATE}"
  MATTER_REMOTE_URL="${MATTER_REMOTE_URL}"
  MATTER_LOCAL_STATUS="${MATTER_LOCAL_STATUS}"

  ZEPHYR_COMMIT_HASH="${ZEPHYR_COMMIT_HASH}"
  ZEPHYR_BRANCH="${ZEPHYR_BRANCH_NAME}"
  ZEPHYR_COMMIT_DATE="${ZEPHYR_COMMIT_DATE}"
  ZEPHYR_REMOTE_URL="${ZEPHYR_REMOTE_URL}"
  ZEPHYR_LOCAL_STATUS="${ZEPHYR_LOCAL_STATUS}"

  TELINK_HAL_COMMIT_HASH="${TELINK_HAL_COMMIT_HASH}"
  TELINK_HAL_LOCAL_STATUS="${TELINK_HAL_LOCAL_STATUS}"
  TELINK_HAL_COMMIT_DATE="${TELINK_HAL_COMMIT_DATE}"
)

message(STATUS "Matter revision:")
message(STATUS "      board: ${CONFIG_BOARD}")
message(STATUS "      branch: ${MATTER_BRANCH_NAME} ${MATTER_COMMIT_HASH} ${MATTER_COMMIT_DATE}")
message(STATUS "      remote: ${MATTER_REMOTE_URL}")
message(STATUS "      build timestamp: ${BUILD_TIMESTAMP}")

message(STATUS "Zephyr revision:")
message(STATUS "      branch: ${ZEPHYR_BRANCH_NAME} ${ZEPHYR_COMMIT_HASH} ${ZEPHYR_COMMIT_DATE}")
message(STATUS "      remote: ${ZEPHYR_REMOTE_URL}")

message(STATUS "HAL revision:")
message(STATUS "      commit: ${TELINK_HAL_COMMIT_HASH} ${TELINK_HAL_COMMIT_DATE}")
