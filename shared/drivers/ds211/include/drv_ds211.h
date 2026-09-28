#ifndef DRV_DS211_H
#define DRV_DS211_H

#include <stdint.h>

#include "bsp_gpio.h"
#include "bsp_uart.h"

#define DS211_DEVICE_ADDRESS_DEFAULT 0xFFFFFFFFUL
#define DS211_DEFAULT_BAUD_RATE      115200UL
#define DS211_FRAME_BUFFER_SIZE      128U

typedef enum
{
    DS211_STATUS_OK = 0,
    DS211_STATUS_INVALID_PARAM = -1,
    DS211_STATUS_IO_ERROR = -2,
    DS211_STATUS_TIMEOUT = -3,
    DS211_STATUS_BAD_FRAME = -4,
    DS211_STATUS_NOT_INITIALIZED = -5,
    DS211_STATUS_FRAME_DROPPED = -6
} ds211_status_t;

typedef enum
{
    DS211_CMD_GET_IMAGE = 0x01,
    DS211_CMD_GENERATE_CHAR = 0x02,
    DS211_CMD_MATCH = 0x03,
    DS211_CMD_SEARCH = 0x04,
    DS211_CMD_GENERATE_TEMPLATE = 0x05,
    DS211_CMD_STORE_TEMPLATE = 0x06,
    DS211_CMD_DELETE_TEMPLATE = 0x0C,
    DS211_CMD_EMPTY_LIBRARY = 0x0D,
    DS211_CMD_READ_SYS_PARAMS = 0x0F,
    DS211_CMD_VALID_TEMPLATE_COUNT = 0x1D,
    DS211_CMD_CANCEL = 0x30,
    DS211_CMD_AUTO_ENROLL = 0x31,
    DS211_CMD_AUTO_IDENTIFY = 0x32,
    DS211_CMD_SLEEP = 0x33,
    DS211_CMD_HANDSHAKE = 0x35,
    DS211_CMD_CHECK_SENSOR = 0x36
} ds211_command_t;

typedef enum
{
    DS211_ACK_OK = 0x00,
    DS211_ACK_RX_ERROR = 0x01,
    DS211_ACK_NO_FINGER = 0x02,
    DS211_ACK_IMAGE_FAIL = 0x03,
    DS211_ACK_IMAGE_TOO_DRY = 0x04,
    DS211_ACK_IMAGE_TOO_WET = 0x05,
    DS211_ACK_IMAGE_DISORDER = 0x06,
    DS211_ACK_FEATURE_TOO_SMALL = 0x07,
    DS211_ACK_NOT_MATCH = 0x08,
    DS211_ACK_NOT_SEARCHED = 0x09,
    DS211_ACK_MERGE_FAIL = 0x0A,
    DS211_ACK_ADDRESS_OVER = 0x0B,
    DS211_ACK_DELETE_FAIL = 0x10,
    DS211_ACK_EMPTY_FAIL = 0x11,
    DS211_ACK_SLEEP_FAIL = 0x12,
    DS211_ACK_FLASH_ERROR = 0x18,
    DS211_ACK_LIBRARY_FULL = 0x1F,
    DS211_ACK_TEMPLATE_NOT_EMPTY = 0x22,
    DS211_ACK_LIBRARY_EMPTY = 0x24,
    DS211_ACK_TIMEOUT = 0x26,
    DS211_ACK_FINGER_EXISTS = 0x27,
    DS211_ACK_SENSOR_FAIL = 0x29
} ds211_ack_t;

typedef enum
{
    DS211_PROGRESS_CAPTURE_IMAGE = 1,
    DS211_PROGRESS_GENERATE_FEATURE = 2,
    DS211_PROGRESS_FINGER_LEAVE = 3,
    DS211_PROGRESS_MERGE_TEMPLATE = 4,
    DS211_PROGRESS_CHECK_DUPLICATE = 5,
    DS211_PROGRESS_STORE_TEMPLATE = 6
} ds211_enroll_stage_t;

typedef struct
{
    bsp_uart_t uart;
    bsp_gpio_t touch_gpio;
    uint8_t has_touch_gpio;
    uint8_t touch_active_high;
    uint32_t device_address;
} ds211_cfg_t;

typedef struct
{
    uint16_t status;
    uint16_t sensor_type;
    uint16_t library_size;
    uint16_t score_level;
    uint32_t device_address;
    uint16_t packet_size_code;
    uint16_t baud_rate_factor;
} ds211_sys_params_t;

typedef struct
{
    uint16_t page_id;
    uint16_t score;
} ds211_search_result_t;

typedef struct
{
    ds211_cfg_t cfg;
    uint8_t rx_frame[DS211_FRAME_BUFFER_SIZE];
    volatile uint16_t rx_size;
    volatile uint16_t rx_expected_size;
    volatile uint8_t frame_ready;
    volatile uint8_t frame_overflow;
    volatile uint8_t boot_handshake_seen;
    uint8_t initialized;
} ds211_t;

typedef void (*ds211_enroll_progress_t)(uint8_t stage, uint8_t detail, void *user_data);

int ds211_init(ds211_t *device, const ds211_cfg_t *config);
void ds211_reset_receiver(ds211_t *device);

int ds211_handshake(ds211_t *device, uint32_t timeout_ms);
int ds211_check_sensor(ds211_t *device, uint32_t timeout_ms);
int ds211_read_sys_params(ds211_t *device,
                          ds211_sys_params_t *params,
                          uint32_t timeout_ms);
int ds211_get_valid_template_count(ds211_t *device,
                                   uint16_t *count,
                                   uint32_t timeout_ms);

int ds211_capture_verify_image(ds211_t *device, uint32_t timeout_ms);
int ds211_capture_enroll_image(ds211_t *device, uint32_t timeout_ms);
int ds211_generate_char(ds211_t *device, uint8_t buffer_id, uint32_t timeout_ms);
int ds211_generate_template(ds211_t *device, uint32_t timeout_ms);
int ds211_store_template(ds211_t *device,
                         uint8_t buffer_id,
                         uint16_t page_id,
                         uint32_t timeout_ms);
int ds211_search_finger(ds211_t *device,
                        uint8_t buffer_id,
                        uint16_t start_page,
                        uint16_t page_count,
                        ds211_search_result_t *result,
                        uint32_t timeout_ms);
int ds211_delete_template(ds211_t *device,
                          uint16_t page_id,
                          uint16_t count,
                          uint32_t timeout_ms);
int ds211_clear_library(ds211_t *device, uint32_t timeout_ms);

int ds211_auto_enroll(ds211_t *device,
                      uint16_t page_id,
                      uint8_t enroll_times,
                      uint16_t flags,
                      ds211_enroll_progress_t progress,
                      void *user_data,
                      uint32_t timeout_ms);
int ds211_auto_identify(ds211_t *device,
                        uint8_t score_level,
                        uint16_t page_id,
                        uint16_t flags,
                        ds211_search_result_t *result,
                        uint32_t timeout_ms);
int ds211_cancel(ds211_t *device, uint32_t timeout_ms);

int ds211_touch_is_active(const ds211_t *device);
const char *ds211_ack_string(int ack);

#endif
