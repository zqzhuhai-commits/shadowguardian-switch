/* Runtime patches for Shadow Guardian NX.
 *
 * The upstream TASM2 port applied binary-level patches at hard-coded offsets
 * inside libtasm2.so. Shadow Guardian uses a different ELF layout and those
 * offsets are not known yet. All binary patches are skipped; only runtime
 * popup polling is wired up.
 */
#include "util.h"
#include "tasm2.h"
#include "tasm2_popups.h"
#include <stdint.h>
#include <stddef.h>

void tasm2_fixes_prepare(so_module *m) {
  (void)m;
  debugPrintf("[fixes] Shadow Guardian: binary patches disabled (offsets not yet known)\n");
  tasm2_popups_init();
}

void tasm2_fixes_tick(void) {
  tasm2_popups_tick();
}
