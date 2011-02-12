#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

void flash_page (uint16_t page, uint8_t * buf)
{
cli();
#if SPM_PAGESIZE < 256
  uint8_t i;
#else
  uint16_t i;
#endif
  eeprom_busy_wait();

  boot_page_erase(page);
  boot_spm_busy_wait();

  for(i = 0; i < SPM_PAGESIZE; i += 2) {
    /* Set up little-endian word. */
    uint16_t w = buf[i];
    w += buf[i+1] << 8;
        
    boot_page_fill (page + i, w);
  }

  boot_page_write (page);
  boot_spm_busy_wait();

  /* Reenable RWW-section again. */
  boot_rww_enable ();
  sei();
}
