# CLAUDE.md

A static recompilation of the N64 *Turok 3: Shadow of Oblivion* (USA) with N64Recomp, N64ModernRuntime, RecompFrontend
and RT64. No decompilation exists: the functions are found in the ROM. Almost all of it is written by AI agents, so
this file is what the next one needs: the rules, where things are, and the facts about the game the code can't say on
its own.

## What lives where

This repo is the game: its symbols, its recompilation, the patches over it and the desktop build. What the Turok games
share is `lib/turok-engine` (n64hub/turok-engine, its README says what is there and how a game uses it): change it
there, then move this repo's submodule onto the commit.

- `syms/layout.toml`: where the game's code is in the ROM and where it runs (through the TLB, at 0x00200400 and
  0x00400000). `syms/names.toml`: the names known, kept for good, and the engine's natives (`turok_*`).
  `syms/turok3.us.syms.toml`: written from both and the ROM by `lib/turok-engine/tools/gen_syms.py`, never by hand.
- `us.toml`: N64Recomp's configuration, with the functions the engine replaces (`ignored`) and the hooks;
  `tools/recompile.sh` writes `RecompiledFuncs/` with it, and the audio microcode with `aspMain.us.toml`.
- `patches/` and `patches.toml`: MIPS C recompiled into PatchesLib. `ui_funcs.h`, `patch_helpers.h` and
  `recompui_event_structs.h` are RecompFrontend's.
- `src/main/main.cpp`: RecompFrontend's main with the GameEntry, which calls the engine at init, at each thread's
  creation and from the audio callbacks. `src/game/`: the settings and the natives the patches call.
- `lib/`: `turok-engine`, upstream `rt64`, `N64ModernRuntime` and `RecompFrontend` at the commits N64Recomp-browser's
  web forks are built on, and `N64Recomp-browser`, whose `patches/` `tools/build_n64recomp.sh` and `CMakeLists.txt`
  apply to the other three. `runtime-patches/` are this game's own over them.
- `tools/build_web.sh` and `tools/docker/Dockerfile.web`: the web page (`docs/web.md`), built with Emscripten in
  N64Recomp-browser's image. `RECOMP_KUSEG_WINDOW` (CMakeLists.txt) gives the TLB-mapped code its memory on the web.
- `tools/release.sh` and `tools/docker/Dockerfile.linux`: the desktop releases for Windows and Linux (the script's header
  says how). Each change a player would notice adds a line to `releases/next.md`; a release moves it to
  `releases/v<version>.md`.
- `icons/`: drawn by `tools/icon.ps1` from plain text and shapes.
- Comments beside the code, for how and why: each file's header. Read the header of the file you change and update
  it in the same commit.

## Rules

- No ROMs, ROM-derived assets or extracted game data in git, CI or PRs (`tools/ci/check_repo.py` checks). Only the
  names, addresses and sizes in `syms/` and the layout file. The ROM is `roms/turok3.us.z64`, never committed.
- CI builds everything but the ROM-generated sources, without linking, on Windows and Linux. turok-engine's CI calls
  it with `engine-ref` to build this repo's `main` against an engine commit.

## Facts about the game

Turok 3 runs Turok 2's engine with its renderer rewritten, so most of Turok 2's facts hold with other addresses
(`syms/names.toml` has Turok 2's names, found here by their code). Where it differs:

- The program is longer than the MB the console's boot code copies: t3_boot_main copies the next 512 KB (ROM 0x101000
  on) to 0x80100400 itself, word by word from the cartridge bus. The runtime can't read the bus, so
  `t3::map_code_sections` makes that copy before the game runs and us.toml jumps over the loop. The code and data run
  to 0x80114B60 (0x00314B60 through the TLB); bss follows.
- The second block is the ROM straight after the program (0x115760, 0x4DF50 bytes), which t3_VmInit copies into the top
  MB of an 8 MB console and maps at 0x00400000 with 1 MB pages. The VM server hands out regions from 0x00800000:
  the first one's pointers into itself (an audio bank's) are linked there. Its data is in main's but for a few tables
  between its functions, and its functions sit end to end otherwise, so the layout calls it packed: gen_syms then
  takes a few words of code between two functions as a start, and the game reaches some of them only through pointers.
- Files are RNC-packed (method 1 or 2), and the unpacker is hand-written assembly entered at the delay slot of its
  wrapper's `jr $ra`: a split there leaves every packed file as it was read, and the game crashes walking garbage.
- Some small game functions have the shape of libultra's debug monitor (`__rmonHitCpuFault` and its two callees), and
  n64sym names them so. N64Recomp then leaves them to the runtime, so `syms/names.toml` names them `t3_Records*`. A
  "Failed to find function" at an address the syms have is a function under a name the runtime replaces.
- The rest is Turok 2's: the FPU in FR mode (`turok::set_fpu_mode`), the VM server (`turok_vm_map_rom_region`), the
  DMA thread (`turok_pi_raw_start_dma`, OS_EVENT_PI with message 0x7D1), the audio pace (`turok_ai_get_status`), the
  default VI mode the game waits on (osViModeNtscLpn1 at 0x8010BEC0), osClockRate (0x80171B40), a Controller Pak in
  port 1 and osMotorInit for the Rumble Pak (`turok_motor_init`), and aspMain, byte for byte Turok 2's.
- The finder's mistakes show up as a stack that doesn't balance: a function split in two leaves $sp short after the
  call, and a crash far from it. Thread entries that never return are the only functions that should be unbalanced
  (the VM server and pager, the DMA, VI and PI managers' threads), and the exception code, which never runs.
