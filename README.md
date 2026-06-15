# GetTemp_Hum - STM32F103 DHTC12 传感器驱动项目

本项目是在 STM32F103C8T6 芯片上读取 DHTC12 温湿度传感器数据的工程，基于 STM32CubeMX 生成，并使用 HAL 库进行开发。开发环境支持 Keil MDK-ARM (ARM Compiler 6) 及 CMake (GCC)。

## 硬件配置
- **MCU**: STM32F103C8T6
- **通讯方式**: I2C (I2C1)
- **串口调试**: USART1 (115200 bps, 8N1)
- **引脚映射**:
  - I2C1_SCL: PB6
  - I2C1_SDA: PB7
  - USART1_TX: PA9
  - USART1_RX: PA10

## 核心实现
本项目编写了针对 DHTC12 温湿度传感器的驱动（`dht12.c` / `dht12.h`）。在查阅了厂家（广州奥松电子）的通信协议文档后，针对其类 SHT3x 的 I2C 指令集进行了驱动开发：
- **I2C 设备地址**: `0x44`（7位地址）
- **触发测量命令**: `0x2C10`
- **规避硬件 Bug**: 在发送读请求后，直接延时 100ms 等待传感器采样完成，从根本上避开 STM32F1 系列由 I2C 硬件产生的轮询 NACK 死锁问题。
- **数据格式**: 读取 6 字节数据，按位移运算组合为16位整数，计算并转换为摄氏度及相对湿度（含温度补偿）。

## 编译及使用说明
1. **Keil 环境**：
   - 使用 Keil MDK 打开 `MDK-ARM/GetTemp_Hum.uvprojx`
   - 使用 `Rebuild All` 进行完全编译
   - 连接 ST-LINK 等下载器，烧录到开发板后复位。
2. **CMake 环境**：
   - 已内置 `CMakeLists.txt`。使用 arm-none-eabi-gcc 工具链可进行跨平台编译。
3. **查看数据**：
   - 使用串口助手连接 `COM` 端口（波特率 `115200`）
   - 单片机将不断输出格式为 `Temp: 24.1C, Hum: 38.7%` 的环境数据。

## 代码结构
- `Core/Inc/dht12.h`: DHTC12 的 I2C 宏定义及结构体声明。
- `Core/Src/dht12.c`: 传感器的发送命令、延时和数据读取换算实现。
- `Core/Src/main.c`: 串口输出重定向(`printf`)，以及在主循环中不断读取并打印温湿度数据。
- `.gitignore`: 配置了对 Keil 及 CMake 的编译中间文件的忽略，保持仓库整洁。

## 免责声明
本仓库中的代码是学习和测试 DHTC12 与 STM32F1 配合使用的示例。使用前请仔细检查 I2C 引脚及电平是否正确。
