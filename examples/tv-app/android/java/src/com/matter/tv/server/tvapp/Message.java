/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.matter.tv.server.tvapp;

public class Message {

  public String messageId;
  public int priority;
  public int messageControl;
  public long startTime;
  public long duration;
  public String messageText;
  public MessageResponseOption responseOptions[];

  public Message(
      String messageId,
      int priority,
      int messageControl,
      long startTime,
      long duration,
      String messageText,
      MessageResponseOption responseOptions[]) {
    this.messageId = messageId;
    this.priority = priority;
    this.messageControl = messageControl;
    this.startTime = startTime;
    this.duration = duration;
    this.messageText = messageText;
    this.responseOptions = responseOptions;
  }
}
