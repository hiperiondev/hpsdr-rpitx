/*
 Copyright ????
 */

#ifndef CORE_RPI_H_
#define CORE_RPI_H_

unsigned get_dt_ranges(const char *filename, unsigned offset);
unsigned bcm_host_get_peripheral_address(void);
unsigned bcm_host_get_sdram_address(void);

#endif
