# Alarm Base and its derivations

Alarm Base is a pseudo cluster. It has no cluster ID of its own. It exists only
to be derived from by alarm clusters such as Dishwasher Alarm and Refrigerator
Alarm.

The code-driven implementation lives in this directory (`AlarmBaseCluster`,
`Delegate`, and tests). Concrete clusters subclass `AlarmBaseCluster` and provide
cluster-specific event generation (for example `DishwasherAlarmCluster` and
`RefrigeratorAlarmCluster`).

## Fixed attributes: Supported and Latch

In the Alarm Base cluster definition, **Supported** and **Latch** are **fixed
attributes**. They describe which alarms the device implementation supports and,
when the Reset feature is present, which alarm bits latch. Their values are
determined by the product configuration (typically ZAP / data model defaults at
build time), not changed at runtime through the server application API.

For that reason, the code-driven Alarm Base implementation:

- Initializes `Supported` and `Latch` once from `AlarmBaseCluster::Config`
  (`.supported` and `.latch`) when the cluster is created.
- Stores them as `const` members on `AlarmBaseCluster` (`mSupported`, `mLatch`).
- Exposes **read-only** accessors: `GetSupportedValue()` and `GetLatchValue()`.
- Does **not** implement `WriteAttribute` for these attributes.

### Removed setters

The legacy Ember-based `DishwasherAlarmServer` exposed `SetSupportedValue()` and
`SetLatchValue()` on its application-facing interface. Those methods were
**removed** in the Alarm Base migration because they allowed runtime mutation of
fixed attributes, which does not match the Alarm Base specification.

Mutable runtime state remains available through:

- `SetMaskValue()` / `GetMaskValue()`
- `SetStateValue()` / `GetStateValue()`
- `ResetLatchedAlarms()` (when the Reset feature is enabled)

To change Supported or Latch for a product, update the cluster configuration in
the data model (ZAP / `.matter`) and ensure `CodegenIntegration` passes the
intended defaults into `AlarmBaseCluster::Config` at cluster construction.
