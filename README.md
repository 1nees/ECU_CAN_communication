# can-ecu-sim

A simulated CAN network in C on Linux. Two ECU nodes exchange periodic sensor, command and heartbeat messages over a virtual CAN bus (SocketCAN `vcan0`). The controller validates every frame and switches to a safe state if the sensor node stops responding.

No hardware is needed. The code is split into layers so that only the lowest one (the CAN driver) is Linux-specific.

## What it does

```
 ┌───────────────┐                          ┌────────────────┐
 │  Node A       │      CAN bus (vcan0)     │  Node B        │
 │  "Sensor"     │ ───── SENSOR_DATA ─────► │  "Controller"  │
 │               │ ───── HEARTBEAT ───────► │                │
 │               │ ◄──────── CMD ────────── │                │
 └───────────────┘                          └────────────────┘
```

- **Node A (sensor)** sends a simulated temperature (`SENSOR_DATA`) every 10 ms and a `HEARTBEAT` every 100 ms.
- **Node B (controller)** receives the frames, checks the checksum and alive counter, and sends a `CMD` frame every 50 ms.
- If no heartbeat arrives for **300 ms**, Node B switches to a **safe state** and sends `CMD` with all zeros. When Node A returns, Node B resumes normal operation.

## Messages

Defined in [`dbc/system.dbc`](dbc/system.dbc) and [`docs/messages.md`](docs/messages.md).

| ID    | Name        | Sender | Period | Content |
|-------|-------------|--------|--------|---------|
| 0x100 | SENSOR_DATA | Node A | 10 ms  | temperature (int16, 0.1 °C/bit), alive counter (0-15), checksum |
| 0x200 | CMD         | Node B | 50 ms  | target value (uint16), enable (bit), checksum |
| 0x7FF | HEARTBEAT   | Node A | 100 ms | node id, alive counter (0-15) |

Checksum = XOR of the preceding data bytes. Byte order is little-endian (Intel).

## Architecture

```
src/
├── hal/          CAN driver interface (can_driver.h) and SocketCAN implementation
├── services/     Signal packing/unpacking and checksum (signals.c)
└── app/          node_sensor.c (Node A), node_controller.c (Node B)
```

| Layer | Responsibility | Platform-specific? |
|-------|----------------|--------------------|
| HAL   | `can_init`, `can_send`, `can_receive`, `can_receive_timeout`, `can_close` | Yes (SocketCAN) |
| Services | Convert physical values to bytes and back, compute and verify checksum | No |
| Application | Periodic sending, heartbeat monitoring, safe state | No |

The application and services layers only use the interface in `can_driver.h`. Running on a microcontroller would mean writing a new driver (for example `can_driver_stm32.c`) with the same functions. This port has **not** been implemented.

## Requirements

- Linux (tested on Ubuntu in a virtual machine)
- `gcc`, `can-utils`, kernel module `vcan`

```bash
sudo apt install -y build-essential can-utils
```

## Build and run

Set up the virtual CAN interface (needed again after every reboot):

```bash
./tools/setup_vcan.sh
```

Build:

```bash
mkdir -p build
gcc -Wall -Wextra -Isrc/hal -Isrc/services src/app/node_sensor.c src/services/signals.c src/hal/can_driver_socketcan.c -o build/node_sensor -lm
gcc -Wall -Wextra -Isrc/hal -Isrc/services src/app/node_controller.c src/services/signals.c src/hal/can_driver_socketcan.c -o build/node_controller
gcc -Wall -Wextra -Isrc/hal -Isrc/services src/services/signals_test.c src/services/signals.c -o build/signals_test
```

Run in three terminals:

```bash
./build/node_controller      # terminal 1
candump -td vcan0            # terminal 2, watch the bus
./build/node_sensor          # terminal 3
```

Signal packing test:

```bash
./build/signals_test
```

## Tests and results

### Signal test

`signals_test` packs a sensor frame, unpacks it, then flips one bit and checks that the corrupted frame is rejected.

```
Bajtovi: EB 00 03 E8
OK: temp=23.5 counter=3
Pokvaren frejm je odbijen (checksum)
```

### Bus timing

Captured with `candump -td vcan0`: `SENSOR_DATA` arrives about every 10 ms and `HEARTBEAT` about every 100 ms. Occasional single intervals of 6-14 ms or 22 ms come from the virtual machine's scheduler, but consecutive intervals add up to the nominal period because the sender sleeps until absolute times (`clock_nanosleep` with `TIMER_ABSTIME`), so the period does not drift.

### Failure scenario (manual)

| Scenario | Expected | Observed |
|----------|----------|----------|
| Stop Node A (Ctrl+C) | Node B prints `HEARTBEAT TIMEOUT` after about 300 ms, `CMD` becomes `00 00 00 00` | _fill in after testing_ |
| Restart Node A | Node B prints `Node A alive -> NORMAL operation`, `CMD` carries data again | _fill in after testing_ |
| Corrupted frame (checksum mismatch) | Frame dropped, counted as bad | _fill in after testing_ |
