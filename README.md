# Turok 3: Recompiled

> **Made with AI.** Almost all of the code, documentation and tests in this repo were written by AI agents
> (Claude), directed and play-tested by a person. None of it is offered to the upstream projects it builds on, which
> don't take AI-written code.

A native PC port of the N64 *Turok 3: Shadow of Oblivion* (USA), made by static recompilation
([N64Recomp](https://github.com/N64Recomp/N64Recomp) + [N64ModernRuntime](https://github.com/N64Recomp/N64ModernRuntime) +
[RT64](https://github.com/rt64/rt64), with [RecompFrontend](https://github.com/N64Recomp/RecompFrontend)'s launcher
and menus): the game's own MIPS code translated to C and run natively, with RT64 drawing it. It is not an emulator. No
decompilation of the game exists, so its functions are found in the ROM's code, and what the Turok games share (the
code they run through the TLB, the finder, the libultra the runtime lacks) is in
[turok-engine](https://github.com/n64hub/turok-engine), this repo's `lib/turok-engine`.

The repo holds no ROM, no asset from the game and no data extracted from it: you supply your own ROM, and nothing in
git or CI is made from one except the symbol names, addresses and sizes in `syms/` and where the code is in the ROM
(`syms/layout.toml`).

**Status:** released for Windows and Linux (`releases/`). The desktop build plays from boot through the menus, the
character choice and the opening scene into the first level, the Breach, with sound, and keyboard and gamepad input.
The game hasn't been played past the Breach yet, and smooth motion, widescreen and mouse look are still to come. The
same game plays in a browser through WebGPU, at [n64hub.com/turok-3](https://n64hub.com/turok-3/) (`docs/web.md`).

## The ROM

*Turok 3: Shadow of Oblivion* (USA), as a big-endian `.z64` of 32 MiB with SHA-1
`a2e0a28dc0d3a48c7c02cb9e4c4b38fe04b2d436`. Other regions are not supported.

## Building

You need that ROM at `roms/turok3.us.z64`. On Windows: LLVM's clang-cl, CMake 3.31 and Ninja, and a clang with a
MIPS backend plus ld.lld for `patches/` (WSL's will do). On Linux: `clang lld libsdl2-dev libgtk-3-dev
libfreetype-dev`, CMake 3.31 and Ninja.

From the repo's root in a bash shell:

```sh
git -c core.longpaths=true submodule update --init --recursive   # paths past Windows' 260 characters
bash tools/recompile.sh -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl   # the game's code as C
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl \
      -DPATCHES_C_COMPILER=<MIPS clang> -DPATCHES_LD=<ld.lld>
ninja -C build Turok3Recompiled
```

On Linux, the same with `clang` and `clang++`. `tools/recompile.sh` builds N64Recomp from the submodule the first
time, then translates the game. Run `build/Turok3Recompiled` from the repo's root, pick the ROM with Load ROM, then
Start Game.

`syms/` is written from the ROM by `python3 lib/turok-engine/tools/gen_syms.py` (it needs `pip install spimdisasm`);
run it again only when the engine's finder or `syms/names.toml` changes.

## Releases

Each release is built where the ROM is, by `tools/release.sh` (its header says how), from the merge commit of a pull
request titled "Release v<version>" that bumps `version_string` in `src/main/main.cpp` and moves `releases/next.md`,
where each change adds what the player gets, to `releases/v<version>.md`, the release's notes.

## License

GPL-3.0. The game itself is not included and belongs to its owners.
