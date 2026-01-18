2️⃣ Encoder — Cycle‑Accurate Architecture
text
+-----------------------------------------------------------------------+
|                               Encoder                                 |
|                                                                       |
|   +-------------------+      +-------------------+                    |
|   |   Config Regs     |<-----|   Reg Bus (MCU)   |                    |
|   +-------------------+      +-------------------+                    |
|            |                         ^                                |
|            v                         |                                |
|   +-------------------+      +-------------------+                    |
|   |   Ingest FIFO     |----->|  MB / Block Maker |                    |
|   +-------------------+      +-------------------+                    |
|                                      |                                 |
|                                      v                                 |
|                            +-------------------+                       |
|                            | Transform / Quant |                       |
|                            |   (DCT/Q)         |                       |
|                            +-------------------+                       |
|                                      |                                 |
|                                      v                                 |
|                            +-------------------+                       |
|                            |  Entropy Coder    |                       |
|                            +-------------------+                       |
|                                      |                                 |
|                                      v                                 |
|                            +-------------------+                       |
|                            | Bitstream FIFO    |                       |
|                            +-------------------+                       |
|                                      |                                 |
|                                      v                                 |
|                            +-------------------+                       |
|                            | DMA Engine        |-----> DDR             |
|                            +-------------------+                       |
|                                                                       |
|   IRQ_out (frame done, buffer full, error)                            |
+-----------------------------------------------------------------------+
Cycle‑accurate modeling notes
Block maker

FSM that groups pixels into 8×8 or 16×16 blocks

Transform/Quant

Multi‑cycle pipeline (e.g., 8 cycles per block)

Entropy coder

Bit‑serial or word‑serial, explicit cycle cost

DMA engine

AXI/TLM master with burst timing

Backpressure

If bitstream FIFO fills, upstream stalls

libjpeg-turbo folder should be parellel to encoder
https://github.com/sailfishos-mirror/libjpeg-turbo