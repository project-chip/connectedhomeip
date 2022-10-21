#pragma once

#define MATTER_PLUGINS_INIT \
    MatterFirstPluginClientInitCallback(); \
    MatterSecondPluginClientInitCallback(); \
    MatterThirdPluginClientInitCallback(); \
    MatterThirdPluginServerInitCallback();

