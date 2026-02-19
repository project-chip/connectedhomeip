### Upgrade notes

The Binding table size was changed from `MATTER_BINDING_TABLE_SIZE` to
`CHIP_CONFIG_MAX_BINDING_ENTRIES_PER_FABRIC * CHIP_CONFIG_MAX_FABRICS`. The
previous value was defined in `src/app/util/config.h` by default and it is
defined in `src/lib/core/CHIPConfig.h`.
