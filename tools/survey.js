// Turok 3's survey for the page's plans (N64Recomp-browser's tools/play_page.mjs and record_shaders.mjs load it with an
// evalFile step): where the game's boot is, so a plan presses Start only once it is past its first stage. There is no
// decomp: the address is the game's own, the values were read as the page played.
//
// The word at 0x80102E84 is the game's mode: 0 through the boot's first stage, then 1, and it only climbs as the game
// goes on through the logos, the title, the menus and the opening scene into the Breach (2, 3, 4, 7, 8, 10 to 13, 15,
// 16 on a regular play). The first stage lasts about a second and a half on a fast host; where the page is slow (a
// browser presenting through a virtual display) Turok 2's lasts 15-30 s, and Turok 3's can too.
//
// A Start held as the game finishes that stage opens its Controller Pak menu, as on the console: the mode stays 0, and
// the way out is B, which no plan presses. Both boots that held Start (Enter) from Start Game on for 40 s ended in
// the menu, at mode 0. The plans pressed Enter for 400 ms every 5 s from 8 s after Start Game, so a boot whose first
// stage outlasted 8 s could take a press there and press on at the menu to the end of the play (Turok 2's did, about
// one boot in thirty or fifty). A plan needs booted before its first press.
(() => {
  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
  const t3 = window.t3 = {
    // The mode, or null while RDRAM isn't there yet (n64r finds it by the boot globals the game writes first).
    mode: () => { try { return n64r(0x80102E84); } catch { return null; } },
    // Waits up to ms for the boot to leave its first stage (mode 0): true once it has. Start is safe to press from then on.
    booted: async (ms = 120000) => {
      for (const t0 = Date.now(); Date.now() - t0 < ms; await sleep(250)) {
        const m = t3.mode();
        if (m !== null && m !== 0) return true;
      }
      console.log(`[survey] still booting after ${ms} ms: ${t3.where()}`);
      return false;
    },
    // For a plan's log.
    where: () => `mode ${t3.mode()}`,
  };
  return 'survey: ' + t3.where();
})()
