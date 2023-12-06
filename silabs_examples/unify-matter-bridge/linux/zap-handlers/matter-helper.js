// This file will gives only the feature supported attributes
const ClusterFeatureAttributesSupport = {
  "Color Control": {
    "0": true,        // Hue/Saturation feature support
    "1": true,
    "2": true,
    "3": true,        // XY feature support
    "4": true,
    "5": true,
    "7": true,        // Color temperature feature support
    "17": true,
    "18": true,
    "19": true,
    "21": true,
    "22": true,
    "23": true,
    "25": true,
    "26": true,
    "27": true,
    "32": true,
    "33": true,
    "34": true,
    "36": true,
    "37": true,
    "38": true,
    "40": true,
    "41": true,
    "42": true,
    "48": true,
    "49": true,
    "50": true,
    "51": true,
    "52": true,
    "54": true,
    "55": true,
    "56": true,
    "58": true,
    "59": true,
    "60": true,
    "16384": true,    // Enhanced Hue feature support
    "16386": true,    // Color loop feature support
    "16387": true,
    "16388": true,
    "16389": true,
    "16390": true,
    "16395": true,    // Color temperature feature support
    "16396": true,
    "16397": true,
    "16400": true,
  },
  "Level Control": {     
  "1": true,              // Lighting feature support
  "2": true,
  "3": true,
  "4": true,              //Frequency feature support
  "5": true,
  "6": true,
  "16": true,
  "18": true,
  "19": true,
  "20": true,
  "16384": true,
  },
  "Door Lock": {
    "3": true,      // Door Position Sensor feature support
  },
  "On/Off": {
  "16384": true,      // Lighting feature support
  "16385": true,
  "16386": true,
  "16387": true,
  },
  "Thermostat": {
    "17": true,       // Cool feature Support
    "18": true,       // Heat feature Support
    "21": true,       // Heat feature Support
    "22": true,       // Heat feature Support
    "23": true,       // Cool feature Support
    "24": true,       // Cool feature Support
  },
  "Illuminance Measurement": {
    "4": true,      // This is optional attribute
  },
};

// Function is to publish supported optional attributes
function matterSupportedClusterFeatureAttributes(cluster_name, attribute_id) {
  if (ClusterFeatureAttributesSupport.hasOwnProperty(cluster_name)) {    
    const cluster = ClusterFeatureAttributesSupport[cluster_name];
    const attributeid = String(attribute_id);

    if (cluster.hasOwnProperty(attributeid)) {
      return true;
    }
  }
  return false;
}

exports.matterSupportedClusterFeatureAttributes = matterSupportedClusterFeatureAttributes
