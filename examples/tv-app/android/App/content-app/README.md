# Content App

This module provides an example of a TV Content App that supports Matter
casting. The content app can register its supported clusters with the Matter SDK
as well as receive any incoming commands and provide responses.

### Specifying the permissions

The content app needs to bind with the Matter agent service to start receiving
commands as well as register its supported clusters. To bind with the service,
the content app need to add the following permissions to their manifest file

```xml
<uses-permission android:name="com.matter.tv.app.api.permission.BIND_SERVICE_PERMISSION"/>
<uses-permission android:name="android.permission.QUERY_ALL_PACKAGES" tools:ignore="QueryAllPackagesPermission"/>
```

The permission string is defined within the `common-api` module as
`PERMISSION_MATTER_AGENT_BIND`

`AndroidManifest.xml` can be used as a good reference point

### Registering a command receiver

The content app should register itself as a receiver in order to get any
incoming matter commands that needs to be handled. To receive commands from
matter stack, the app will need to register a receiver instance that listens for
`MATTER_COMMAND` intents and have permission to send data via Matter API. Here
is a sample snippet

```xml
<!-- Intent action for receiving an Matter directive-->
<receiver
        android:name=".receiver.MatterCommandReceiver"
        android:permission="com.matter.tv.app.api.permission.SEND_DATA"
        android:enabled="true"
        android:exported="true">
    <intent-filter>
        <action android:name="com.matter.tv.app.api.action.MATTER_COMMAND" />
    </intent-filter>
</receiver>
```

In order to send data and respond to commands we need to include the permission
`com.matter.tv.app.api.permission.SEND_DATA` for the receiver as above.

### Structuring the application

The first step is to register for appropriate permissions as well as an intent
receiver as described in the steps above. Once the application is started, it
can bind to the Matter agent service. Refer to `MatterAgentClient.java` as an
example.

Once the content app binds to the Matter agent, it can register all its dynamic
endpoints and supported clusters through the `reportClusters` API call.

```cpp
executorService.execute(() -> matterAgentClient.reportClusters(supportedClustersRequest));
```

Whenever the content app wants to report an attribute change, it can leverage
the `reportAttributeChange` on the matter agent to notify the SDK.

Upon receiving a command from the Matter SDK - an intent of type
`ACTION_MATTER_COMMAND` will be received by the BroadcastReceiver implemented by
the content app. The intent would have the command id, cluster id as well as
corresponding payload data. Once the command handler is called, a response for
the command needs to be sent. An example for a receiver can be found within
`MatterCommandReceiver.java`. All the internal fields within the intents can be
found under `MatterIntentConstants` provided through the `common-api`.

### Example files

-   `MainActivity` - a sample content app
-   `MatterCommandReceiver` - a receiver for incoming commands
-   `MatterAgentClient` - a client that binds and manages connection to matter
    app agent service
