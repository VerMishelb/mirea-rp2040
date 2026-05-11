# RP2040 WIZnet OPC UA server based on open62541

This example replaces the previous hand-written OPC UA Binary prototype with an **open62541-based** OPC UA server.

## What is included

The server exposes a browsable object:

```text
Objects
└─ RP2040
   ├─ TestValue       ns=1;s=TestValue   Int32, read/write, default 42
   ├─ UptimeMs        ns=1;s=UptimeMs     UInt64, read-only
   ├─ LedState        ns=1;s=LedState     Boolean, read/write
   └─ ResetTestValue  Method, if open62541 was built with UA_ENABLE_METHODCALLS
```

Features compared with the first minimal version:

- OPC UA protocol stack is handled by open62541, not by hand-written packet encoding.
- Address space is browsable in clients such as UaExpert.
- Standard services such as Browse, Read, Write, Sessions and Subscriptions are handled by open62541 if enabled in the generated library.
- `TestValue` and `LedState` are protected with a FreeRTOS mutex.
- WIZnet TCP transport is separated in `w5x00_open62541_net.c`.

## Network settings

Edit `w5x00_opcua_config.h`:

```c
#define OPCUA_BOARD_IP_0 192
#define OPCUA_BOARD_IP_1 168
#define OPCUA_BOARD_IP_2 1
#define OPCUA_BOARD_IP_3 2
```

Default endpoint:

```text
opc.tcp://192.168.1.2:4840
```

## open62541 dependency

The project expects these files:

```text
libraries/open62541/open62541.c
libraries/open62541/open62541.h
```

Recommended branch: **open62541 v1.3.x**. The WIZnet transport uses the open62541 1.3 `UA_ServerNetworkLayer` plugin API. The 1.4/1.5 branch moved the networking layer to EventLoop/ConnectionManager, so it needs a different port.

From project root:

```bash
./tools/open62541/prepare_open62541_amalgamation.sh v1.3.15
```

If the script cannot fetch the files, see `libraries/open62541/README.md`.

## Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target w5x00_opcua
```

The UF2 should be created at:

```text
build/examples/opcua/w5x00_opcua.uf2
```

## Test from the laptop

Install a Python OPC UA client:

```bash
pip install opcua
```

Read `TestValue`:

```bash
python examples/opcua/tools/test_opcua_connection.py \
  --endpoint opc.tcp://192.168.1.2:4840 \
  --node ns=1;s=TestValue
```

Write and read `TestValue`:

```bash
python examples/opcua/tools/test_opcua_connection.py \
  --endpoint opc.tcp://192.168.1.2:4840 \
  --node ns=1;s=TestValue \
  --write 123
```

Read uptime:

```bash
python examples/opcua/tools/test_opcua_connection.py \
  --endpoint opc.tcp://192.168.1.2:4840 \
  --node ns=1;s=UptimeMs
```

## Notes

This is now an open62541 integration, but not yet a certified product build. For production use, the next steps are:

1. Generate open62541 with a small, explicit RP2040/WIZnet architecture configuration.
2. Tune `UA_NAMESPACE_ZERO`, subscriptions, diagnostics, method calls and memory allocation options.
3. Add certificates and security policies if the network is not trusted.
4. Add more W5500 sockets if multiple concurrent clients are required.
