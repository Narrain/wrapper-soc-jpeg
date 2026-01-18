# ISP IP Block

This directory contains a cycle‑accurate, modular Image Signal Processor (ISP) implemented in SystemC.  
The design uses a streaming ready/valid interface and a multi‑stage pipeline suitable for integration into a Vision SoC.

---

## 1. ISP architecture we’re implementing

```text
+-------------------------------------------------------------+
|                           ISP                               |
|                                                             |
|  clk, rst_n                                                 |
|                                                             |
|  +-------------------+      +-------------------+           |
|  |  Input Formatter  |----->|   Demosaic Stage  |           |
|  +-------------------+      +-------------------+           |
|            |                         |                      |
|            v                         v                      |
|  +-------------------+      +-------------------+           |
|  |  Color / Gamma    |----->|  Noise / WDR      |           |
|  +-------------------+      +-------------------+           |
|            |                         |                      |
|            v                         v                      |
|                     +-----------------------------+         |
|                     |     Output Formatter        |-------->|
|                     +-----------------------------+         |
|                                                             |
|  in_pixel,  in_valid,  in_ready                             |
|  out_pixel, out_valid, out_ready                            |
+-------------------------------------------------------------+


Streaming interface
The ISP uses a ready/valid streaming protocol:

in_pixel, in_valid, in_ready

out_pixel, out_valid, out_ready

A pixel transfers when:

valid == 1 AND ready == 1


Throughput is 1 pixel per cycle when both sides agree.

Pipeline structure
Each stage is implemented as an independent SC_CTHREAD:

Input Formatter

Demosaic Stage

Color / Gamma

Noise / WDR

Output Formatter

Each stage introduces 1 cycle of latency, and the pipeline can be extended with deeper algorithms.

Reset behavior
The ISP uses a synchronous active‑low reset:
async_reset_signal_is(rst_n, false);

