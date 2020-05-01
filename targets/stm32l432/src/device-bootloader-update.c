#include APP_CONFIG
#include "flash.h"
#include "log.h"
#include "memory_layout.h"
#include "nfc.h"
#include "crypto.h"
#include "device.h"
#include "device-bootloader-update.h"
#include <stdint.h>
#include "led.h"

#if !defined(IS_BOOTLOADER) && defined(APP_UPDATE_BOOTLOADER)
#include "boot_payload.h"

#warning "Bootloader payload added"

void erase_bootloader(void){
    int page;
    printf1(TAG_ERR,"Erasing bootloader\r\n");
    for(page = 0; page < APPLICATION_START_PAGE; page++)
    {
        printf1(TAG_ERR,"Erasing page: %d\r\n", page);
        flash_erase_page(page);
    }
}

void bootloader_calculate_hash(uint8_t *hash){
    timestamp();
    const uint16_t hashlen = APPLICATION_START_ADDR-BOOTLOADER_START_ADDR;
    crypto_sha256_init();
    crypto_sha256_update( (uint8_t*) BOOTLOADER_START_ADDR, hashlen);
    crypto_sha256_final(hash);
    printf1(TAG_TIME,"hash time flash: %d ms\n",timestamp());

    printf1(TAG_ERR, "hash: start: %p, len: %d\n", BOOTLOADER_START_ADDR, hashlen);
    dump_arrl(TAG_ERR, hash, 32);
}

void update_bootloader(void){
    if (device_is_nfc() == NFC_IS_ACTIVE)
    {
        printf1(TAG_ERR, "NFC is active. Skip bootloader update.\n");
        return;
    }
#if DEBUG_LEVEL >= 2
    uint8_t hash[32];
    bootloader_calculate_hash(hash);
#endif
    timestamp();
    bool success = memcmp((uint8_t*) BOOTLOADER_START_ADDR, boot_payload, boot_payload_length) == 0;
    printf1(TAG_TIME,"bootloader memcmp calc time: %d ms\n",timestamp());
    if (success) {
        printf1(TAG_GREEN, "Bootloader already up-to-date. Skipping.\n");
        return;
    }
    led_rgb(0xFF0000);
    timestamp();
    erase_bootloader();  //about 220 ms
    printf1(TAG_TIME,"boot erase: %d ms\n",timestamp());
    printf1(TAG_ERR, "Write: %p %d\n", boot_payload, boot_payload_length);
    flash_write(BOOTLOADER_START_ADDR,boot_payload, boot_payload_length); //about 225 ms
    printf1(TAG_TIME,"boot write: %d ms\n",timestamp());
    led_rgb(0x00FF00);

    success = memcmp((uint8_t*) BOOTLOADER_START_ADDR, boot_payload, boot_payload_length) == 0;
    printf1(TAG_TIME,"boot update finished, compare: %d ms\n",timestamp()); // about 30 ms
#if DEBUG_LEVEL >= 2
    bootloader_calculate_hash(hash);
#endif

    led_rgb(0x0000FF);
}
#endif
