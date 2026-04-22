# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Embedded firmware for the **MOD SOM EPSI APF** — a scientific instrument deployed on APEX profiling floats to measure ocean microstructure (turbulence, dissipation rates). Runs on Silicon Labs EFM32GG11B (ARM Cortex-M4) with Micrium OS (preemptive RTOS). Developed at Multiscale Ocean Dynamics, Scripps Institution of Oceanography, UC San Diego.

## Build System

This is a **Simplicity Studio v5 / Eclipse CDT managed-build project**. There is no standalone Makefile or CMake — the build is controlled by `.cproject`.

- **To build**: Open in Simplicity Studio v5, select the `GNU ARM v7.2.1 - Debug` build configuration, and build via the IDE.
- **Toolchain**: GNU ARM v7.2.1, targeting EFM32GG11B420F2048GQ100
- **Output**: ELF/HEX/BIN in `GNU ARM v7.2.1 - Debug/`
- **Key compiler defines**: `EFM32GG11B420F2048GQ100=1`, `PROJECTNAME`, `TOTO`
- **No optimization** (`-O0`), full debug symbols

There is no unit test framework. Testing is done through the interactive shell interface over USART.

## Architecture

### Module System

Each subsystem lives under `mod/<name>/` and follows a consistent pattern:
- `mod_som_<name>.c/h` — driver/task logic
- `mod_som_<name>_cmd.c` — shell commands for interactive testing

Modules are enabled/disabled by feature flags in `mod/cfg/mod_som_cfg.h`. The main entry point (`src/mod_som_main.c`) includes and initializes only the enabled modules.

### Key Modules

| Module | Path | Role |
|--------|------|------|
| **Core** | `mod/mod_som.c` | Board init, memory pools, task creation, watchdog |
| **APF** | `mod/apf/` | APEX float protocol: CRC-checked command/response packets over LEUART0 , Merges Spectral data + CTD data streams with metadata for SD card and APF output|
| **EFE** | `mod/efe/` | 7-channel AD7124 ADC acquisition via LDMA; 625Hz sampling |
| **SBE41** | `mod/sbe41/` | CTD (conductivity-temperature-depth) over USART4; |
| **EFE OBP** | `mod/efe_obp/` | On-board processing for spectral analysis of EFE data using CMSIS DSP |
| **SDIO** | `mod/sdio/` | FatFS on SD card; HBLib SDIO driver |
| **Settings** | `mod/settings/` | Persistent configuration storage |
| **Shell** | `mod/shell/` | CLI framework; each module registers its commands here |
| **I/O** | `mod/io/` | Buffered I/O with message queues |
| **Calendar** | `mod/calendar/` | Timestamp / RTC management |
| **Voltage** | `mod/voltage/` | Battery / voltage monitoring |
| **DSP** | `mod/DSP/` | CMSIS DSP library (FFT, statistics) — used by EFE OBP |

Top-level folders outside `mod/` (`kernel/`, `service/`, `emlib/`, `CMSIS/`, `Drivers/`, `cpu/`, `ports/`) are embedded SDK code and should not be modified. The exception is `FatFS/` (and `mod/sdio/HBLib/FatFS/`), which is open-source and may be touched when needed.

### Data Flow

```
EFE ADC (LDMA ISR) ──────────────────────┐
                                          ▼
SBE41 USART4 (ISR) ──────────────→  Aggregator (producer) task → SD card (FatFS)/OBP module
                                          │
                                          └──→ OBP module → SD card → APEX float (LEUART0)
```

The RTOS producer-consumer pattern is central: sensor ISRs/tasks fill ring buffers, consumer tasks drain them. Task priorities: I/O task (16), data consumers (18), main task (21).

### Communication Interfaces

Defined in `mod/cfg/mod_som_bsp.h` (89 KB board support file — all pin assignments are here):

- **USART5** @ 230400 baud — main console, usually off when interacting with the APEX float 
- **LEUART0** @ 9600 baud —  APEX platform link
- **USART2** — debug header
- **USART1** — mezzanine board (J9 header); used for spoofing/simulator input
- **USART4** @ 9600 baud — SBE41 - CTD data acquisition
- **SPI/SDIO** — SD card

### Configuration

- `mod/cfg/mod_som_cfg.h` — master feature-flag file; toggle modules here
- `mod/cfg/mod_som_bsp.h` — all hardware pin/UART assignments
- `mod/cfg/os_cfg.h`, `cpu_cfg.h`, `rtos_cfg.h` — Micrium OS tuning

### On-Board Processing (OBP)

Current branch (`obp_testing_claude`) is testing on-board dissipation calculation. The spoof/simulator path generates synthetic sensor data via USART1 to exercise OBP without deploying hardware. Key files: `mod/efe_obp/mod_som_efe_obp.c` and `mod/efe_obp/mod_som_efe_obp_calc.c`.

### Error Handling

Tasks accumulate errors in a counter (max `MOD_SOM_MAX_ERROR_CNT` = 5) before triggering a restart. Watchdog timer (`MOD_SOM_DEBUG_WDOG`) has a 2-second timeout. Do not add defensive error handling beyond this pattern — the framework handles recovery.

## Naming conventions

### Variables

- All lower case
- Separating between words in name should be an underscore `_`
- There should be an abbreviation standard that is common to all programmers in the group
- A defined variable type should be concatenated with `_t`
- A define function should be concatenated with `_f`
- A variable that has a measureable units should append the unit to the end

#### Example:

- `mod_som_mod_t` - a defined type
- `sonar_ctlr_cnt` - a variable

### Suffices

- `_t` - type definition
- `_f` - function definition
- `_ptr` - pointer
- `_fd` - file descriptor
- `_file` - file

#### Type definitions

- suffix: `_t`
- Example:
  - `uint32_t`
  - `int64_t`

#### Function naming

- suffix: `_f`
- Example:
  - `do_something_funny_f()` - a defined function

#### Variable with units
- Bytes Example:
  - `data_length_byte` - a defined variable describing the length of data chunk measured in bytes
  - `data_leghth_word` - a defined variable describing the length of data chunk measured in words (sets of 4 bytes)
- Time units:
  - `sampling_period_us` - a defined variable describing the sampling period of an instrument measured in micro seconds
  - `averaging_period_ms` - a defined variable describing the averaging period of a set of data
  

#### Macro Defines

- Should be ALL CAPS
- Separation between words in the name should be an underscore `_`
- An abbreviation standard that is known to all programmers in the group should apply to the naming convention

### Abbreviations

An abbreviation standard that is known to all programmers in the group should apply to the naming convention
