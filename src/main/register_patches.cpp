#include "turok3.h"
#include "../../RecompiledPatches/patches_bin.h"
#include "../../RecompiledPatches/recomp_overlays.inl"

#include "librecomp/overlays.hpp"
#include "librecomp/game.hpp"

// The patches (patches/, recompiled by N64Recomp patches.toml) over the game's functions they replace.
void t3::register_patches() {
    recomp::overlays::register_patches(t3_patches_bin, sizeof(t3_patches_bin), section_table, ARRLEN(section_table));
    recomp::overlays::register_base_exports(export_table);
    recomp::overlays::register_base_events(event_names);
    recomp::overlays::register_manual_patch_symbols(manual_patch_symbols);
}
