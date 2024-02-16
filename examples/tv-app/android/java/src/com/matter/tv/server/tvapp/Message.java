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

import java.util.Vector;

public class Message {

    public class PriorityType {
        int priority;
        public PriorityType(int priority)
        {
            this.priority = priority;
        }
    }

    public String messageId;
    public PriorityType priority;
    public int messageControl;
    public long startTime;
    public int duration;
    public String messageText;
    public Vector<MessageResponseOption> responseOptions;

    public Message(String messageId,
        PriorityType priority,
        int messageControl,
        long startTime,
        int duration,
        String messageText,
        Vector<MessageResponseOption> responseOptions)
    {
        this.messageId = messageId;
        this.priority = priority;
        this.messageControl = messageControl;
        this.startTime = startTime;
        this.duration = duration;
        this.messageText = messageText;
        this.responseOptions = responseOptions;
    }
}
