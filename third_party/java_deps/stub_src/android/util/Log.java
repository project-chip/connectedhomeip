/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

/** Log.java Stub file to allow compiling standalone, without Android SDK. */
package android.util;

public final class Log {

  public static final int VERBOSE = 2;

  public static final int DEBUG = 3;

  public static final int INFO = 4;

  public static final int WARN = 5;

  public static final int ERROR = 6;

  public static final int ASSERT = 7;

  public static final int NONE = Integer.MAX_VALUE;

  /**
   * Send a DEBUG log message.
   *
   * @param tag Used to identify the source of a log message. It usually identifies the class or
   *     activity where the log call occurs.
   * @param msg The message you would like logged.
   * @return A positive value if the message was loggable.
   */
  public static int d(String tag, String msg) {
    return log(DEBUG, tag, msg);
  }

  /**
   * Send a ERROR log message.
   *
   * @param tag Used to identify the source of a log message. It usually identifies the class or
   *     activity where the log call occurs.
   * @param msg The message you would like logged.
   * @return A positive value if the message was loggable.
   */
  public static int e(String tag, String msg) {
    return log(ERROR, tag, msg);
  }

  /**
   * Send a ERROR log message and log the exception.
   *
   * @param tag Used to identify the source of a log message. It usually identifies the class or
   *     activity where the log call occurs.
   * @param msg The message you would like logged.
   * @param tr An exception to log.
   * @return A positive value if the message was loggable.
   */
  public static int e(String tag, String msg, Throwable tr) {
    return log(ERROR, tag, msg, tr);
  }

  /**
   * Send a WARN log message.
   *
   * @param tag Used to identify the source of a log message. It usually identifies the class or
   *     activity where the log call occurs.
   * @param msg The message you would like logged.
   * @return A positive value if the message was loggable.
   */
  public static int w(String tag, String msg) {
    return log(WARN, tag, msg);
  }

  private static int log(int level, String tag, String msg) {
    if (level > ASSERT) {
      return -1;
    }

    System.out.println(tag + ':' + msg);

    return 1;
  }

  private static int log(int level, String tag, String msg, Throwable tr) {
    if (level > ASSERT) {
      return -1;
    }

    System.out.println(tag + ':' + msg);
    System.out.println(tr.toString());

    return 1;
  }
}
