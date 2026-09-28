#include "drv_ds211.h"

#include <string.h>

#include "bsp_tick.h"

#define DS211_HEADER_0             0xEFU
#define DS211_HEADER_1             0x01U
#define DS211_BOOT_BYTE            0x55U
#define DS211_PACKET_COMMAND       0x01U
#define DS211_PACKET_ACK           0x07U
#define DS211_HEADER_SIZE          9U
#define DS211_CHECKSUM_SIZE        2U
#define DS211_MAX_PARAM_SIZE       16U

static uint16_t ds211_read_u16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static uint32_t ds211_read_u32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) |
           (uint32_t)data[3];
}

static void ds211_write_u16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

static void ds211_write_u32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value >> 24);
    data[1] = (uint8_t)(value >> 16);
    data[2] = (uint8_t)(value >> 8);
    data[3] = (uint8_t)value;
}

static uint16_t ds211_checksum(const uint8_t *data, uint16_t size)
{
    uint16_t sum = 0U;
    uint16_t index;

    for (index = 0U; index < size; ++index)
    {
        sum = (uint16_t)(sum + data[index]);
    }

    return sum;
}

static void ds211_rx_byte(const bsp_uart_t *uart, uint8_t byte, void *user_data)
{
    ds211_t *device = (ds211_t *)user_data;
    uint16_t expected;

    (void)uart;
    if (device == 0)
    {
        return;
    }

    if (device->rx_size == 0U)
    {
        if (byte == DS211_BOOT_BYTE)
        {
            device->boot_handshake_seen = 1U;
            return;
        }
        if (byte == DS211_HEADER_0)
        {
            if (device->frame_ready != 0U)
            {
                device->frame_overflow = 1U;
                return;
            }
            device->rx_frame[0] = byte;
            device->rx_size = 1U;
        }
        return;
    }

    if (device->rx_size == 1U)
    {
        if (byte == DS211_HEADER_1)
        {
            device->rx_frame[1] = byte;
            device->rx_size = 2U;
        }
        else if (byte == DS211_HEADER_0)
        {
            device->rx_frame[0] = byte;
        }
        else
        {
            device->rx_size = 0U;
        }
        return;
    }

    if (device->rx_size >= DS211_FRAME_BUFFER_SIZE)
    {
        device->rx_size = 0U;
        device->rx_expected_size = 0U;
        device->frame_overflow = 1U;
        return;
    }

    device->rx_frame[device->rx_size] = byte;
    device->rx_size++;

    if (device->rx_size == DS211_HEADER_SIZE)
    {
        expected = (uint16_t)(DS211_HEADER_SIZE +
                              ds211_read_u16(&device->rx_frame[7]));
        if ((expected < 12U) || (expected > DS211_FRAME_BUFFER_SIZE))
        {
            device->rx_size = 0U;
            device->rx_expected_size = 0U;
            device->frame_overflow = 1U;
            return;
        }
        device->rx_expected_size = expected;
    }

    if ((device->rx_expected_size != 0U) &&
        (device->rx_size >= device->rx_expected_size))
    {
        uint16_t frame_size = device->rx_expected_size;
        uint16_t calculated = ds211_checksum(&device->rx_frame[6],
                                                 (uint16_t)(frame_size - 8U));
        uint16_t received = ds211_read_u16(&device->rx_frame[frame_size - 2U]);

        if (calculated == received)
        {
            device->frame_ready = 1U;
        }
        else
        {
            device->frame_overflow = 1U;
        }

        device->rx_size = 0U;
    }
}

static int ds211_send_command(ds211_t *device,
                                  uint8_t command,
                                  const uint8_t *params,
                                  uint16_t param_size)
{
    uint8_t packet[DS211_HEADER_SIZE + DS211_MAX_PARAM_SIZE];
    uint16_t packet_size;
    uint16_t payload_size;
    uint16_t checksum;

    if ((device == 0) || (device->initialized == 0U) ||
        (param_size > DS211_MAX_PARAM_SIZE) ||
        ((params == 0) && (param_size != 0U)))
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    /* 1 command byte + parameters + 2 checksum bytes. */
    payload_size = (uint16_t)(1U + param_size + DS211_CHECKSUM_SIZE);
    packet_size = (uint16_t)(DS211_HEADER_SIZE + payload_size);

    packet[0] = DS211_HEADER_0;
    packet[1] = DS211_HEADER_1;
    ds211_write_u32(&packet[2], device->cfg.device_address);
    packet[6] = DS211_PACKET_COMMAND;
    ds211_write_u16(&packet[7], payload_size);
    packet[9] = command;
    if (param_size != 0U)
    {
        memcpy(&packet[10], params, param_size);
    }

    checksum = ds211_checksum(&packet[6], (uint16_t)(packet_size - 8U));
    ds211_write_u16(&packet[packet_size - 2U], checksum);

    ds211_reset_receiver(device);
    if (bsp_uart_write(&device->cfg.uart, packet, packet_size) != (int)packet_size)
    {
        return DS211_STATUS_IO_ERROR;
    }

    return DS211_STATUS_OK;
}

static int ds211_wait_until(ds211_t *device, uint32_t deadline_ms)
{
    if (device == 0)
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    while (device->frame_ready == 0U)
    {
        if (device->frame_overflow != 0U)
        {
            return DS211_STATUS_BAD_FRAME;
        }
        if ((int32_t)(bsp_tick_get_ms() - deadline_ms) >= 0)
        {
            return DS211_STATUS_TIMEOUT;
        }
    }

    return DS211_STATUS_OK;
}

static int ds211_get_response(ds211_t *device,
                                  uint8_t *ack,
                                  uint8_t *payload,
                                  uint16_t payload_capacity,
                                  uint16_t *payload_size)
{
    uint16_t frame_size;
    uint16_t response_size;

    if ((device == 0) || (device->frame_ready == 0U))
    {
        return DS211_STATUS_NOT_INITIALIZED;
    }
    if (device->frame_overflow != 0U)
    {
        device->frame_ready = 0U;
        device->frame_overflow = 0U;
        return DS211_STATUS_FRAME_DROPPED;
    }

    frame_size = device->rx_expected_size;
    if ((frame_size < 12U) ||
        (device->rx_frame[0] != DS211_HEADER_0) ||
        (device->rx_frame[1] != DS211_HEADER_1) ||
        (device->rx_frame[6] != DS211_PACKET_ACK))
    {
        device->frame_ready = 0U;
        return DS211_STATUS_BAD_FRAME;
    }

    response_size = (uint16_t)(frame_size - 12U);
    if (ack != 0)
    {
        *ack = device->rx_frame[9];
    }
    if ((payload != 0) && (payload_size != 0))
    {
        if (response_size > payload_capacity)
        {
            response_size = payload_capacity;
        }
        if (response_size != 0U)
        {
            memcpy(payload, &device->rx_frame[10], response_size);
        }
    }
    if (payload_size != 0)
    {
        *payload_size = response_size;
    }

    device->frame_ready = 0U;
    return DS211_STATUS_OK;
}

static int ds211_execute(ds211_t *device,
                             uint8_t command,
                             const uint8_t *params,
                             uint16_t param_size,
                             uint8_t *payload,
                             uint16_t payload_capacity,
                             uint16_t *payload_size,
                             uint32_t timeout_ms)
{
    uint32_t deadline;
    uint8_t ack = DS211_ACK_OK;
    int status;

    if ((device == 0) || (device->initialized == 0U))
    {
        return DS211_STATUS_NOT_INITIALIZED;
    }

    status = ds211_send_command(device, command, params, param_size);
    if (status != DS211_STATUS_OK)
    {
        return status;
    }

    deadline = bsp_tick_get_ms() + timeout_ms;
    status = ds211_wait_until(device, deadline);
    if (status != DS211_STATUS_OK)
    {
        return status;
    }

    status = ds211_get_response(device, &ack, payload, payload_capacity, payload_size);
    if (status != DS211_STATUS_OK)
    {
        return status;
    }

    return (int)ack;
}

int ds211_init(ds211_t *device, const ds211_cfg_t *config)
{
    if ((device == 0) || (config == 0))
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    memset(device, 0, sizeof(*device));
    device->cfg = *config;
    if (device->cfg.device_address == 0U)
    {
        device->cfg.device_address = DS211_DEVICE_ADDRESS_DEFAULT;
    }

    if (device->cfg.has_touch_gpio != 0U)
    {
        bsp_gpio_config_input(&device->cfg.touch_gpio);
    }

    device->initialized = 1U;
    if (bsp_uart_register_rx_callback(&device->cfg.uart,
                                      ds211_rx_byte,
                                      device) != 0)
    {
        device->initialized = 0U;
        return DS211_STATUS_IO_ERROR;
    }

    return DS211_STATUS_OK;
}

void ds211_reset_receiver(ds211_t *device)
{
    if (device == 0)
    {
        return;
    }

    device->rx_size = 0U;
    device->rx_expected_size = 0U;
    device->frame_ready = 0U;
    device->frame_overflow = 0U;
}

int ds211_handshake(ds211_t *device, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_HANDSHAKE,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_check_sensor(ds211_t *device, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_CHECK_SENSOR,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_read_sys_params(ds211_t *device,
                              ds211_sys_params_t *params,
                              uint32_t timeout_ms)
{
    uint8_t payload[16];
    uint16_t payload_size = 0U;
    int ack;

    if (params == 0)
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    ack = ds211_execute(device, DS211_CMD_READ_SYS_PARAMS,
                            0, 0U,
                            payload, sizeof(payload), &payload_size, timeout_ms);
    if ((ack == DS211_ACK_OK) && (payload_size >= sizeof(payload)))
    {
        params->status = ds211_read_u16(&payload[0]);
        params->sensor_type = ds211_read_u16(&payload[2]);
        params->library_size = ds211_read_u16(&payload[4]);
        params->score_level = ds211_read_u16(&payload[6]);
        params->device_address = ds211_read_u32(&payload[8]);
        params->packet_size_code = ds211_read_u16(&payload[12]);
        params->baud_rate_factor = ds211_read_u16(&payload[14]);
    }

    return ack;
}

int ds211_get_valid_template_count(ds211_t *device,
                                       uint16_t *count,
                                       uint32_t timeout_ms)
{
    uint8_t payload[2];
    uint16_t payload_size = 0U;
    int ack;

    if (count == 0)
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    ack = ds211_execute(device, DS211_CMD_VALID_TEMPLATE_COUNT,
                            0, 0U,
                            payload, sizeof(payload), &payload_size, timeout_ms);
    if ((ack == DS211_ACK_OK) && (payload_size >= sizeof(payload)))
    {
        *count = ds211_read_u16(payload);
    }

    return ack;
}

int ds211_capture_verify_image(ds211_t *device, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_GET_IMAGE,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_capture_enroll_image(ds211_t *device, uint32_t timeout_ms)
{
    const uint8_t command = 0x29U;

    return ds211_execute(device, command,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_generate_char(ds211_t *device, uint8_t buffer_id, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_GENERATE_CHAR,
                             &buffer_id, 1U, 0, 0U, 0, timeout_ms);
}

int ds211_generate_template(ds211_t *device, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_GENERATE_TEMPLATE,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_store_template(ds211_t *device,
                             uint8_t buffer_id,
                             uint16_t page_id,
                             uint32_t timeout_ms)
{
    uint8_t params[3];

    params[0] = buffer_id;
    ds211_write_u16(&params[1], page_id);
    return ds211_execute(device, DS211_CMD_STORE_TEMPLATE,
                             params, sizeof(params), 0, 0U, 0, timeout_ms);
}

int ds211_search_finger(ds211_t *device,
                            uint8_t buffer_id,
                            uint16_t start_page,
                            uint16_t page_count,
                            ds211_search_result_t *result,
                            uint32_t timeout_ms)
{
    uint8_t params[5];
    uint8_t payload[4];
    uint16_t payload_size = 0U;
    int ack;

    if (result == 0)
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    result->page_id = 0U;
    result->score = 0U;
    params[0] = buffer_id;
    ds211_write_u16(&params[1], start_page);
    ds211_write_u16(&params[3], page_count);

    ack = ds211_execute(device, DS211_CMD_SEARCH,
                            params, sizeof(params),
                            payload, sizeof(payload), &payload_size, timeout_ms);
    if ((ack == DS211_ACK_OK) && (payload_size >= sizeof(payload)))
    {
        result->page_id = ds211_read_u16(&payload[0]);
        result->score = ds211_read_u16(&payload[2]);
    }

    return ack;
}

int ds211_delete_template(ds211_t *device,
                              uint16_t page_id,
                              uint16_t count,
                              uint32_t timeout_ms)
{
    uint8_t params[4];

    ds211_write_u16(&params[0], page_id);
    ds211_write_u16(&params[2], count);
    return ds211_execute(device, DS211_CMD_DELETE_TEMPLATE,
                             params, sizeof(params), 0, 0U, 0, timeout_ms);
}

int ds211_clear_library(ds211_t *device, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_EMPTY_LIBRARY,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_auto_enroll(ds211_t *device,
                          uint16_t page_id,
                          uint8_t enroll_times,
                          uint16_t flags,
                          ds211_enroll_progress_t progress,
                          void *user_data,
                          uint32_t timeout_ms)
{
    uint8_t params[5];
    uint8_t payload[3];
    uint16_t payload_size;
    uint32_t deadline;
    uint8_t ack = DS211_ACK_OK;
    uint8_t stage;
    int status;

    if ((device == 0) || (device->initialized == 0U) ||
        (enroll_times == 0U))
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    ds211_write_u16(&params[0], page_id);
    params[2] = enroll_times;
    ds211_write_u16(&params[3], flags);

    status = ds211_send_command(device, DS211_CMD_AUTO_ENROLL,
                                    params, sizeof(params));
    if (status != DS211_STATUS_OK)
    {
        return status;
    }

    deadline = bsp_tick_get_ms() + timeout_ms;
    for (;;)
    {
        status = ds211_wait_until(device, deadline);
        if (status != DS211_STATUS_OK)
        {
            return status;
        }

        payload_size = 0U;
        status = ds211_get_response(device, &ack,
                                        payload, sizeof(payload), &payload_size);
        if (status != DS211_STATUS_OK)
        {
            return status;
        }
        if (payload_size < 2U)
        {
            return DS211_STATUS_BAD_FRAME;
        }

        stage = payload[0];
        if (progress != 0)
        {
            progress(stage, payload[1], user_data);
        }

        if ((stage == DS211_PROGRESS_STORE_TEMPLATE) ||
            ((stage == 0U) && (ack != DS211_ACK_OK)) ||
            (ack == DS211_ACK_TIMEOUT) ||
            ((stage >= DS211_PROGRESS_CHECK_DUPLICATE) &&
             (ack != DS211_ACK_OK)))
        {
            return (int)ack;
        }
    }
}

int ds211_auto_identify(ds211_t *device,
                            uint8_t score_level,
                            uint16_t page_id,
                            uint16_t flags,
                            ds211_search_result_t *result,
                            uint32_t timeout_ms)
{
    uint8_t params[5];
    uint8_t payload[6];
    uint16_t payload_size;
    uint32_t deadline;
    uint8_t ack = DS211_ACK_OK;
    uint8_t stage;
    int status;

    if ((device == 0) || (device->initialized == 0U) || (result == 0))
    {
        return DS211_STATUS_INVALID_PARAM;
    }

    result->page_id = 0U;
    result->score = 0U;
    params[0] = score_level;
    ds211_write_u16(&params[1], page_id);
    ds211_write_u16(&params[3], flags);

    status = ds211_send_command(device, DS211_CMD_AUTO_IDENTIFY,
                                    params, sizeof(params));
    if (status != DS211_STATUS_OK)
    {
        return status;
    }

    deadline = bsp_tick_get_ms() + timeout_ms;
    for (;;)
    {
        status = ds211_wait_until(device, deadline);
        if (status != DS211_STATUS_OK)
        {
            return status;
        }

        payload_size = 0U;
        status = ds211_get_response(device, &ack,
                                        payload, sizeof(payload), &payload_size);
        if (status != DS211_STATUS_OK)
        {
            return status;
        }
        if (payload_size < 5U)
        {
            return DS211_STATUS_BAD_FRAME;
        }

        stage = payload[0];
        result->page_id = ds211_read_u16(&payload[1]);
        result->score = ds211_read_u16(&payload[3]);

        if ((stage == 5U) ||
            ((stage == 0U) && (ack != DS211_ACK_OK)))
        {
            return (int)ack;
        }
    }
}

int ds211_cancel(ds211_t *device, uint32_t timeout_ms)
{
    return ds211_execute(device, DS211_CMD_CANCEL,
                             0, 0U, 0, 0U, 0, timeout_ms);
}

int ds211_touch_is_active(const ds211_t *device)
{
    if ((device == 0) || (device->cfg.has_touch_gpio == 0U))
    {
        return -1;
    }

    if (device->cfg.touch_active_high != 0U)
    {
        return (bsp_gpio_read(&device->cfg.touch_gpio) == BSP_GPIO_LEVEL_HIGH) ? 1 : 0;
    }

    return (bsp_gpio_read(&device->cfg.touch_gpio) == BSP_GPIO_LEVEL_LOW) ? 1 : 0;
}

const char *ds211_ack_string(int ack)
{
    switch ((uint8_t)ack)
    {
    case DS211_ACK_OK:
        return "OK";
    case DS211_ACK_RX_ERROR:
        return "receive error";
    case DS211_ACK_NO_FINGER:
        return "no finger";
    case DS211_ACK_IMAGE_FAIL:
        return "image capture failed";
    case DS211_ACK_IMAGE_TOO_DRY:
        return "finger too dry";
    case DS211_ACK_IMAGE_TOO_WET:
        return "finger too wet";
    case DS211_ACK_IMAGE_DISORDER:
        return "image disorder";
    case DS211_ACK_FEATURE_TOO_SMALL:
        return "feature too small";
    case DS211_ACK_NOT_MATCH:
        return "fingerprint does not match";
    case DS211_ACK_NOT_SEARCHED:
        return "fingerprint not found";
    case DS211_ACK_MERGE_FAIL:
        return "template merge failed";
    case DS211_ACK_ADDRESS_OVER:
        return "page id out of range";
    case DS211_ACK_DELETE_FAIL:
        return "delete failed";
    case DS211_ACK_EMPTY_FAIL:
        return "library clear failed";
    case DS211_ACK_SLEEP_FAIL:
        return "sleep failed";
    case DS211_ACK_FLASH_ERROR:
        return "flash error";
    case DS211_ACK_LIBRARY_FULL:
        return "fingerprint library full";
    case DS211_ACK_TEMPLATE_NOT_EMPTY:
        return "page id already contains a template";
    case DS211_ACK_LIBRARY_EMPTY:
        return "fingerprint library is empty";
    case DS211_ACK_TIMEOUT:
        return "module operation timeout";
    case DS211_ACK_FINGER_EXISTS:
        return "fingerprint already exists";
    case DS211_ACK_SENSOR_FAIL:
        return "sensor initialization failed";
    default:
        return "unknown module status";
    }
}
