/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef __RingPthread_h__
#define __RingPthread_h__

#include <pthread.h>

#include "Ring.h"

class RingPthread : Ring
{
    typedef Ring super_t;

    pthread_mutex_t _mutex;
    pthread_mutexattr_t _mutexAttr;
    pthread_cond_t _condv;

public:
    RingPthread(uint16_t itemSize, uint16_t itemCount) : Ring(itemSize, itemCount)
    {
        pthread_mutexattr_init(&_mutexAttr);
        pthread_mutexattr_settype(&_mutexAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_mutex, &_mutexAttr);
        pthread_cond_init(&_condv, NULL);
    }

    ~RingPthread()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_condv);
    }

    void put(void * itemIn)
    {
        pthread_mutex_lock(&_mutex);
        super_t::push_back(itemIn);
        pthread_cond_signal(&_condv);
        pthread_mutex_unlock(&_mutex);
    }

    bool get(void * itemOut, uint32_t tmo)
    {
        void * item = NULL;

        pthread_mutex_lock(&_mutex);
        if (tmo)
        {
            while (super_t::size() == 0)
            {
                pthread_cond_wait(&_condv, &_mutex);
            }
        }

        if (super_t::size() != 0)
        {
            item = super_t::front();

            if (item)
            {
                memcpy(itemOut, item, super_t::item_size());
                super_t::pop_front();
            }
        }

        pthread_mutex_unlock(&_mutex);

        return (item != NULL);
    }

    uint16_t size()
    {
        pthread_mutex_lock(&_mutex);
        uint16_t size = super_t::size();
        pthread_mutex_unlock(&_mutex);
        return size;
    }
};

#endif // __RingPthread_h__
