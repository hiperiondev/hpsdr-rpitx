/*
 Copyright ????
 */

#include <stdint.h>
#include <cstdio>

#include "util.hpp"

static unsigned get_dt_ranges(const char *filename, unsigned offset) {
    dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    unsigned address = ~0;
    FILE *fp = fopen(filename, "rb");
    if (fp) {
        unsigned char buf[4];
        fseek(fp, offset, SEEK_SET);
        if (fread(buf, 1, sizeof buf, fp) == sizeof buf)
            address = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
        fclose(fp);
    }
    dbg_printf(2, "< func: %s |\n", __func__);
    return address;
}

unsigned bcm_host_get_peripheral_address(void) {
    dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    unsigned address = get_dt_ranges("/proc/device-tree/soc/ranges", 4);
    if (address == 0)
        address = get_dt_ranges("/proc/device-tree/soc/ranges", 8);

    dbg_printf(2, "< func: %s |\n", __func__);
    return address == ~0u ? 0x20000000 : address;
}

unsigned bcm_host_get_sdram_address(void) {
    dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    unsigned address = get_dt_ranges("/proc/device-tree/axi/vc_mem/reg", 8);

    dbg_printf(2, "< func: %s |\n", __func__);
    return address == ~0u ? 0x40000000 : address;
}
