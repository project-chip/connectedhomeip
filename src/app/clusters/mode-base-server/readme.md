# Mode Base and its aliases

ModeBase is a pseudo cluster. It has no cluster ID. It exists only to be derived into other clusters.

# How to use a ModeBase derived cluster

To use a ModeBase derived cluster, you need to
* create a class that inherits the ModeBase::Instance class. 
* In the class implement the `NumberOfModes`, `GetModeLabelByIndex`, `GetModeValueByIndex`w `GetModeTagsByIndex`
and `HandleChangeToMode` functions. Look at the documentation in `mode-base-server.h` for more information about 
these functions. Cluster specific enums can be accessed from the cluster namespace.
* Optionally implement the `AppInit` function.

**Note** Zap accessor functions for these clusters will not return true values. Use the instance `Update...` 
and `Get...` functions to access the attributes.

# How to add new derived clusters

Once a ModeBase derived cluster has been defined in the spec, adding the implementation to ModeBase is fairly easy.
* translate the spec as an XML in `src/app/zap-templates/zcl/data-model/chip`. You can look at similar files on how to do this.
* Add the cluster ID to `ModeBaseAliasesInstanceMap`.
* Regenerate the zap code.
* Extend the all-clusters-app example to include your new cluster.