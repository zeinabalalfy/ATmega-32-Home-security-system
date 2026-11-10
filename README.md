# Bi-Directional Dual ATmega32 Security System

An embedded security and remote monitoring system designed around two **ATmega32** microcontrollers communicating over a full-duplex **USART** serial bus. The system utilizes hardware external interrupts for instant local/remote toggling, bit-packed serial packets for real-time telemetry, and a software-synthesized audio alarm driven through an NPN transistor amplifier circuit.

---

## Architecture Overview

[ Local Control Unit (MCU 1) ] <======== Full-Duplex USART ========> [ Remote Monitor Unit (MCU 2) ]
├── PIR Motion Sensor (PA0)             (9600 Baud)                  ├── Status LEDs (Port C)
├── Magnetic Door Sensor (PA1)                                       └── Arm/Disarm Button (PD2 / INT0)
├── System Status LEDs (Port C)
├── Arm/Disarm Button (PD2 / INT0)
└── 500Hz Audio Alarm (PB0 -> 2N2222 Transistor -> Speaker)

The system splits functionality across two distinct nodes:
1. **Main Security Hub (MCU 1):** Processes local sensor inputs, manages security states (Armed/Disarmed), drives local visual indicators, synthesizes the audio alarm waveform, and streams compressed telemetry bytes to MCU 2.
2. **Remote Monitor (MCU 2):** Mirrors system state and sensor indicators in real time and can trigger a remote toggle command back to MCU 1 over USART.

---

## Technical Features & Design Highlights

* **Hardware External Interrupts (`INT0`):** Utilizes `INT0` on `PD2` with falling-edge detection for zero-latency local and remote Arm/Disarm toggling.
* **Internal Pull-Up Resistors:** Configured via software (`PORTD |= (1 << PD2)`) to eliminate external pull-up components, reducing BOM cost and circuit footprint.
* **Bit-Packed Telemetry Frame:** Optimizes USART bandwidth by compressing three boolean states into a single 8-bit payload:
  $$\text{Payload Byte} = \text{Armed Bit} \mid (\text{PIR Bit} \ll 1) \mid (\text{Mag Bit} \ll 2)$$
* **Software Audio Waveform Generator:** Generates a ~500Hz square wave by rapidly toggling pin `PB0` with 1ms high/low intervals, driving an analog speaker model.
* **Transistor Driver Power Stage:** Uses a **2N2222 NPN BJT** in common-emitter configuration (1kΩ base resistor) to safely isolate the microcontroller pin from the high-current/12V load of the alarm output.

---

## Hardware Configuration & Pinout Mapping

### Main Security Hub (MCU 1 - Right Unit)

| Component | ATmega32 Pin | Logic Direction / Configuration |
| :--- | :--- | :--- |
| **PIR Motion Sensor** | `PA0` (Pin 40) | Input (Active HIGH) |
| **Magnetic Door Sensor** | `PA1` (Pin 39) | Input (Active HIGH) |
| **Speaker / Alarm** | `PB0` (Pin 1) | Output (Drives 2N2222 Base via 1kΩ) |
| **Arm/Disarm Button** | `PD2` (Pin 16) | Input (`INT0` External Interrupt w/ Internal Pull-up) |
| **Armed LED (Red)** | `PC0` (Pin 22) | Output (Active HIGH via 330Ω) |
| **Disarmed LED (Green)** | `PC1` (Pin 23) | Output (Active HIGH via 330Ω) |
| **PIR Status LED (Yellow)**| `PC2` (Pin 24) | Output (Active HIGH via 330Ω) |
| **Mag Status LED (Yellow)**| `PC3` (Pin 25) | Output (Active HIGH via 330Ω) |
| **USART RX / TX** | `PD0` / `PD1` | Serial Lines connected cross-over to MCU 2 |

### Remote Monitor (MCU 2 - Left Unit)

| Component | ATmega32 Pin | Logic Direction / Configuration |
| :--- | :--- | :--- |
| **Remote Toggle Button** | `PD2` (Pin 16) | Input (`INT0` External Interrupt w/ Internal Pull-up) |
| **Remote Status LEDs** | `PC0`–`PC3` | Outputs (Mirrors MCU 1 telemetric frame) |
| **USART RX / TX** | `PD0` / `PD1` | `RXD` connected to MCU 1 `TXD`, `TXD` to MCU 1 `RXD` |

---

## Software Implementation Details

### Bitwise Operation Strategy
* **Setting Output Pins:** `PORTC |= (1 << PC0);` (Applies a bitmask without disturbing adjacent bits).
* **Clearing Output Pins:** `PORTC &= ~(1 << PC1);` (Bitwise AND with inverted mask).
* **Safe Input Reading:** `(PINA & (1 << PA0)) ? 1 : 0;` (Isolates bit 0 regardless of port noise or state of PA1).

### Interrupt Service Routines (ISRs)
```c
// Toggle system state instantly on local button press
ISR(INT0_vect) {
    system_armed = !system_armed;
}
// Receive remote 'T' (Toggle) command from Remote Monitor over USART
ISR(USART_RXC_vect) {
    unsigned char received_cmd = UDR;
    if (received_cmd == 'T') {
        system_armed = !system_armed;
    }
}
```

---

## How to Build & Simulate

### Prerequisites
* **IDE:** Microchip Studio (formerly Atmel Studio) or `avr-gcc` toolchain.
* **Simulator:** Proteus Design Suite 8.x or later.

### Steps
1. **Compile Code:**
   * Open the MCU 1 and MCU 2 C projects in Microchip Studio.
   * Target Chip: `ATmega32` | Clock Frequency: `1000000UL` (1 MHz).
   * Build both solutions (`F7`) to generate `.hex` output binaries.

2. **Configure Proteus Schematic:**
   * Double-click MCU 1 and attach `main_hub_mcu1.hex`. Set Clock Frequency to `1MHz`.
   * Double-click MCU 2 and attach `remote_monitor_mcu2.hex`. Set Clock Frequency to `1MHz`.

3. **Run Simulation:**
   * Press **Play** in Proteus.
   * Click either button (`PD2`) to alternate system states between **Disarmed** (Green LED) and **Armed** (Red LED).
   * Toggle sensor inputs on `PA0` or `PA1` to `1`. When armed, the speaker output on `PB0` activates the 2N2222 driver stage to play the 500Hz synthesized alarm tone.
