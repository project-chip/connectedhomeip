#pragma once
void MatterThirdPluginServerInitCallback();
void MatterThirdPluginServerShutdownCallback();

#define MATTER_PLUGINS_INIT \
    MatterThirdPluginServerInitCallback();

#define MATTER_PLUGINS_SHUTDOWN \
    MatterThirdPluginServerShutdownCallback();

