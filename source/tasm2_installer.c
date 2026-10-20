/* Installer synchronization shim for Shadow Guardian.
 *
 * The upstream TASM2 port patched four hard-coded worker-array slots inside
 * libtasm2.so. Shadow Guardian (2010) predates that installer architecture;
 * the ArmHF PortMaster loader ran the .so without any such patch.  No-op
 * until libshadowguardian.so is reverse-engineered.
 */
#include "util.h"
#include <stdint.h>
#include <stddef.h>

void tasm2_installer_init(uint8_t *base, size_t size) {
  (void)base;
  (void)size;
  debugPrintf("[installer] Shadow Guardian installer shim: no-op\n");
}
