# 无人车（Unmanned-vehicle）

> 北京航空航天大学 2026 实训 · QST「先锋号」鸿蒙智能小车 —— 代码仓库（STM32 底层控制 + Hi3861 鸿蒙应用）。

## 项目简介

本仓库用于存放无人车（智能小车）实训过程中的工程代码，分为两部分：

**一、STM32 底层控制部分**（基于 **STM32F103** 的 Keil MDK5 工程）：

- **`跑马灯/`**：串口收发打印 + 炫彩灯跑马灯（驱动前后共 **12 颗 WS2812 可寻址 RGB 灯**，前灯 6 颗 + 尾灯 6 颗）。
- **`4_PWM驱动机电/`**：**TIM4 输出 PWM** 驱动左右两个电机，支持调速与正反转，并可用串口命令控制电机启停。

**二、Hi3861 鸿蒙应用部分**（基于 **OpenHarmony 1.0 + Hi3861V100**，使用 gn/ninja + scons 构建）：

- **`Hi3861_SG90_UART/`**：**GPIO 模拟 PWM 驱动 SG90 舵机**，支持通过**串口命令**控制舵机转到指定角度（0°/45°/90°/135°/180°）。

## 硬件环境

### STM32 部分

| 项目 | 说明 |
|---|---|
| 主控 | STM32F103（QST「先锋号」小车主板） |
| 调试/下载 | ST-Link |
| 串口 | CH340（USART1，PA9/PA10） |
| 前灯 | WS2812，PC13（`DIL`） |
| 尾灯 | WS2812，PC14（`DIR`） |
| 电机驱动 | L9110S（两路推挽式电机驱动） |
| 电机 PWM | TIM4 CH1（PB6）、CH2（PB7），1kHz |
| 电机方向 | PB13、PB14 |

### Hi3861 部分

| 项目 | 说明 |
|---|---|
| 主控 | Hi3861V100（RISC-V WiFi SoC） |
| 系统 | OpenHarmony 1.0（liteos_riscv） |
| 舵机 | SG90，接 **GPIO2**（3.3V~5V 供电，PWM 0.5~2.5ms 对应 0°~180°） |
| 串口 | UART0，115200 8N1（调试/命令共用） |
| 烧录 | HiBurn（COM 口，需 loader + boot + app 三个文件） |

## 软件环境（环境配置）

### STM32 部分

- **Keil MDK5（uVision5）**
- STM32F1xx 器件支持包（DFP）
- **ST-Link 驱动**（下载/调试）
- **CH340 驱动**（串口收发）
- 串口助手（UartAssist / XCOM 等，波特率 115200）

### Hi3861 部分

- **OpenHarmony 1.0 源码**（含 `vendor/hisi/hi3861` 板级支持）
- 工具链：`gcc_riscv32`（riscv32-unknown-elf-gcc 7.3.0）、`gn`、`ninja`、`hc-gen`、`llvm`
- 构建：`python3 build.py wifiiot` + `./build.sh wifiiot_app`（scons）
- **HiBurn 2.0** 烧录工具、**CH340 驱动**
- 串口助手（UartAssist，115200）

## 目录结构

```
Unmanned-vehicle/
├── README.md
├── 跑马灯/                  # STM32 串口收发打印 + 炫彩灯跑马灯
│   ├── USER/                # 主程序、中断、系统文件、工程文件(.uvprojx)
│   ├── SYSTEM/              # delay / sys / usart 系统层
│   ├── QST_HARDWARE/        # colorful_led 炫彩灯、SYSTEM_CONTROL 整车控制
│   ├── CORE/                # 内核文件 + 启动文件(.s)
│   └── STM32F10x_FWLib/     # STM32F10x 标准外设库（inc + src）
├── 4_PWM驱动机电/            # STM32 PWM 驱动机电
│   ├── USER/                # main.c（电机启停/调速控制）等
│   ├── SYSTEM/              # delay / sys / usart（串口命令解析）
│   ├── QST_HARDWARE/        # motor 电机驱动、colorful_led、SYSTEM_CONTROL
│   ├── CORE/
│   └── STM32F10x_FWLib/
└── Hi3861_SG90_UART/        # Hi3861 鸿蒙 GPIO 驱动 SG90 舵机（串口可控）
    ├── SG90.c               # 舵机 PWM + 串口命令解析 + 多任务
    └── BUILD.gn             # OpenHarmony 构建定义
```

## 编译与烧录

### STM32 部分

1. 用 Keil MDK5 打开对应工程的工程文件（`跑马灯/USER/Template.uvprojx` 或 `4_PWM驱动机电/USER/Template.uvprojx`）。
2. 菜单 **Project → Rebuild all target files**（或 `F7`），确保 `0 Error(s)`。
3. 连接 ST-Link，点 **Download**（`F8`）烧录到单片机。
4. 建议在 `Options for Target → Debug → ST-Link → Settings → Flash Download` 勾选 **Reset and Run**，下载后自动运行。

### Hi3861 部分

1. 将 `Hi3861_SG90_UART/` 目录放到 OpenHarmony 1.0 源码的：
   `applications/sample/wifi-iot/app/4.0_SG90_Uart/`
2. 在 `applications/sample/wifi-iot/app/BUILD.gn` 的 `features` 中注册：
   ```gn
   features = [
       "4.0_SG90_Uart:SG90",
   ]
   ```
3. 在源码根目录执行（需先 export 工具链 PATH）：
   ```bash
   python3 build.py wifiiot          # 编译 OpenHarmony 侧库
   cd vendor/hisi/hi3861/hi3861 && ./build.sh wifiiot_app   # scons 编出固件
   ```
   固件输出在 `out/wifiiot/`，烧录需要 `Hi3861_loader_signed.bin` + `Hi3861_wifiiot_app_allinone.bin` + `Hi3861_boot_signed_B.bin`。
4. 用 **HiBurn** 烧录：勾 `Select all` + `Auto burn`，选 COM 口，`Select file` 加载上述三个文件，`Connect` 后按板子复位键，等待 `Execution Successful`。

## 使用方法一：触发跑马灯（`跑马灯/`）

1. 用 CH340 串口线连接小车，打开串口助手，波特率 **115200**。
2. 复位后，串口会打印：

   ```
   QST青软
   ```

3. 在发送框输入 **`HELLO`**（5 个字节，`H` 开头、`O` 结尾）并发送，前后 **12 颗灯**就会一起跑马灯。

> 触发逻辑：串口收到以 `H` 开头、第 5 个字节为 `O` 的完整数据时，置位 `USART_RX_STA`，主循环检测到后调用 `L_runingled()`。

## 使用方法二：控制电机启停（`4_PWM驱动机电/`）

1. 打开 `4_PWM驱动机电/USER/Template.uvprojx`，Rebuild → Download（**驱动电机建议接电池供电**）。
2. 串口助手波特率 **115200**，复位后打印 `QST青软`。
3. 发送 **`START`** → 左右轮开始转动（约 35% 占空比）；发送 **`STOPP`** → 停止。

| 命令 | 效果 | 串口回复 |
|---|---|---|
| `START` | 两轮启动 | `电机启动` |
| `STOPP` | 两轮停止 | `电机停止` |

4. 调速 / 反转：修改 `main.c` 中 `Set_Pwm(2500,2500)` 的数值（范围 -7199 ~ 7199，负数反转）。

> 命令必须大写且正好 5 个字符；串口中断解析 `S` 开头、第 5 字节为 `T`/`P` 的命令，置 `USART_RX_STA=1/2`，主循环据此控制电机启停。

## 使用方法三：串口控制舵机（`Hi3861_SG90_UART/`）

1. 烧录后打开串口助手（115200），复位后打印横幅：
   ```
   UartInit ret=0
   === SG90 UART v3 READY, send 0/45/90/135/180 ===
   ```
2. 在发送框输入角度数字并发送，舵机转到对应角度：

| 发送 | 舵机角度 | 脉宽 |
|---|---|---|
| `0` | 0° | 0.5ms |
| `45` | 45°（左转） | 1.0ms |
| `90` | 90°（居中） | 1.5ms |
| `135` | 135°（右转） | 2.0ms |
| `180` | 180° | 2.5ms |

3. 串口会回显：
   ```
   RX(2): 34 35
   OK angle=45
   done
   ```

## 核心代码思路

### 炫彩灯跑马灯（`跑马灯/`）

- 每颗 WS2812 灯用 3 字节（G、R、B）表示颜色，先写入数组 `L_ws_data[]` / `R_ws_data[]`，再通过 `L_ws2812_refresh()` / `R_ws2812_refresh()` 按位发送。
- 位时序用 `__NOP()` 延时凑出：`0 码 = 高 400ns + 低 850ns`，`1 码 = 高 850ns + 低 400ns`，发完拉低复位。
- 跑马灯逻辑：循环改变「哪颗灯是白色、其余熄灭」，每次刷新后延时，形成流光效果。

### PWM 驱动电机（`4_PWM驱动机电/`）

- PWM 频率 = 72MHz / ((PSC+1) × (ARR+1))，取 ARR=7199、PSC=9 → **1000Hz**。
- 占空比 = CCR / (ARR+1)；`Set_Pwm()` 用 `myabs()` 取绝对值、`7199-myabs()` 反向互补，实现正反转。
- L9110S 驱动：IA 接 PWM（PB6/PB7 = TIM4 CH1/CH2），IB 接方向（PB13/PB14）；`Set_Pwm(0,0)` 即停止。

### GPIO 驱动 SG90 舵机（`Hi3861_SG90_UART/`）

- **软件模拟 PWM**：`set_angle(duty)` 里先拉高 GPIO2，`hi_udelay(duty)` 保持高电平，再拉低补足到 20ms 周期。
- **角度↔脉宽映射**：0.5ms=0°、1.0ms=45°、1.5ms=90°、2.0ms=135°、2.5ms=180°；`engine_run()` 连发 50 个脉冲让舵机转到目标角度。
- **串口命令解析**：`UartInit(WIFI_IOT_UART_IDX_0, ...)` 显式初始化 UART0（115200/8N1），`UartRead()` 轮询读取，`atoi()` 把数字字符串转角度。
- **多任务**：用 CMSIS-OS `osThreadNew()` 创建 `UartTask` 后台任务，`APP_FEATURE_INIT()` 挂载应用入口。

## 课程收获

### 环境配置

- 学会了安装与配置 **Keil MDK5** 开发环境，以及 STM32F1xx 器件支持包（DFP）；
- 学会了安装 **ST-Link 下载器驱动** 和 **CH340 串口驱动**，并能排查「设备代码 28 无驱动」等常见问题；
- 学会了搭建 **STM32F103 标准库工程模板**，理解 `CORE / SYSTEM / HARDWARE / FWLib` 的分层结构；
- 学会了搭建 **OpenHarmony 1.0 + Hi3861** 编译环境（gcc_riscv32 + gn + ninja + scons），并掌握 `build.py` 构建流程；
- 学会了使用 **串口助手**（UartAssist / XCOM）与单片机进行串口通信，以及 **HiBurn** 烧录固件。

### 基础代码实现

- 学会了 **GPIO** 的时钟使能、模式配置与输出控制；
- 学会了 **USART 串口**的初始化、收发、接收中断处理，以及 `printf` 重定向；
- 学会了 **WS2812 炫彩灯**的单总线时序驱动（0/1 码、复位、刷新）；
- 学会了 **TIM 定时器 PWM** 的配置（时基、比较输出、预装载），并能计算 PWM 频率与占空比；
- 学会了用 **PWM 驱动 L9110S 电机**，实现转速调节与正反转，以及用串口命令控制电机启停；
- 学会了 **跑马灯 / 电机启停**等状态逻辑的编写，理解了「数据写入 → 刷新输出」的编程思想；
- 学会了在 **OpenHarmony** 中用 **GPIO 软件模拟 PWM** 驱动 **SG90 舵机**，理解脉宽与舵机角度的对应关系；
- 学会了 **Hi3861 的 UART 收发**（`UartInit`/`UartRead`）与 **CMSIS-OS 多任务**（`osThreadNew`/`osMutexNew`），以及用串口命令控制外设。
