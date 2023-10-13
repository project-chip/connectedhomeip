
// Match supportedMappedCluster but with codes instead of names
function supportedClusterByMpc(code)
{
  switch (code) {
  case 6:
    return true // Cluster OnOff
    case 8: return true // Cluster Level
        default: return false
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
    if (n == "MoveMode") return true
    if (n == "StepMode") return true
    else return false
}
function unifyEnumName(n)
{
  console.log(n)
  if (n == "EffectIdentifierEnum") return "OffWithEffectEffectIdentifier"
  if (n == "MoveMode") return "MoveStepMode"
  if (n == "StepMode") return "MoveStepMode"
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

exports.isUnifyEnumNameDifferent = isUnifyEnumNameDifferent
exports.clusterNameWithoutSlash  = clusterNameWithoutSlash
exports.unifyEnumName            = unifyEnumName
exports.clusterInitName          = clusterInitName
exports.listComma                = listComma
exports.commandCbFnName          = commandCbFnName
exports.asSnakeCaseLower         = asSnakeCaseLower
exports.supportedClusterByMpc    = supportedClusterByMpc
