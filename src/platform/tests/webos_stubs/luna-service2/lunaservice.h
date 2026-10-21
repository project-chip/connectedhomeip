/*
 *
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

#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long LSMessageToken;

#define LSMESSAGE_TOKEN_INVALID 0

typedef struct LSError
{
    int error_code;
    char * message;
    const char * file;
    int line;
    const char * func;
    void * padding;
    unsigned long magic;
} LSError;

typedef struct LSHandle
{
    const char * name;
} LSHandle;

typedef struct LSMessage
{
    const char * payload;
} LSMessage;

typedef bool (*LSFilterFunc)(LSHandle * sh, LSMessage * reply, void * ctx);

static inline void LSErrorInit(LSError * lserror)
{
    if (lserror != NULL)
    {
        memset(lserror, 0, sizeof(*lserror));
    }
}

static inline void LSErrorFree(LSError * lserror)
{
    if (lserror != NULL)
    {
        free(lserror->message);
        lserror->message = NULL;
    }
}

static inline const char * LSMessageGetPayload(LSMessage * message)
{
    return (message != NULL && message->payload != NULL) ? message->payload : "";
}

static inline bool LSCall(LSHandle * sh, const char * uri, const char * payload, LSFilterFunc callback, void * ctx,
                          LSMessageToken * ret_token, LSError * lserror)
{
    (void) sh;
    (void) uri;
    (void) payload;
    (void) callback;
    (void) ctx;
    (void) lserror;

    static LSMessageToken nextToken = 1;
    if (ret_token != NULL)
    {
        *ret_token = nextToken++;
        if (*ret_token == LSMESSAGE_TOKEN_INVALID)
        {
            *ret_token = nextToken++;
        }
    }

    return true;
}

static inline bool LSCallCancel(LSHandle * sh, LSMessageToken token, LSError * lserror)
{
    (void) sh;
    (void) token;
    (void) lserror;

    return true;
}

#ifdef __cplusplus
}
#endif
