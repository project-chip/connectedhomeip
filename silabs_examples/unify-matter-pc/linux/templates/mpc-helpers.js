
// Match supportedMappedCluster but with codes instead of names
function supportedClusterByMpc(code)
{
  switch (code) {
  case 6:
    return true // Cluster OnOff
    // case 8: return true // Cluster Level
  case 29:
    return true
        default: return false
  }
}

const supportedAttributesByMpc = {
  "On/Off": [
    "0",
    "16384",
    "16385",
    "16386",
    "16387",
    "65531",
    "65532",
    "65533",
    "65529",
    "65528"
  ],
  "Descriptor": [
    "0",
    "1",
    "2",
    "3"
  ],
}

const AttributeProcessRequired = {
  "On/Off": {
    "65531" : true,
    "65529" : true,
  },
  "Descriptor": {
    "1" : true,
    "3" : true,
  }
}

function isCommandHandlingRequired(code)
{
  switch (code) {
    case 29: return false
    default: return true
  }
}

function clusterInitName(zclCluster)
{
  return asSnakeCaseLower(zclCluster.label) + "_cluster_mapper_init"
}

function isUnifyEnumNameDifferent(n)
{
  if (n == "EffectIdentifierEnum")
    return true
    if (n == "MoveModeEnum") return true
    if (n == "StepModeEnum") return true
    else return false
}
function unifyEnumName(n)
{
  console.log(n)
  if (n == "EffectIdentifierEnum") return "OffWithEffectEffectIdentifier"
  if (n == "MoveModeEnum") return "MoveStepMode"
  if (n == "StepModeEnum") return "MoveStepMode"
  else return n
}

function clusterNameWithoutSlash(label)
{
  var l = label.trim()
  l     = l.replace(/ /g, '')
  l     = l.replace(/[/]/g, '')
  return l
}

function asCleanSymbol(label)
{
  var l = label.trim()
  l     = l.replace(/ /g, '_')
  l     = l.replace(/-/g, '_')
  l     = l.replace(/[/]/g, '_')
  l     = l.replace(/:/g, '_')
  l     = l.replace(/__+/g, '_').toLowerCase()
  return l
}

function listComma(obj)
{
  if (obj.index == obj.count - 1)
    return ""
    return ","
}

function asSnakeCaseLower(str)
{
  if (!str) {
    return str
  }
  var label = str.replace(/\.?([A-Z][a-z])/g, function(x, y) {
    return '_' + y
  })
  label     = asCleanSymbol(label)
  if (label.startsWith('_'))
  {
    label = label.substring(1)
  }
  if (label == "level_control")
    label = "level"
    return label
}

function commandCbFnName(zclCommand)
{
  if (asSnakeCaseLower(zclCommand.parent.label) == "level_control")
    return "level" +
        "_cluster_" + asSnakeCaseLower(zclCommand.label) + "_command"
    else return asSnakeCaseLower(zclCommand.parent.label) + "_cluster_" + asSnakeCaseLower(zclCommand.label) + "_command"
}

function supportedAttributes(cluster_name, attribute_id) {
  if (supportedAttributesByMpc.hasOwnProperty(cluster_name)) {
    const cluster = supportedAttributesByMpc[cluster_name];
    const attributeid = String(attribute_id);

    if (cluster.includes(attributeid)) {
      return true;
    }
  }
  return false;
}

function supportedMappedClusterCode(code){
  switch (code) {
    default: return true
  }
}

function matterSupportedClusterFeatureCommands(cluster_name, command_id) {
  return true;
}

function matterSupportedClusterFeatureAttributes(cluster_name, command_id){
  return true;
}

function supportedClusterCommandResponse(code)
{
  switch (code) {
    default: return false
  }
}

function supportedClusterEvents(code)
{
  switch (code) {
    default: return false
  }
}

function isDefaultNull(zclAttribute){
  if(zclAttribute.isNullable == true){
    return true;
  }
  return false;
}

function isEntryTypeNotNull(value){
  if (value == null){
    return false;
  }
  return true;
}

function isProcessingRequired(cluster_name, attribute_id){
  if (AttributeProcessRequired.hasOwnProperty(cluster_name)){
    const cluster = AttributeProcessRequired[cluster_name];
    const attributeid = String(attribute_id);
    if(cluster.hasOwnProperty(attributeid)){
      return true;
    }
  }
  return false;
}

exports.isUnifyEnumNameDifferent = isUnifyEnumNameDifferent
exports.clusterNameWithoutSlash  = clusterNameWithoutSlash
exports.unifyEnumName            = unifyEnumName
exports.clusterInitName          = clusterInitName
exports.listComma                = listComma
exports.commandCbFnName          = commandCbFnName
exports.asSnakeCaseLower         = asSnakeCaseLower
exports.supportedClusterByMpc    = supportedClusterByMpc
exports.supportedMappedClusterCode    = supportedMappedClusterCode
exports.supportedAttributes      = supportedAttributes
exports.isDefaultNull            = isDefaultNull
exports.isEntryTypeNotNull       = isEntryTypeNotNull
exports.isProcessingRequired     = isProcessingRequired
exports.isCommandHandlingRequired= isCommandHandlingRequired
exports.matterSupportedClusterFeatureCommands = matterSupportedClusterFeatureCommands
exports.matterSupportedClusterFeatureAttributes = matterSupportedClusterFeatureAttributes
exports.supportedClusterCommandResponse = supportedClusterCommandResponse
exports.supportedClusterEvents = supportedClusterEvents