# Matter tv app common-api

The tv-app common-api module defines the interface to interact with the Matter
agent service for the content apps. This module defines the AIDL interfaces,
clusters and command abstractions accessible. It also defines various constants
and intent field definitions that would be used by the content app while
interacting with the Matter SDK. The interface provided by this module helps
content app to register dynamic endpoints as well as report any attribute
changes to the SDK.

## Permissions needed

To utilize the Matter agent interface the partner apps need to

-   Query and bind themselves to a service that handles
    `com.matter.tv.app.api.action.MatterAppAgent` action
-   The host process should hold the
    `com.matter.tv.app.api.permission.SEND_DATA` permission
-   To bind with the Matter app agent service, the client should hold the
    `com.matter.tv.app.api.permission.BIND_SERVICE_PERMISSION` permission

## Matter app agent APIs

The Matter app agent service exposes the following interface

-   `setSupportedClusters`
    -   This API allows partners to report clusters dynamically to the Matter
        agent
    -   This API is not incremental and on each API call we should report the
        full set of clusters - any clusters that are omitted in the latest API
        call that were added previously will be removed
    -   The above behavior does not impact static clusters declared in app
        resources and they will not be removed
    -   Dynamic cluster can be used to override and hide a static cluster based
        on cluster name
-   `reportAttributeChange`
    -   This API allows reporting changes to attributes by the content app
    -   It takes in the cluster ID and attribute ID for which the attribute
        change is reported

## Cluster and Attribute IDs

The common API package defines commonly used cluster IDs as well as
corresponding Attribute ID's through `com.matter.tv.app.api.Clusters`. The
common-api provides a quick way to refer to different clusters defined by the
Matter spec relevant for media casting, as well as attributes and commands
associated with each one.

## Intents

The supported intents are defined under the
`com.matter.tv.app.api.MatterIntentConstants`. This helper class defines the
relevant intents used while interacting with the Matter app agent service. This
includes the android permission strings to as well as command and attribute
intent fields to be used.
