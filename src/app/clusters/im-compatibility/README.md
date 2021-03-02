# Interaction Model Handlers for compatibility

This file defines cluster handlers that need to be compatiable with ember ZCL
callbacks.

The templates for this directory is app/clusters/zap-templates/compatibility

-   `compatibility-clusters.zap` The list of clusters that need to be included.
-   `templates/compatibility/compatibility-command-handlers.zapt` The template
    of `compatibility-command-handlers.cpp`

You don't need to generate this handler too often, since this is just a call
converter of ember ZCL library and IM command handlers.
