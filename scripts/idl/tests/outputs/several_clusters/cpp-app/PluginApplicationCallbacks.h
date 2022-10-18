#pragma once

#include <app-common/zap-generated/callbacks/PluginCallbacks.h>

#define MATTER_PLUGINS_INIT \
    MatterFirstPluginClientInitCallback(); \
    MatterSecondPluginClientInitCallback(); \
    MatterThirdPluginClientInitCallback();

