// The game's own widescreen, beside what the engine does for every Turok game (lib/turok-engine/docs/widescreen.md):
// the menus' swirling backdrop.
//
// The boot screens, the title and the menus draw a blue swirl behind them, a turning 3D mesh with an eight-sided edge,
// sized to cover the game's 4:3 view with some to spare. RT64 widens the 3D to the window by itself, and up to 16:9
// the mesh still covers it; in a wider window its edge shows, the corners black and, at 21:9, strips down the sides.
// The menu code (func_00414FD0, in us.toml) builds the mesh's float matrix and converts it for the display list; before
// that, the matrix's view x and y are scaled by the window's aspect over 16:9, so a wider window sees the swirl as a
// 16:9 one does, a little closer. Nothing changes at 16:9 and narrower, or outside Expand.
#include <cstdint>
#include <cstring>

#include "recomp.h"
#include "recompui/recompui.h"
#include "turok_engine.h"

extern "C" void t3_widen_menu_backdrop(uint8_t* rdram, uint32_t matrix) {
    if (!turok::widescreen_active() || matrix == 0) {
        return;
    }
    int width = 0, height = 0;
    recompui::get_window_size(width, height);
    if (width <= 0 || height <= 0) {
        return;
    }
    float scale = ((float)width / (float)height) / (16.0f / 9.0f);
    if (scale <= 1.0f) {
        return;
    }
    // A row vector times the matrix: its columns 0 and 1 make the view's x and y.
    for (uint32_t row = 0; row < 4; row++) {
        for (uint32_t col = 0; col < 2; col++) {
            uint32_t a = matrix + row * 16 + col * 4;
            uint32_t bits = (uint32_t)MEM_W(0, (gpr)(int32_t)a);
            float f;
            std::memcpy(&f, &bits, sizeof(f));
            f *= scale;
            std::memcpy(&bits, &f, sizeof(f));
            MEM_W(0, (gpr)(int32_t)a) = (int32_t)bits;
        }
    }
}
