#pragma once
void MatterThirdPluginServerInitCallback();

#define MATTER_PLUGINS_INIT \
    MatterThirdPluginServerInitCallback();

