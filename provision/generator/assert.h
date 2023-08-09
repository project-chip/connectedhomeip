
#pragma once

#define ASSERT(cond, action, message, ...)                              \
    do                                                                  \
    {                                                                   \
        if (!(cond))                                                    \
        {                                                               \
            action;                                                     \
        }                                                               \
    } while (0)
