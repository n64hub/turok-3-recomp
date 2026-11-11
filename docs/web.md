# The web page

The game plays in a browser: built with Emscripten on N64Recomp-browser's `frontend/` into `Turok3Recompiled.html`
(the launcher, the recomp's menus, SDL2's audio and keyboard, the ROM from Load ROM, and the saves kept in IndexedDB;
`lib/N64Recomp-browser/frontend/CMakeLists.txt`'s header says what each part does), drawn through WebGPU. The page is
built from your ROM like the desktop program, so it is never built in CI and never published with the game in it.

What is the game's own stays here: the `EMSCRIPTEN` branches of `CMakeLists.txt` that make the game a page,
`tools/build_web.sh`, the prebuilt shader list (`src/rt64_shaders.txt`, recorded by `tools/shader_plan.json`; the
list's header says how to record it again) and the play plan (`tools/play_plan.json`). The rest is N64Recomp-browser's:
`docs/porting.md` (WebGPU and the shaders, prebuilt shader lists, Emscripten), `docs/playing.md` (playing and measuring
a page), `docs/forks.md` and `docs/findings.md`. Every play of this game's page is a line in N64Recomp-browser's
`plays.jsonl` (game `t3`), never a record here.

## The TLB-mapped code

The game's code runs from TLB-mapped addresses (0x00200400 and 0x00400000) and reads its strings, tables and constants
there, as do the audio banks its VM server maps from ROM (lib/turok-engine's `docs/rom_layout.md`). On a page RDRAM is
WebAssembly's linear memory, where those addresses would wrap into the heap, so `CMakeLists.txt` defines
`RECOMP_KUSEG_WINDOW` for every target: on the web the runtime puts KUSEG's first 16 MB in a window after RDRAM, and the
engine opens each range there with `recomp::make_kuseg_accessible` (natively it commits the reserved pages). Every TLB
section and VM region lies below 0x01000000: the VM server's two regions, the audio banks it maps at boot, run from
0x00800000 to 0x00B47000.

## Build it

On Linux with Docker, with the ROM at `roms/turok3.us.z64`:

```bash
tools/build_web.sh                  # [cpus] [cmake arguments]
```

It builds `build/web/Turok3Recompiled.html` with its `.js`, `.wasm` (about 13 MB) and `.data` (about 9 MB: the
recomp's fonts and menus, nothing of the game) in `tools/docker/Dockerfile.web`, the image every N64Recomp-browser
page is built in, with ccache in `build/ccache-web`. The script's header says each step. Two things differ from the
desktop build:

- The patches library (`patches/`, built for MIPS) is recompiled by the native N64Recomp that `tools/recompile.sh`
  builds, not the runtime's, which Emscripten would build for the browser.
- The generated code and the hooks in `us.toml` are compiled with implicit function declarations as errors, so a hook
  declares any native it calls.
- RT64 builds its pipelines from `src/rt64_shaders.txt` (`RT64_PREBUILT_SHADER_LIST`), in sections: the boot's before
  the game starts, and a level's the first time it draws.

Serve `build/web` over HTTP from localhost (any static server; the page brings its own `coi_serviceworker.js` for the
headers threads need) and open `Turok3Recompiled.html` in Chrome or Firefox with WebGPU, then Load ROM.

## The plan

`tools/play_plan.json` (about 2 minutes) reaches the first level the way a player does, by the keyboard's defaults
(Enter is Start, Space A, the arrows the C buttons, which walk, WASD the stick, which turns, Q Z, which fires): Start
Game, then Start held 400 ms every 5 s through the legal screen, the logos, the title, New Game, the difficulty, the
Rumble Pak notice and the opening scene to the character choice, Danielle and her scene. Its waits are the browser's
time, since nothing tells the plan where the game is, and Start is held rather than tapped because the game reads its
pad a few times a second while it loads. Then it walks, turns and fires in the Breach for 25 s. Play it with
N64Recomp-browser's `tools/play_page.mjs` (its `docs/playing.md`).

## The shader list

`src/rt64_shaders.txt` holds the boot's shaders (the launcher, the menus, the opening scene, the character choice, and
the pause menu with its Save Game screen, which any level shows) and a section for the Breach, built the first time it
draws. It is recorded by code with `tools/shader_plan.json` (the list's header says what each section plays and how to
record it again). A play that misses a shader prints it (`play_page.mjs` counts the misses); add it to the section of
the level it was missed in, or, for a screen any level shows, to the plan's last section, pause, which goes into the
boot's. The other levels have no section yet, so what they draw is missed: Chrome draws it with the ubershader until
it is built behind the game, Firefox builds it with the game waiting, and both remember it for the next visit
(N64Recomp-browser's `docs/porting.md`).

## How it plays

By the plan on a desktop GPU, uncapped at 3840x2000, Chrome presents 29 a second, the game's own 30 (RT64 draws no
frames between the game's for it yet), its longest frame gap 57 ms, with no shader waits, no misses and no WebGPU
errors. Firefox waits for each section as the game reaches it (3.3 s for the boot's, 1 s for the Breach's) and plays
with no misses or errors, at 30 a second in the menus and 15-25 in the Breach on a host busy with other work, its
sound short now and then. Its runs are in N64Recomp-browser's `plays.jsonl` (`t3`).

The game sets a VI mode of its own whose origin is the framebuffer's start, not a row into it as libultra's modes put
it. RT64 took the framebuffer shown to be a row before that, so no draw ended a game frame and the page showed four of
the game's frames as one, 7.5 a second, until N64Recomp-browser v0.184.0 (its `docs/forks.md`, rt64 d49a689).
