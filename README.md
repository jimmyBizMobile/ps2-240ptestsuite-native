# PS2 240p Test Suite (Native)

A native PlayStation 2 calibration suite for CRT displays, patterned after
Artemio Urbina's 240p Test Suite. Provides grid, monoscope, PLUGE, and color
bar patterns with a 240p/480i toggle, intended for calibrating a pro CRT
(developed against a Sony PVM-20M2MDJ over RGB/component).

Written in C against PS2SDK + gsKit. Unlike a JavaScript (AthenaEnv) build,
this native version controls the GS DISPLAY registers directly, so the 240p
horizontal width matches conventional broadcast geometry rather than gsKit's
wider default.

## Patterns


**Grid** — geometry and convergence; adjust display size/centering until the
  outer border is just visible.

**Monoscope** — overall geometry, sharpness, and linearity reference.
**PLUGE** — black level (Brightness). Lower Brightness until the side bars
  disappear, then raise until they are just barely visible.

**Color bars** — black/white/color levels. A full 8-bit 0x00–0xFF ramp per
  channel (red, green, blue, white). Set Contrast so the top two steps (E and
  F) are just distinguishable; repeat per color channel.

## Controls


**D-pad Up/Down** — move through the menu
**Cross (✕)** — select menu item
**Circle (○)** — back to menu
**Triangle (△)** — toggle 240p / 480i
**Select** — exit


## Building

Requires Docker. The build runs in the `ps2dev/ps2sdk-ports` image; no local
toolchain needed.

```bash
docker compose run --rm build      # builds the ELF
docker compose run --rm shell      # interactive shell in the build image