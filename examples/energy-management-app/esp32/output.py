cluster_t *IdentifyCluster = esp_matter::cluster::create(endpoint, 3, CLUSTER_FLAG_SERVER);
        set_plugin_server_init_callback(IdentifyCluster, MatterIdentifyPluginServerInitCallback);
        add_function_list(IdentifyCluster, identify_function_list, CLUSTER_FLAG_INIT_FUNCTION | CLUSTER_FLAG_ATTRIBUTE_CHANGED_FUNCTION);

attribute_t *identifyTimeAttribute = esp_matter::attribute::create(IdentifyCluster, 0, ATTRIBUTE_FLAG_WRITABLE,0);
    esp_matter::attribute::add_bounds(attribute, esp_matter_uint16(min), esp_matter_uint16(m     ax));
# min and max are mentioned in spec and also in esp_matter_cluster.cpp

attribute_t *identifyTypeAttribute = esp_matter::attribute::create(IdentifyCluster, 1, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *generatedCommandListAttribute = esp_matter::attribute::create(IdentifyCluster, 65528, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *acceptedCommandListAttribute = esp_matter::attribute::create(IdentifyCluster, 65529, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *eventListAttribute = esp_matter::attribute::create(IdentifyCluster, 65530, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *attributeListAttribute = esp_matter::attribute::create(IdentifyCluster, 65531, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *featureMapAttribute = esp_matter::attribute::create(IdentifyCluster, 65532, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *clusterRevisionAttribute = esp_matter::attribute::create(IdentifyCluster, 65533, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
command_t *IdentifyCommand = esp_matter::command::create(IdentifyCluster, 0, COMMAND_FLAGS, commandCallback);
command_t *TriggerEffectCommand = esp_matter::command::create(IdentifyCluster, 64, COMMAND_FLAGS, commandCallback);

cluster_t *GroupsCluster = esp_matter::cluster::create(endpoint, 4, CLUSTER_FLAG_SERVER);
        set_plugin_server_init_callback(cluster, MatterGroupsPluginServerInitCallback);
        add_function_list(cluster, function_list, function_flags);

attribute_t *nameSupportAttribute = esp_matter::attribute::create(GroupsCluster, 0, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *generatedCommandListAttribute = esp_matter::attribute::create(GroupsCluster, 65528, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *acceptedCommandListAttribute = esp_matter::attribute::create(GroupsCluster, 65529, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *eventListAttribute = esp_matter::attribute::create(GroupsCluster, 65530, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *attributeListAttribute = esp_matter::attribute::create(GroupsCluster, 65531, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *featureMapAttribute = esp_matter::attribute::create(GroupsCluster, 65532, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *clusterRevisionAttribute = esp_matter::attribute::create(GroupsCluster, 65533, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
command_t *AddGroupCommand = esp_matter::command::create(GroupsCluster, 0, COMMAND_FLAGS, commandCallback);
command_t *ViewGroupCommand = esp_matter::command::create(GroupsCluster, 1, COMMAND_FLAGS, commandCallback);
command_t *GetGroupMembershipCommand = esp_matter::command::create(GroupsCluster, 2, COMMAND_FLAGS, commandCallback);
command_t *RemoveGroupCommand = esp_matter::command::create(GroupsCluster, 3, COMMAND_FLAGS, commandCallback);
command_t *RemoveAllGroupsCommand = esp_matter::command::create(GroupsCluster, 4, COMMAND_FLAGS, commandCallback);
command_t *AddGroupIfIdentifyingCommand = esp_matter::command::create(GroupsCluster, 5, COMMAND_FLAGS, commandCallback);

cluster_t *ScenesCluster = esp_matter::cluster::create(endpoint, 5, CLUSTER_FLAG_SERVER);
        set_plugin_server_init_callback(cluster, MatterScenesPluginServerInitCallback);
        add_function_list(cluster, NULL, CLUSTER_FLAG_NONE);

attribute_t *sceneCountAttribute = esp_matter::attribute::create(ScenesCluster, 0, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *currentSceneAttribute = esp_matter::attribute::create(ScenesCluster, 1, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *currentGroupAttribute = esp_matter::attribute::create(ScenesCluster, 2, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *sceneValidAttribute = esp_matter::attribute::create(ScenesCluster, 3, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *nameSupportAttribute = esp_matter::attribute::create(ScenesCluster, 4, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *lastConfiguredByAttribute = esp_matter::attribute::create(ScenesCluster, 5, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *sceneTableSizeAttribute = esp_matter::attribute::create(ScenesCluster, 6, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *remainingCapacityAttribute = esp_matter::attribute::create(ScenesCluster, 7, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *generatedCommandListAttribute = esp_matter::attribute::create(ScenesCluster, 65528, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *acceptedCommandListAttribute = esp_matter::attribute::create(ScenesCluster, 65529, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *eventListAttribute = esp_matter::attribute::create(ScenesCluster, 65530, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *attributeListAttribute = esp_matter::attribute::create(ScenesCluster, 65531, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *featureMapAttribute = esp_matter::attribute::create(ScenesCluster, 65532, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *clusterRevisionAttribute = esp_matter::attribute::create(ScenesCluster, 65533, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
command_t *AddSceneCommand = esp_matter::command::create(ScenesCluster, 0, COMMAND_FLAGS, commandCallback);
command_t *ViewSceneCommand = esp_matter::command::create(ScenesCluster, 1, COMMAND_FLAGS, commandCallback);
command_t *RemoveSceneCommand = esp_matter::command::create(ScenesCluster, 2, COMMAND_FLAGS, commandCallback);
command_t *RemoveAllScenesCommand = esp_matter::command::create(ScenesCluster, 3, COMMAND_FLAGS, commandCallback);
command_t *StoreSceneCommand = esp_matter::command::create(ScenesCluster, 4, COMMAND_FLAGS, commandCallback);
command_t *RecallSceneCommand = esp_matter::command::create(ScenesCluster, 5, COMMAND_FLAGS, commandCallback);
command_t *GetSceneMembershipCommand = esp_matter::command::create(ScenesCluster, 6, COMMAND_FLAGS, commandCallback);
command_t *EnhancedAddSceneCommand = esp_matter::command::create(ScenesCluster, 64, COMMAND_FLAGS, commandCallback);
command_t *EnhancedViewSceneCommand = esp_matter::command::create(ScenesCluster, 65, COMMAND_FLAGS, commandCallback);
command_t *CopySceneCommand = esp_matter::command::create(ScenesCluster, 66, COMMAND_FLAGS, commandCallback);

cluster_t *OnOffCluster = esp_matter::cluster::create(endpoint, 6, CLUSTER_FLAG_SERVER);
attribute_t *onOffAttribute = esp_matter::attribute::create(OnOffCluster, 0, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
        set_plugin_server_init_callback(cluster, MatterOnOffPluginServerInitCallback);
        add_function_list(cluster, on_off_function_list, CLUSTER_FLAG_INIT_FUNCTION | CLUSTER_FLAG_SHUTDOWN_FUNCTION);


attribute_t *globalSceneControlAttribute = esp_matter::attribute::create(OnOffCluster, 16384, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *onTimeAttribute = esp_matter::attribute::create(OnOffCluster, 16385, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *offWaitTimeAttribute = esp_matter::attribute::create(OnOffCluster, 16386, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *startUpOnOffAttribute = esp_matter::attribute::create(OnOffCluster, 16387, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *generatedCommandListAttribute = esp_matter::attribute::create(OnOffCluster, 65528, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *acceptedCommandListAttribute = esp_matter::attribute::create(OnOffCluster, 65529, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *eventListAttribute = esp_matter::attribute::create(OnOffCluster, 65530, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *attributeListAttribute = esp_matter::attribute::create(OnOffCluster, 65531, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *featureMapAttribute = esp_matter::attribute::create(OnOffCluster, 65532, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *clusterRevisionAttribute = esp_matter::attribute::create(OnOffCluster, 65533, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
command_t *OffCommand = esp_matter::command::create(OnOffCluster, 0, COMMAND_FLAGS, commandCallback);
command_t *OnCommand = esp_matter::command::create(OnOffCluster, 1, COMMAND_FLAGS, commandCallback);
command_t *ToggleCommand = esp_matter::command::create(OnOffCluster, 2, COMMAND_FLAGS, commandCallback);
command_t *OffWithEffectCommand = esp_matter::command::create(OnOffCluster, 64, COMMAND_FLAGS, commandCallback);
command_t *OnWithRecallGlobalSceneCommand = esp_matter::command::create(OnOffCluster, 65, COMMAND_FLAGS, commandCallback);
command_t *OnWithTimedOffCommand = esp_matter::command::create(OnOffCluster, 66, COMMAND_FLAGS, commandCallback);


cluster_t *DescriptorCluster = esp_matter::cluster::create(endpoint, 29, CLUSTER_FLAG_SERVER);
        set_plugin_server_init_callback(cluster, MatterDescriptorPluginServerInitCallback);
        add_function_list(cluster, NULL, CLUSTER_FLAG_NONE);

attribute_t *deviceTypeListAttribute = esp_matter::attribute::create(DescriptorCluster, 0, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *serverListAttribute = esp_matter::attribute::create(DescriptorCluster, 1, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *clientListAttribute = esp_matter::attribute::create(DescriptorCluster, 2, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *partsListAttribute = esp_matter::attribute::create(DescriptorCluster, 3, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *generatedCommandListAttribute = esp_matter::attribute::create(DescriptorCluster, 65528, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *acceptedCommandListAttribute = esp_matter::attribute::create(DescriptorCluster, 65529, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *eventListAttribute = esp_matter::attribute::create(DescriptorCluster, 65530, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *attributeListAttribute = esp_matter::attribute::create(DescriptorCluster, 65531, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *featureMapAttribute = esp_matter::attribute::create(DescriptorCluster, 65532, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
attribute_t *clusterRevisionAttribute = esp_matter::attribute::create(DescriptorCluster, 65533, ATTRIBUTE_FLAGS, DEFAULT_VALUE);
endpoint_t *endpoint = esp_matter::endpoint::create(node, 1, ENDPOINT_FLAGS, priv_data);

