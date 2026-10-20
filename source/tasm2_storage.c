/* Shadow Guardian setup/storage bridge. No OBB expansion; loose gameloft data. */
#include "config.h"
#include "dcr_manifest.h"
#include "dcr_path.h"
#include "dcr_prefs.h"
#include "dcr_setup.h"
#include "error.h"
#include "tasm2.h"
#include "util.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/stat.h>
static char obb_path[3][768], obb_name[3][160];
static mz_zip_archive apk;
static Mutex apk_lock;
static int apk_open;
int tasm2_signature_hash(void);
static const char *const libs[] = {"libshadowguardian.so"};
const RtSetupPlan port_setup_plan = {
    .libs = libs, .nlibs = 1, .libs_p0 = 0, .libs_p1 = 800, .classes_p0 = 800, .classes_p1 = 990};
const char *port_apk_help(void) {
  return "Copy your own ARMv7 Shadow Guardian 1.0.1 APK and the gameloft/games/GloftSGHP "
         "data folder to sd:/switch/shadowguardian_nx/. See README.md.";
}
/* Shadow Guardian (2010) does not use Android OBB expansion files. Game data
 * lives as loose files under sdcard/gameloft/games/GloftSGHP/. */
static void find_obbs(void) {
  obb_path[0][0] = obb_path[1][0] = obb_path[2][0] = '\0';
  obb_name[0][0] = obb_name[1][0] = obb_name[2][0] = '\0';
  DcrPrefsEdit *e = dcr_prefs_edit(dcr_prefs_open("GamePrefs"));
  dcr_prefs_put(e, 's', "SDFolder", A_RESOURCE);
  dcr_prefs_commit(e);
  dcr_prefs_edit_free(e);
  debugPrintf("[setup] Shadow Guardian: no OBB expansion; using loose gameloft data\n");
}
void tasm2_storage_init(const char *path) {
  debugPrintf("[setup] APK version %s (code %d)\n",
             dcr_manifest_version_name(), dcr_manifest_version_code());
  find_obbs();
  dcr_setup_from_apk(path);
  for (unsigned i = 0; i < 1; i++) {
    char library[768];
    rt_root_path(library, sizeof library, libs[i]);
    FILE *f = fopen(library, "rb");
    if (!f)
      fatal_error("Cannot read %s. Extract it from the APK first.", libs[i]);
    fclose(f);
    debugPrintf("[setup] %s present\n", libs[i]);
  }
  memset(&apk, 0, sizeof apk);
  if (!mz_zip_reader_init_file(&apk, path, 0))
    fatal_error("Cannot read the APK's assets.");
  apk_open = 1;
  tasm2_signature_hash();
}
const char *tasm2_storage_obb_path(int i) {
  return i >= 0 && i < 3 ? obb_path[i] : NULL;
}
const char *port_path_fixup(const char *real, char *out, size_t cap) {
  const char *base = strrchr(real, '/');
  base = base ? base + 1 : real;
  for (int i = 0; i < 3; i++)
    if (*obb_name[i] && !strcmp(base, obb_name[i])) {
      snprintf(out, cap, "%s", obb_path[i]);
      return out;
    }
  return real;
}
uint8_t *tasm2_catalog_read(size_t *);
uint8_t *tasm2_apk_asset(const char *name, size_t *len) {
  if (len)
    *len = 0;
  char entry[768];
  while (*name == '/' || *name == '.')
    name++;
  snprintf(entry, sizeof entry, "%s%s", strncmp(name, "assets/", 7) ? "assets/" : "", name);
  mutexLock(&apk_lock);
  if (!apk_open) {
    mutexUnlock(&apk_lock);
    return NULL;
  }
  size_t n = 0;
  void *p = mz_zip_reader_extract_file_to_heap(&apk, entry, &n, 0);
  mutexUnlock(&apk_lock);
  if (!p && !strcmp(entry, "assets/IapLocalData_Google.json"))
    return tasm2_catalog_read(len);
  if (len)
    *len = n;
  return p;
}
/* Read the actual signing certificate from PKCS#7 rather than inventing a signature. */
static int der(const uint8_t *p, size_t n, unsigned *tag, size_t *hdr, size_t *len) {
  if (n < 2)
    return 0;
  *tag = p[0];
  size_t h = 2, v = p[1];
  if (v & 128) {
    unsigned k = v & 127;
    if (!k || k > 4 || n < 2 + k)
      return 0;
    v = 0;
    for (unsigned i = 0; i < k; i++)
      v = (v << 8) | p[2 + i];
    h += k;
  }
  if (v > n - h)
    return 0;
  *hdr = h;
  *len = v;
  return 1;
}
int tasm2_signature_hash(void) {
  static int ready, hash;
  if (ready)
    return hash;
  mutexLock(&apk_lock);
  unsigned count = mz_zip_reader_get_num_files(&apk);
  size_t size = 0;
  uint8_t *pkcs = NULL;
  for (unsigned i = 0; i < count; i++) {
    mz_zip_archive_file_stat st;
    if (!mz_zip_reader_file_stat(&apk, i, &st))
      continue;
    size_t l = strlen(st.m_filename);
    if (!strncmp(st.m_filename, "META-INF/", 9) && l > 4 &&
        !strcmp(st.m_filename + l - 4, ".RSA")) {
      pkcs = mz_zip_reader_extract_to_heap(&apk, i, &size, 0);
      break;
    }
  }
  mutexUnlock(&apk_lock);
  if (pkcs) {
    unsigned tag;
    size_t h, n;
    const uint8_t *v = pkcs;
    size_t left = size;
    /* ContentInfo SEQUENCE, OID then explicit signedData [0], then its SEQUENCE. */
    if (der(v, left, &tag, &h, &n) && tag == 0x30) {
      v += h;
      left = n;
      if (der(v, left, &tag, &h, &n)) {
        v += h + n;
        left -= h + n;
        if (der(v, left, &tag, &h, &n) && tag == 0xa0) {
          v += h;
          left = n;
          if (der(v, left, &tag, &h, &n) && tag == 0x30) {
            v += h;
            left = n;
            /* version INTEGER, digestAlgorithms SET, encapContentInfo SEQUENCE. */
            int ok = 1;
            for (int i = 0; i < 3; i++) {
              if (!der(v, left, &tag, &h, &n)) {
                ok = 0;
                break;
              }
              v += h + n;
              left -= h + n;
            }
            if (ok && der(v, left, &tag, &h, &n) && tag == 0xa0) {
              v += h;
              left = n;
              if (der(v, left, &tag, &h, &n) && tag == 0x30) {
                uint32_t result = 1;
                for (size_t i = 0; i < h + n; i++)
                  result = result * 31 + (int8_t)v[i];
                hash = (int32_t)result;
                ready = 1;
              }
            }
          }
        }
      }
    }
    free(pkcs);
  }
  if (!ready)
    fatal_error("Could not read the APK signing certificate. Use your original, signed APK.");
  debugPrintf("[setup] APK certificate hashCode %d\n", hash);
  return hash;
}
void tasm2_storage_close(void) {
  mutexLock(&apk_lock);
  if (apk_open)
    mz_zip_reader_end(&apk);
  apk_open = 0;
  mutexUnlock(&apk_lock);
} /* Catalog is a small plain-text member inside this verified main OBB. */
uint8_t *tasm2_catalog_read(size_t *len) {
  FILE *f = fopen(obb_path[0], "rb");
  if (!f)
    return NULL;
  uint8_t *data = malloc(9578);
  if (!data) {
    fclose(f);
    return NULL;
  }
  int ok = fseek(f, 312677714L, SEEK_SET) == 0 && fread(data, 1, 9577, f) == 9577;
  fclose(f);
  if (!ok || mz_crc32(0, data, 9577) != 0x036b8ff1u) {
    free(data);
    debugPrintf("[setup] expansion's original catalog failed CRC; storefront unavailable\n");
    return NULL;
  }
  data[9577] = 0;
  if (len)
    *len = 9577;
  return data;
}
