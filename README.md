 # epc - Embedded Parameters Compiler

epc (Embedded Parameters Compiler) is a lightweight DSL and expression engine designed to describe software parameters for embedded applications.
<br />Its goal is to let firmware developers define configuration values, thresholds, constants, and other application-level parameters in a structured, human-readable format,
and compile them into binary data that the firmware can directly use.

---

## Features

- JSON/DTC-inspired syntax with `{}` for nodes and `=` for properties
- Supports arithmetic (`+ - * / %`) and bitwise operations (`& | ^ ~ << >>`)
- Typed values: `u8`, `u16`, `u32` for accurate representation in firmware
- Easy to extend with new operators or additional node types
- Built-in C preprocessor support:
  - `#define` for constants
  - `#include` for parameter headers
- Generates binary output ready for firmware consumption

# Getting started

## Prerequisites

- gcc
- cpp

## Usage

`$ ./epc input_file.params output.bin`

## Build

`$ make`

## Example

- config.h:

```c
#define MAX_CLIENT	10
#define TIMEOUT_MS	60 * 1000

#define MAX_VOLUME	100
#define DEFAULT_VOLUME	MAX_VOLUME - 90
```

- example.params:

```c
#include "config.h"

{
    network {
        max_clients = <MAX_CLIENTS, u8>;
        timeout_ms = <TIMEOUT_MS, u16>;
    };

    audio {
        default_volume = <DEFAULT_VOLUME, u8>;
        gain_factors = <0x5, u16>, <0x6, u16>, <0xA, u16>;
    };
}
```

- Generated binary:

```bash
$ ./epc example/example.params example.bin
$ xxd example.bin
00000000: 0a60ea0a 05000600  .`......
00000008: 0a00
```
