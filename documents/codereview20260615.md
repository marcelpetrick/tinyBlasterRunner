# Code Review — Tiny Blaster Runner — 2026-06-15

Reviewer: internal architectural review

---

## The 10 worst things

### 1. One 1 000-line God file

`main/shooter_game.c` contains spawn logic, physics, AABB collision,
particle system, enemy AI, HUD, menu screens, splash screen, NVS
persistence, and all rendering — in a single translation unit. Individual
subsystems are hard to understand, modify, or unit-test in isolation.
Split at minimum into `game_physics.c`, `game_render.c`, and
`game_ui.c`.

### 2. Sprite data in a header that defines static arrays

`shooter_sprites.h` uses `static const uint16_t ROBOT_RUN0[...]` etc.
directly in the header. Every translation unit that includes it gets its
own copy of ~10 KB of pixel data. Currently only one TU includes it, but
the pattern is a latent ODR violation waiting to happen. Data definitions
belong in a `.c` file; the header should expose `extern const` declarations.

### 3. Fixed pool sizes expressed in two independent places

`#define SG_MAX_ENEMIES 4` lives in `shooter_game.c`, but the struct
field `sg_enemy_t enemies[4]` uses the literal `4`. The same mismatch
exists for all other pools (`projectiles[3]`, `coins[12]`, etc.). A
change to any pool size requires two edits; the compiler will not catch a
mismatch.

### 4. No dt_ms clamping — physics jumps on stall

`shooter_game_update` passes `dt_ms` directly to velocity integration,
timer decrements, and spawn-interval checks. A 200 ms frame stall (NVS
flash erase, SPI bus contention) causes the player position to jump by
6× a normal step, and can trigger multiple simultaneous enemy spawns.
Clamping to e.g. `dt_ms = MIN(dt_ms, 50u)` costs one line.

### 5. NVS write is synchronous on the game-loop task

`nvs_save_hi()` is called from inside `shooter_game_update()` when a new
high score is set. NVS commits can take 10–50 ms on ESP32 (flash sector
erase). This causes a visible frame drop at the exact moment the player
most wants feedback. The save should be dispatched to a low-priority
background task.

### 6. Weak LCG RNG seeded from a literal constant

`g->rng` is initialised to `1337` in `reset_play()`. Every game session
produces the exact same spawn sequence. A different skill level may
exploit this. Seed from `esp_random()` at init time instead.

The underlying generator is also a 32-bit Lehmer LCG. Modulo operations
on low-bit values (`% 3`, `% 200`) exhibit strong linear correlation.
A simple xorshift32 would be meaningfully better for no additional cost.

### 7. Enemy type dispatch duplicated in four locations

Adding a new enemy type currently requires touching spawn (`try_spawn_enemy`),
scroll speed (ternary chain), off-screen culling, two collision blocks,
and the render loop — five separate sites. A small `enemy_def_t` table
(one entry per type: width, height, speed_mul, y_kind, sprite pair)
would reduce this to a single table lookup at each call site.

### 8. Particle pool has no eviction — important particles silently drop

When all 24 `sg_particle_t` slots are occupied, `alloc_particle` returns
`NULL` and `spawn_explosion` / `spawn_sparkle` silently do nothing. The
player's death explosion, which is a critical visual cue, can simply not
appear mid-game if the pool is full. At minimum, reserve two slots for
explosion events; ideally add priority-based eviction.

### 9. `display_smoke_test.c` state is stale in `ideas.md` comment

`ideas.md #2` (pipeline) describes the smoke test as "dead code" and "not
registered in CMakeLists.txt", but both of those facts were fixed during
the pipeline commit. The ideas backlog retains the old pre-fix status
description without being fully updated, leaving an accurate-at-the-time
but misleading historical artefact.

### 10. `build_flash.sh` timing uses `$SECONDS` but is fragile cross-shell

`$SECONDS` is a Bash built-in that counts seconds since the current shell
started — not since the script started. If the ESP-IDF source step at the
top of the script takes several seconds (e.g., first run), `t0` is
already non-zero and the elapsed time is correct only by coincidence.
Using `t0=$(date +%s)` and `elapsed=$(( $(date +%s) - t0 ))` is portable
and unambiguous.

---

## The 5 best things

### 1. Software framebuffer eliminates display tearing

Drawing all game elements to a 135×240 uint16 RAM buffer and flushing in
one DMA-backed SPI burst per frame is the correct pattern for the ST7789.
Scattered GRAM writes mid-scan produce visible tearing at 30 FPS; the
framebuffer approach makes it impossible regardless of how many draw calls
are made. This is a solid architectural decision that scales for free.

### 2. Pure-logic extraction enables host-side unit testing

`game_math.c` isolates RNG and AABB functions in a file with zero
ESP-IDF headers. The `test/` directory compiles those functions with plain
GCC + Unity, producing a native test binary that runs in milliseconds
without any hardware. The 10-test suite is wired into `ci.sh` and passes
on every run. This is the right way to get test coverage on embedded C
without a hardware-in-the-loop dependency.

### 3. Kconfig smoke-test opt-in follows ESP-IDF conventions

`CONFIG_RUN_SMOKE_TEST` is defined in `Kconfig.projbuild` and guards the
display self-test via `#ifdef` only in `app_main.c`. The game code has
zero awareness of the test mode. The opt-in follows the ESP-IDF component
configuration pattern exactly; any developer already familiar with the SDK
will know exactly where to find the flag and how to enable it.

### 4. Fixed-rate game loop with delta-time compensation

`app_main.c` drives the game at 33 ms ticks using `vTaskDelay` while
passing the actual elapsed `dt_ms` to `shooter_game_update`. Physics,
timer decrements, and animation state all use the measured delta rather
than assuming a perfect tick. Frames that run long do not accumulate
error; fast frames do not skip physics steps. This is textbook
fixed-rate-loop-with-delta-time and it works correctly.

### 5. Layered build tooling with a clear separation of concerns

`.clang-format` encodes style once. `build_flash.sh` encapsulates device
operations (build / flash / monitor) with a human-readable summary line.
`ci.sh` composes the above into a single gate (format → build → test).
The pre-commit hook enforces the gate at commit time. Each layer is
independently runnable, composable, and understandable. A new contributor
can read `ci.sh` in 30 seconds and know exactly what "green" means.
