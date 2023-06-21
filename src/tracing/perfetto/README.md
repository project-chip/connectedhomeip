This contains a data logging and tracing backend based on Perfetto.

Uses the perfetto submodule and guides based on
https://perfetto.dev/docs/instrumentation/tracing-sdk

## Capturing single app tracing

Example capturing a trace file output for chip-tool during pairing:

```
out/linux-x64-chip-tool/chip-tool \
    pairing onnetwork 1 20202021  \
    --trace-to perfetto:$HOME/tmp/test_perfetto.log
```

where the trace file is available through the Perfetto UI

## Capturing potentially multiple app tracing

An example capturing all `track_event` entries is availabe in `matter.cfg` and
the perfetto repo has more examples.

To capture using a central daemon on linux, you have to start the daemon:

```
traced --background
```

Then start perfetto (using a config in text format):

```
perfetto -o ~/tmp/example.log --txt -c src/tracing/perfetto/matter.cfg
```

And finally start the app(s) with system tracing set up:

```
out/linux-x64-chip-tool/chip-tool \
    pairing onnetwork 1 20202021  \
    --trace-to perfetto
```
