
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"

#include "tinyusb.h"
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include "diskio_wl.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "driver/gpio.h"
#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_types.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "driver/uart.h"
#include "sdmmc_cmd.h"
#include "assert.h"

static sdmmc_card_t* mount_card = NULL;
static const char *TAG = "usb_demo";

#ifdef CONFIG_USE_EXTERNAL_SDCARD
#define BOARD_SDCARD_MOSI_PIN CONFIG_SDCARD_MOSI_PIN
#define BOARD_SDCARD_MISO_PIN CONFIG_SDCARD_MISO_PIN
#define BOARD_SDCARD_SCLK_PIN CONFIG_SDCARD_SCLK_PIN
#define BOARD_SDCARD_CS_PIN CONFIG_SDCARD_CS_PIN
#endif

// Mount path for the partition
const char *base_path = "/extflash";
/* Function to initialize SPIFFS */
static esp_err_t init_fat(sdmmc_card_t **card_handle)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    esp_err_t err = ESP_FAIL;
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
#ifdef CONFIG_USE_INTERNAL_FLASH
// Handle of the wear levelling library instance
    wl_handle_t s_wl_handle_1 = WL_INVALID_HANDLE;
    BYTE pdrv_msc = 0xFF;
    ESP_LOGI(TAG, "using internal flash");
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 9,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    err = esp_vfs_fat_spiflash_mount(base_path, "vfs", &mount_config, &s_wl_handle_1);
    ESP_LOGE(TAG, "esp_vfs_fat_spiflash_mount (%s), size(%d)", esp_err_to_name(err), CONFIG_WL_SECTOR_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return ESP_FAIL;
    }

    pdrv_msc = ff_diskio_get_pdrv_wl(s_wl_handle_1);
    ESP_LOGI(TAG, "pdrv_msc = %d !!", pdrv_msc);
#elif defined CONFIG_USE_EXTERNAL_SDCARD
    ESP_LOGI(TAG, "using external sdcard");
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 64*512
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    //host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = BOARD_SDCARD_MOSI_PIN,
        .miso_io_num = BOARD_SDCARD_MISO_PIN,
        .sclk_io_num = BOARD_SDCARD_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    err = spi_bus_initialize(host.slot, &bus_cfg, host.slot);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ESP_FAIL;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = BOARD_SDCARD_CS_PIN;
    slot_config.host_id = host.slot;

    sdmmc_card_t *card;
    err = esp_vfs_fat_sdspi_mount(base_path, &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return ESP_FAIL;
    }

    sdmmc_card_print_info(stdout, card);
    if(card_handle) *card_handle = card;
#endif

    return ESP_OK;
}

//--------------------------------------------------------------------+
// tinyusb callbacks
//--------------------------------------------------------------------+

extern void usb_msc_mount();
extern void usb_msc_umount();

// Invoked when device is mounted
void tud_mount_cb(void)
{
    usb_msc_mount();
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    usb_msc_umount();
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allows us to perform remote wakeup
// USB Specs: Within 7ms, device must draw an average current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    ESP_LOGW(__func__, "");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGW(__func__, "");
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void tud_msc_write10_complete_cb (uint8_t lun) {
    (void) lun;
    // This write is complete, start the autoreload clock.
    ESP_LOGD(__func__, "");
}

void app_main(void)
{
    /* Initialize file storage */
    ESP_ERROR_CHECK(init_fat(&mount_card));
    vTaskDelay(100 / portTICK_PERIOD_MS);

    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false // In the most cases you need to use a `false` value
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGE(TAG, "USB initialization DONE");

    while (1) { //TODO: monitor system if you want
        vTaskDelay(500 / portTICK_RATE_MS);
    }

}
