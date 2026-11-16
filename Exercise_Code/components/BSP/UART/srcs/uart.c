#include "uart.h"

static const char *TAG = "uart";

static size_t len = 0;  // 缓存数据长度
static uint8_t data[BUFF_SIZE] = {0};

/**********************************************
 * @brief 串口回显处理函数
 **********************************************/
void uart_echo_process(void)
{
    // 获取缓存剩余数据长度
    uart_get_buffered_data_len(UART_NUM, &len);
    
    if(len>0)
    {
        uart_read_bytes(UART_NUM, data, len, 100); // 读取
        uart_write_bytes(UART_NUM, (const char *)data, len); // 写入

        if (len < BUFF_SIZE) {
            data[len] = '\0';
        } else {
            data[BUFF_SIZE - 1] = '\0';
        }

        ESP_LOGI(TAG, "你的消息为：%s", data);
        // 清空缓存数据
        memset(data, 0, BUFF_SIZE);
    }
    else
    {

    }
}
/**********************************************
 * @brief 串口初始化
 * @param buadrate  波特率
 * @retval 无
 **********************************************/
void uart_init(uint32_t buadrate)
{
    uart_config_t uart_conf = {
        .baud_rate = buadrate,
        .data_bits = UART_DATA_8_BITS,      // 8位数据
        .parity = UART_PARITY_DISABLE,      // 无校验
        .stop_bits = UART_STOP_BITS_1,      // 停止位
        .source_clk = UART_SCLK_APB,        // APB时钟
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,         // 接收缓冲区阈值
    };
    uart_param_config(UART_NUM, &uart_conf);

    // 映射UART引脚
    uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 安装UART驱动程序
    uart_driver_install(UART_NUM, BUFF_SIZE*2, BUFF_SIZE*2, 0, NULL, 0);
}