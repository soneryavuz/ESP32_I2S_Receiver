#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "esp_check.h"

#define SAMPLE_RATE     44100
#define I2S_DMA_BUF_LEN 1024

#define I2S_BCLK_IO     26
#define I2S_WS_IO       25
#define I2S_DATA_IN_IO  22

static const char *TAG = "I2S_RX_NEW";

void app_main(void)
{
    esp_err_t ret;

    // Configuration channel 
    i2s_chan_config_t chan_cfg = {
        .id = 0,
        .role = I2S_ROLE_SLAVE,
        .dma_desc_num = 4,
        .dma_frame_num = I2S_DMA_BUF_LEN,
        .auto_clear = true
    };

    i2s_chan_handle_t rx_handle;
    ret = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    ESP_ERROR_CHECK(ret);

    // Configuration standart I2S
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_RIGHT | I2S_STD_SLOT_LEFT,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA_IN_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        }
    };

    // Aply configured channel
    ret = i2s_channel_init_std_mode(rx_handle, &std_cfg);
    ESP_ERROR_CHECK(ret);

    // Enable I2S channel
    ret = i2s_channel_enable(rx_handle);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Started I2S RX");

    int16_t buffer[I2S_DMA_BUF_LEN];
    size_t bytes_read = 0;

    while (1) {
        ret = i2s_channel_read(rx_handle, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
        if (ret == ESP_OK && bytes_read > 0) {
            ESP_LOGI(TAG, "Read %d bytes", bytes_read);
            for (int i = 0; i < 10; i++) {
                printf("%d ", buffer[i]);
            }
            printf("\n");
        }
    }
}
