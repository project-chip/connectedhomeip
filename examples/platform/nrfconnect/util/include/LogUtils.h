#pragma once

#ifndef CONFIG_CHIP_PW_RPC
#include <logging/log.h>
LOG_MODULE_DECLARE(app);
#define LOG_STRDUP log_strdup
#else
#include "pw_log/log.h"
#define LOG_INF(message, ...) PW_LOG_INFO(message, __VA_ARGS__)
#define LOG_ERR(message, ...) PW_LOG_ERROR(message, __VA_ARGS__)
#define LOG_STRDUP
#endif
