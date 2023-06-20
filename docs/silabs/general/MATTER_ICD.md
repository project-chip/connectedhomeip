# Matter Intermittently Connected Devices (ICD)

With the 1.1 release, Matter introduced the concept of Intermittently Connected Devices (ICD) in the SDK and in the specification.
An Intermittently Connected Device is the Matter representation of a device that is not always reachable.
This covers battery-powered devices that disable their underlying hardware when in a low-power mode or devices that can be disconnected from the network, like a phone app.

This page focuses on features designed to improve the performance and reliability of battery-powered devices.

## Subscription Maximum Interval

The subscription mechanism is used by ecosystems and controllers to receive attribute change updates and liveness checks.
The maximum interval of a subscription request is what defines the frequency at which a device will send a liveness check if there are no attribute changes.

Within the subscription request / response model, a device has the opportunity to decide the maximum interval at which it will send its liveness check (Empty Report Update). 
The device can set a maximum interval within this range:

```shell
MinIntervalRequested ≤ MaxInterval ≤ MAX(1h, MaxIntervalRequested)
```

The following table shows the subscribe response fields.
| Action Field | Type | Description |
|-|-|-|
| SubscriptionId | uint32 | identifies the subscription |
| MaxInterval | uint16 | the final maximum interval for the subscription in seconds |

### Maximum Interval Negotiation
For a device to be able to Negotiate the Maximum Interval when receiving a subscribe request, 
the application first needs to implement the `ApplicationCallback` class from the `ReadHandler.h` header.

```cpp
/*
 * A callback used to interact with the application.
 */
class ApplicationCallback
{
public:
    virtual ~ApplicationCallback() = default;
    /*
     * Called right after a SubscribeRequest has been parsed and processed. This notifies an interested application
     * of a subscription that is about to be established. It also provides an avenue for altering the parameters of the
     * subscription (specifically, the min/max negotiated intervals) or even outright rejecting the subscription for
     * application-specific reasons.
     *
     * TODO: Need a new IM status code to convey application-rejected subscribes. Currently, a Failure IM status code is sent
     * back to the subscriber, which isn't sufficient.
     *
     * To reject the subscription, a CHIP_ERROR code that is not equivalent to CHIP_NO_ERROR should be returned.
     *
     * More information about the set of paths associated with this subscription can be retrieved by calling the appropriate
     * Get* methods below.
     *
     * aReadHandler:            Reference to the ReadHandler associated with the subscription.
     * aSecureSession:          A reference to the underlying secure session associated with the subscription.
     *
     */
    virtual CHIP_ERROR OnSubscriptionRequested(ReadHandler & aReadHandler, Transport::SecureSession & aSecureSession)
    {
        return CHIP_NO_ERROR;
    }
    /*
     * Called after a subscription has been fully established.
     */
    virtual void OnSubscriptionEstablished(ReadHandler & aReadHandler){};
    /*
     * Called right before a subscription is about to get terminated. This is only called on subscriptions that were terminated
     * after they had been fully established (and therefore had called OnSubscriptionEstablished).
     * OnSubscriptionEstablishment().
     */
    virtual void OnSubscriptionTerminated(ReadHandler & aReadHandler){};
 };
```

The second step is registering the callback object to the Interaction Model Engine.
```cpp
// Register ICD subscription callback to match subscription max intervals to its idle time interval
chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&mICDSubscriptionHandler);
```

The `ICDSubscriptionCallback` class in `examples/platform/silabs/ICDSubscriptionCallback.cpp` gives an example implementation of the Maximum Interval negotiation a device can do when receiving a Subscription Request.

```cpp
CHIP_ERROR ICDSubscriptionCallback::OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler,
                                                            chip::Transport::SecureSession & aSecureSession)
{
    using namespace chip::System::Clock;

    Seconds32 interval_s32 = std::chrono::duration_cast<Seconds32>(CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL);

    if (interval_s32 > Seconds16::max())
    {
        interval_s32 = Seconds16::max();
    }
    uint32_t decidedMaxInterval = interval_s32.count();

    uint16_t requestedMinInterval = 0;
    uint16_t requestedMaxInterval = 0;
    aReadHandler.GetReportingIntervals(requestedMinInterval, requestedMaxInterval);

    // If requestedMinInterval is greater than IdleTimeInterval, select next wake up time as max interval
    if (requestedMinInterval > decidedMaxInterval)
    {
        uint16_t ratio = requestedMinInterval / decidedMaxInterval;
        if (requestedMinInterval % decidedMaxInterval)
        {
            ratio++;
        }

        decidedMaxInterval *= ratio;
    }

    // Verify that decidedMaxInterval is an acceptable value
    if (decidedMaxInterval > Seconds16::max().count())
    {
        decidedMaxInterval = Seconds16::max().count();
    }

    // Verify that the decidedMaxInterval respects MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)
    uint16_t maximumMaxInterval = std::max(kSubscriptionMaxIntervalPublisherLimit, requestedMaxInterval);
    if (decidedMaxInterval > maximumMaxInterval)
    {
        decidedMaxInterval = maximumMaxInterval;
    }

    return aReadHandler.SetReportingIntervals(decidedMaxInterval);
}
```

The Silabs implementation is enabled by default when building an SED application.
```bash
  # Use default handler to negotiate subscription max interval
  chip_config_use_icd_subscription_callbacks = enable_sleepy_device
```

To disable the feature when building a sleepy end device (SED) application, this argument needs to be added to the build command.
```bash
chip_config_use_icd_subscription_callbacks = false
```

## Persistent Subscriptions
Persistent subscriptions were added to Matter as a means to ensure that an ICD can re-establish its subscription and by extension its secure session to a subscriber in the event of a power cycle.
When a device accepts a subscription request, it will persist the subscription.
When the device reboots, it will try to re-establish its subscription with the subscriber.
If the subscription is torn down during normal operations or if the re-establishement fails,
the subscription will be deleted.

Persistent subscriptions are enable by default when enabling an SED example.
```bash
# Use persistent subscriptions for ICDs
chip_persist_subscriptions = enable_sleepy_device
```
To disable the feature when building an SED application, this argument needs to be added to the build command.
```bash
chip_persist_subscriptions = false
```
