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

import android.util.Log;
import java.util.HashMap;
import java.util.Map;

public class MessagesManagerStub implements MessagesManager {
  private static final String TAG = MessagesManagerStub.class.getSimpleName();

  private int endpoint = -1;

  private Map<String, Message> messages = new HashMap<String, Message>();

  public MessagesManagerStub(int endpoint) {
    this.endpoint = endpoint;
    Log.d(TAG, "MessagesManagerStub: at " + this.endpoint);
  }

  @Override
  public Message[] getMessages() {
    Log.d(TAG, "getMessages: at " + this.endpoint);
    return messages.values().toArray(new Message[0]);
  }

  @Override
  public boolean presentMessages(
      String messageId,
      int priority,
      int messageControl,
      long startTime,
      long duration,
      String messageText,
      HashMap<Long, String> responseOptions) {
    Log.d(
        TAG, "presentMessages: at " + this.endpoint + " id:" + messageId + " text:" + messageText);
    MessageResponseOption[] options = new MessageResponseOption[responseOptions.size()];
    int i = 0;

    for (Map.Entry<Long, String> set : responseOptions.entrySet()) {
      Log.d(
          TAG,
          "presentMessages option: key:" + set.getKey().longValue() + " value:" + set.getValue());
      options[i] = new MessageResponseOption(set.getKey().longValue(), set.getValue());
      i++;
    }

    Message message =
        new Message(messageId, priority, messageControl, startTime, duration, messageText, options);
    messages.put(messageId, message);

    UserPrompter prompter = UserPrompterResolver.getUserPrompter();
    if (prompter != null) {
      prompter.promptWithMessage(message);
    } else {
      Log.d(TAG, "presentMessages no global user prompter");
    }

    return true;
  }

  @Override
  public boolean cancelMessage(String messageId) {
    Log.d(TAG, "cancelMessage: at " + this.endpoint + " messageId:" + messageId);
    messages.remove(messageId);
    return true; // per spec, succeed unless error
  }
}
