// This file is for manipulating a device type if it should conform to spec when
// mapped from Unify devices. There are three levels of conformance:
// 1. Cluster conformance: If a cluster is not supported by Matter, it will be
//    removed from the device.
// 2. Attribute conformance: If an attribute is not supported by Matter, it will
//    be removed from the device.
// 3. Command conformance: If a command is not supported by Matter, it will be
//    removed from the device.

exports.model = {
    "onofflight": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "dimmablelight": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "colortemperaturelight": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "extendedcolorlight": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "onoffpluginunit": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "dimmablepluginunit": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "onofflightswitch": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "dimmerswitch": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "contactsensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "lightsensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "occupancysensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "tempsensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "pressuresensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "flowsensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "onoffsensor": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "doorlock": {
        "clusterSpecConformance": true,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "windowcovering": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    },
    "thermostat": {
        "clusterSpecConformance": false,
        "attributeSpecConformance": false,
        "commandSpecConformance": false
    }
}