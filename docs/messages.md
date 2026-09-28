# CAN poruke — can-ecu-sim
## 0x100 — SENSOR_DATA (šalje Node A, perioda 10 ms)
| Signal        | Bitovi   | Tip     | Skaliranje       | Opseg         |
|---------------|----------|---------|-------------------|---------------|
| temperature   | bajt 0-1 | int16   | 0.1 °C / bit      | -40.0 – 125.0 |
| alive_counter | bajt 2   | uint8   | 1 / bit           | 0 – 15        |
| checksum      | bajt 3   | uint8   | -                 | 0 – 255       |

## 0x200 — CMD (šalje Node B, perioda 50 ms)
| Signal      | Bitovi   | Tip   | Skaliranje | Opseg  |
|-------------|----------|-------|------------|--------|
| target_val  | bajt 0-1 | uint16| 1 / bit    | 0-1000 |
| enable      | bit 16   | bool  | -          | 0/1    |
| checksum    | bajt 3   | uint8 | -          | 0-255  |

## 0x7FF — HEARTBEAT (šalju oba čvora, perioda 100 ms)
| Signal        | Bitovi | Tip   | Opseg |
|---------------|--------|-------|-------|
| node_id       | bajt 0 | uint8 | A=1, B=2 |
| alive_counter | bajt 1 | uint8 | 0-15  |#