
function asUpperCamelCaseUnify(label, options) {
  const preserveAcronyms = options && options.hash.preserveAcronyms;

  let tokens = label.replace(/[+()&]/g, '').split(/ |_|-|\//);

  let str = tokens
    .map((token) => {
      let isAcronym = (token == token.toUpperCase());
      if (!isAcronym) {
        let newToken = token[0].toUpperCase();
        if (token.length > 1) {
            newToken += token.substring(1);
        }
        return newToken;
      }

      if (preserveAcronyms) {
        return token;
      }

      // if preserveAcronyms is false, then anything beyond the first letter becomes lower-case.
      let newToken = token[0];
      if (token.length > 1) {
          newToken += token.substring(1).toLowerCase();
      }
      return newToken;
    })
    .join('');

  return str.replace(/[^A-Za-z0-9_]/g, '');
}

function supportedCluster(clusterName) {
  switch (clusterName) {
    case "Network Commissioning": return false
    case "Groups": return false   
    case "Test Cluster": return false
    case "Descriptor": return false
    case "Binding": return false
    case "Access Control": return false
    case "AdministratorCommissioning": false
    case "Bridged Device Basic": false
    case "Actions": false
    case "Ethernet Network Diagnostics": false
    default: return true
  }
}

function matterClusterConversion(clusterName) {
  switch (asUpperCamelCaseUnify(clusterName)) {
    case "LevelControl": return "Level"
    default: return asUpperCamelCaseUnify(clusterName)
  }
}

exports.matterClusterConversion = matterClusterConversion
exports.supportedCluster = supportedCluster
exports.asUpperCamelCaseUnify = asUpperCamelCaseUnify;