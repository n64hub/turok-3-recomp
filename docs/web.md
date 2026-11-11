# The web page

The game plays in a browser: built with Emscripten on N64Recomp-browser's `frontend/` into `Turok3Recompiled.html`
(the launcher, the recomp's menus, SDL2's audio and keyboard, the ROM from Load ROM, and the saves kept in IndexedDB;
`lib/N64Recomp-browser/frontend/CMakeLists.txt`'s header says what each part does), drawn through WebGPU. The page is
built from your ROM like the desktop program, so it is never built in CI and never published with the game in it.

What is the game's own stays here: the `EMSCRIPTEN` branches of `CMakeLists.txt` that make the game a page and
`tools/build_web.sh`. The rest is N64Recomp-browser's: `docs/porting.md` (WebGPU and the shaders, prebuilt shader
lists, Emscripten), `docs/playing.md` (playing and measuring a page), `docs/forks.md` and `docs/findings.md`. Every play
of this game's page is a line in N64Recomp-browser's `plays.jsonl`, never a record here.

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

Serve `build/web` over HTTP from localhost (any static server; the page brings its own `coi_serviceworker.js` for the
headers threads need) and open `Turok3Recompiled.html` in Chrome or Firefox with WebGPU, then Load ROM.

## How it plays

TO FILL
