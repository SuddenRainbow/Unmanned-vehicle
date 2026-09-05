# 无人车（Unmanned-vehicle）

> 北京航空航天大学 2026 实训 · QST「先锋号」鸿蒙智能小车 —— 代码仓库（STM32 底层控制 + Hi3861 鸿蒙应用）。

## 项目简介

本仓库用于存放无人车（智能小车）实训过程中的工程代码，分为两部分：

**一、STM32 底层控制部分**（基于 **STM32F103** 的 Keil MDK5 工程）：

- **`跑马灯/`**：串口收发打印 + 炫彩灯跑马灯（驱动前后共 **12 颗 WS2812 可寻址 RGB 灯**，前灯 6 颗 + 尾灯 6 颗）。
- **`4_PWM驱动机电/`**：**TIM4 输出 PWM** 驱动左右两个电机，支持调速与正反转，并可用串口命令控制电机启停。
- **`6_PID速度闭环直线/`**：**增量式 PID 速度闭环**，编码器反馈控制两轮转速一致，上电自动启动并保持直线行驶。
- **`7_系统通信协议/`**：**任务24 系统通信协议**（STM32↔Hi3861 双系统通信）——STM32 端解析 6 字节协议帧（0xFC…0xFD）并执行 PID 闭环，Hi3861 端位于 `7_系统通信协议/12.0_UART_Correspondence/`。

**二、Hi3861 鸿蒙应用部分**（基于 **OpenHarmony 1.0 + Hi3861V100**，使用 gn/ninja + scons 构建）：

- **`Hi3861_SG90_UART/`**：**GPIO 模拟 PWM 驱动 SG90 舵机**，支持通过**串口命令**控制舵机转到指定角度（0°/45°/90°/135°/180°）。
- **`Hi3861_HC-SR04_Tick/`**：**GPIO 驱动 HC-SR04 超声波测距**，使用 **2 个软件定时器**：每 3 秒测距打印距离、每 1 秒打印系统 tick 值（`hi_get_tick`）。
- **`Hi3861_OLED_Ssd1306/`**：**I2C 驱动 0.91 寸 OLED（SSD1306, 128×32）**，显示字符串 + 时钟，并支持 **16×16 中文字符显示**（"鸿蒙先锋号"点阵由 WQY 字体生成）。
- **`Hi3861_SHT20_TempHum/`**：**I2C 驱动 SHT20 温湿度传感器**，并用 **信号量（Semaphore）** 实现多任务同步（Task1 每 3 秒释放信号量，Task2 读温湿度、Task3 抢占，交替执行）。
- **`Hi3861_Ap3216c_Light/`**：**I2C 驱动 AP3216C 三合一传感器（光照 ALS / 接近 PS / 红外 IR）+ OLED 显示 + LED 自动灯光控制**（晚上亮白天关；或晚上+有人靠近才亮，`LIGHT_MODE` 可切换）。
- **`Hi3861_TCRT_IR/`**：**GPIO 读取红外对管（循迹模块 TCRT）**，使用 **2 个软件定时器**：一个每 3 秒打印 `hello QST`，另一个每 0.5 秒扫描左右红外对管（低电平=黑、高电平=白，循迹原理）。
- **`Hi3861_Obstacle_Avoid/`**：**综合任务·超声波避障 + 红外避黑胶带**——超声波（GPIO7/8）前方 <20cm 障碍（急停→后退→左转绕行），红外对管（GPIO13/14）地面黑胶带（左黑右转/右黑左转/双黑后退掉头），UART2 0xFC 帧驱动电机。
- **`14.0_Bluetooth_control/`**：**任务26 蓝牙遥控小车**——手机蓝牙调试器 APP 通过 **JDY-16 蓝牙模块**（UART1，9600）发字符，Hi3861 解析后按 **0xFC 协议帧经 UART2 发给 STM32** 控制电机（前/后/左/右/停/一键系列动作）。
- **`test6-way/`**：**巡线实验**——Hi3861 读取左右红外（GPIO13/14）判断黑线位置，经 UART2（GPIO11/12，115200）发 **0xFC 电机帧** 给 STM32；STM32 端 TIM4 PWM 驱动电机 + WS2812 车灯，按「黑-白-黑」判定终点并停车闪灯。

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
├── 6_PID速度闭环直线/        # STM32 增量式PID速度闭环直行
│   ├── USER/                # main.c（上电自动跑）
│   ├── SYSTEM/
│   ├── QST_HARDWARE/        # motor/encoder/speed_ctrl(PID)/colorful_led/SYSTEM_CONTROL
│   ├── CORE/
│   └── STM32F10x_FWLib/
├── 7_系统通信协议/            # 任务24 STM32↔Hi3861 系统通信协议
│   ├── USER/                # STM32端: 协议帧解析+PID闭环
│   ├── SYSTEM/              # delay/sys/usart(0xFC..0xFD帧解析)
│   ├── QST_HARDWARE/        # motor/encoder/speed_ctrl/colorful_led/SYSTEM_CONTROL
│   ├── CORE/
│   ├── STM32F10x_FWLib/
│   └── 12.0_UART_Correspondence/   # Hi3861端: 协议帧发送(四动作验证)
│       ├── correspondence.c
│       ├── BUILD.gn
│       └── app_BUILD_corr.gn
├── Hi3861_SG90_UART/        # Hi3861 鸿蒙 GPIO 驱动 SG90 舵机（串口可控）
│   ├── SG90.c               # 舵机 PWM + 串口命令解析 + 多任务
│   └── BUILD.gn             # OpenHarmony 构建定义
├── Hi3861_HC-SR04_Tick/     # Hi3861 鸿蒙 GPIO 驱动 HC-SR04 超声波（2 个软件定时器）
│   ├── Hcsr04.c             # 超声波测距 + tick 打印（osTimerNew 定时器）
│   └── BUILD.gn             # OpenHarmony 构建定义
├── Hi3861_OLED_Ssd1306/     # Hi3861 鸿蒙 I2C 驱动 SSD1306 OLED（含中文点阵）
│   ├── I2c_Ssd1306.c        # 显示"鸿蒙先锋号" + 时钟
│   ├── include/             # hal_bsp_ssd1306 驱动 + 字库(ASCII/中文点阵)
│   ├── src/                 # hal_bsp_ssd1306.c 驱动实现
│   └── BUILD.gn
└── Hi3861_SHT20_TempHum/    # Hi3861 鸿蒙 I2C 驱动 SHT20 温湿度 + 信号量同步
    ├── Sht20.c              # 3 任务信号量演示, 读温湿度
    ├── include/             # hal_bsp_sht20.h
    ├── src/                 # hal_bsp_sht20.c 驱动实现
    └── BUILD.gn
└── Hi3861_Ap3216c_Light/    # Hi3861 鸿蒙 AP3216C 光照/接近/红外 + OLED + LED 自动灯控
    ├── Ap3216c.c            # 传感器读取 + 灯光逻辑(LIGHT_MODE 1/2) + OLED 显示
    ├── include/             # hal_bsp_ap3216c.h / hal_bsp_ssd1306.h / 字库
    ├── src/                 # hal_bsp_ap3216c.c / hal_bsp_ssd1306.c
    └── BUILD.gn
└── Hi3861_TCRT_IR/          # Hi3861 鸿蒙红外对管收发(循迹) + 2 个软件定时器
    ├── TCRT.c              # GPIO13/14 读红外黑白 + 双定时器(hello QST 3s / 扫描 0.5s)
    └── BUILD.gn
├── Hi3861_Car_Edge_Guard/    # 综合任务: 桌面防掉落(电机+红外对管边缘检测)
│   ├── Car_Edge.c            # 直行->急停->后退->掉头 状态机, UART2 0xFC 电机帧
│   └── BUILD.gn
└── Hi3861_Sum_Experiment_First/  # 第一阶段综合实验(寻线/舵机测距/蓝牙/消息队列)
    ├── Sum_Experiment_First.c    # 3任务联动 + osMessageQueue 消息队列
    └── BUILD.gn
└── 14.0_Bluetooth_control/      # 任务26 蓝牙遥控小车(JDY-16→UART1→0xFC帧→UART2→STM32)
    ├── Control.c                # 蓝牙字符解析(W/A/S/D/O/Z) + 0xFC帧发送
    ├── BUILD.gn
    └── app_BUILD_corr.gn        # 注册模板 + APP_INIT_EVENT_NUM=7 提醒
├── Hi3861_Obstacle_Avoid/   # 综合任务: 超声波避障 + 红外避黑胶带
│   ├── Obstacle_Avoid.c      # 障碍<20cm停退绕 / 黑胶带左黑右转右黑左转双黑掉头
│   └── BUILD.gn
└── test6-way/                # 巡线实验: Hi3861 红外循线 + STM32 电机/车灯
    ├── way.c                 # Hi3861 巡线主程序(红外判断/终点检测/岔路处理)
    ├── BUILD.gn
    ├── README.md             # 项目说明 + 调参记录
    └── STM32/                # STM32 端源码(main/usart/motor/colorful_led)
```

## 编译与烧录

### STM32 部分

1. 用 Keil MDK5 打开对应工程的工程文件（`跑马灯/USER/Template.uvprojx`、`4_PWM驱动机电/USER/Template.uvprojx`、`6_PID速度闭环直线/USER/Template.uvprojx` 或 `7_系统通信协议/USER/Template.uvprojx`）。
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

## 使用方法四：超声波测距（`Hi3861_HC-SR04_Tick/`）

1. 烧录后打开串口助手（115200），复位后开始输出。
2. **每 3 秒**打印一次距离：
   ```
   distance is 16.0 (cm)
   ```
3. **每 1 秒**打印一次系统 tick 值：
   ```
   current tick: 1234
   ```
4. 在超声波模块前伸手/挡板，`distance` 数值会变化。

## 使用方法五：系统通信协议（`7_系统通信协议/`，STM32↔Hi3861）
1. STM32：打开 `7_系统通信协议/USER/Template.uvprojx`，Rebuild → Download（接电池）。
2. Hi3861：把 `7_系统通信协议/12.0_UART_Correspondence/` 放到 OpenHarmony `applications/sample/wifi-iot/app/` 下，在 app/BUILD.gn 注册 `"12.0_UART_Correspondence:correspondence"`，`python3 build.py wifiiot` 后 HiBurn 烧录。
3. 接线：3861 GPIO11(UART2_TX)→STM32 PA10(RX)，GPIO12→PA9(TX)，共地，115200。
4. 上电验证四动作：前进 1s → 后退 1s → 左转 1s → 右转 1s 循环。
> 协议帧：`0xFC | 左方向(0正/1反) | 左速度 | 右方向 | 右速度 | 0xFD`，速度精度 0.01 转/s。

## 使用方法六：蓝牙遥控小车（`14.0_Bluetooth_control/`，任务26）
1. STM32 烧 `7_系统通信协议` 固件；Hi3861 烧 `14.0_Bluetooth_control`（**app/BUILD.gn 注册 + 改 `APP_INIT_EVENT_NUM` 5→7**）。
2. 手机装「蓝牙调试器」APP → 连接 **JDY-16** 蓝牙模块。
3. 发送字符：`W`前 `S`后 `A`左 `D`右 `O`停 `Z`一键系列动作。
4. 接线：JDY-16 TX→GPIO1 / RX→GPIO0（UART1 9600）；3861 GPIO11/12（UART2 115200）→STM32 PA10/PA9。
> 架构：手机(蓝牙调试器APP) → JDY-16(UART1 9600) → Hi3861(UART2 115200, 0xFC帧) → STM32 → 电机。

## 核心代码思路

### 炫彩灯跑马灯（`跑马灯/`）

- 每颗 WS2812 灯用 3 字节（G、R、B）表示颜色，先写入数组 `L_ws_data[]` / `R_ws_data[]`，再通过 `L_ws2812_refresh()` / `R_ws2812_refresh()` 按位发送。
- 位时序用 `__NOP()` 延时凑出：`0 码 = 高 400ns + 低 850ns`，`1 码 = 高 850ns + 低 400ns`，发完拉低复位。
- 跑马灯逻辑：循环改变「哪颗灯是白色、其余熄灭」，每次刷新后延时，形成流光效果。

### PWM 驱动电机（`4_PWM驱动机电/`）

- PWM 频率 = 72MHz / ((PSC+1) × (ARR+1))，取 ARR=7199、PSC=9 → **1000Hz**。
- 占空比 = CCR / (ARR+1)；`Set_Pwm()` 用 `myabs()` 取绝对值、`7199-myabs()` 反向互补，实现正反转。
- L9110S 驱动：IA 接 PWM（PB6/PB7 = TIM4 CH1/CH2），IB 接方向（PB13/PB14）；`Set_Pwm(0,0)` 即停止。

### 系统通信协议（`7_系统通信协议/`，任务24）
- STM32 端：串口中断按 `0xFC` 帧头、`0xFD` 帧尾解析 6 字节帧 → `CAR_buff[4]` → 方向还原（1 取负）→ 倒车灯 → `CalculateAndControlMotors()`（转/s×280 → 脉冲/100ms）→ 左右独立增量式 PI（20ms）→ `Set_Pwm`。
- Hi3861 端：`stm32motor_control(motorA, motorB)` 组帧（负数→方向1 并取绝对值，±150 限幅）→ `UartWrite(UART2)` 发送；封装 `car_forward/backward/left/right/stop`；双线程 + 互斥锁演示四动作。
### 巡线实验（`test6-way/`）

- **巡线判断**：Hi3861 每 `10ms` 读一次左右红外（GPIO13/14），`0,0` 直行、`1,0` 左修、`0,1` 右修、`1,1` 按标记处理；修正不是固定时长转向，而是每轮按最新红外值发一帧短修正，避免转过头。
- **终点判定**：按「黑-白-黑」判定（第一根黑线约 1.9cm、白间隔约 1.5cm、再遇第二根黑线），检测到后停车并循环发 `E\n` 让 STM32 闪灯。
- **岔路口处理**：双黑若不是终点就按岔路口处理，第 1 次左偏、第 2 次右偏、之后奇偶交替。
- **两端协作**：Hi3861 组 `0xFC` 帧经 UART2 发 STM32，STM32 串口中断解析后 `Set_Pwm(speed*48)` 驱动电机。
### GPIO 驱动 SG90 舵机（`Hi3861_SG90_UART/`）

- **软件模拟 PWM**：`set_angle(duty)` 里先拉高 GPIO2，`hi_udelay(duty)` 保持高电平，再拉低补足到 20ms 周期。
- **角度↔脉宽映射**：0.5ms=0°、1.0ms=45°、1.5ms=90°、2.0ms=135°、2.5ms=180°；`engine_run()` 连发 50 个脉冲让舵机转到目标角度。
- **串口命令解析**：`UartInit(WIFI_IOT_UART_IDX_0, ...)` 显式初始化 UART0（115200/8N1），`UartRead()` 轮询读取，`atoi()` 把数字字符串转角度。
- **多任务**：用 CMSIS-OS `osThreadNew()` 创建 `UartTask` 后台任务，`APP_FEATURE_INIT()` 挂载应用入口。

### GPIO 驱动 HC-SR04 超声波（`Hi3861_HC-SR04_Tick/`）

- **触发与测量**：GPIO7 输出 ≥10µs 高电平脉冲触发 HC-SR04，模块收到后回 ECHO 高电平；GPIO8 读输入，用 `hi_get_us()` 测高电平持续时间 `time`，`距离 = time × 0.034 / 2`（cm）。
- **2 个软件定时器**：`osTimerNew(callback, osTimerPeriodic, ...)` 创建，`osTimerStart(timer, ticks)` 启动（系统 10ms/tick，300 tick=3s、100 tick=1s）。
  - 定时器 1：每 3 秒 `GetDistance()` 并打印距离；
  - 定时器 2：每 1 秒 `hi_get_tick()` 打印当前系统 tick 值。
- **看门狗**：测距使用 `hi_udelay`/忙等，先 `WatchDogDisable()` 关闭看门狗防止复位（注意 SDK 中函数名 **D 大写**）。

### I2C 驱动 SSD1306 OLED（`Hi3861_OLED_Ssd1306/`）

- **I2C 总线**：OLED 挂 I2C0（SDA=GPIO10、SCL=GPIO9，400kHz），`I2cInit` + `I2cWrite` 收发；从机地址 0x3C。
- **显示流程**：`SSD1306_Init()` 发初始化命令序列 → `SSD1306_CLS()` 清屏 → `SSD1306_ShowStr(x, y, 字符串, 16)` 显示 8×16 ASCII；页寻址模式：每页 8 行、每字节一列。
- **中文显示**：BSP 原 `ShowStr` 仅支持 ASCII；新增 **`SSD1306_ShowChineseByIndex()`**，用 16×16 点阵数组显示汉字（"鸿蒙先锋号"5 字点阵由 WQY 字体渲染生成，位序 bit0=顶行）。
- **注意**：SDK 配置需开启 **`CONFIG_I2C_SUPPORT = y`**（`build/config/usr_config.mk`），否则链接报 `hi_i2c_write undefined`。

### I2C 驱动 SHT20 温湿度 + 信号量（`Hi3861_SHT20_TempHum/`）

- **传感器**：SHT20 挂同一条 I2C0（从机地址 0x40，100kHz），`SHT20_Init()` + `SHT20_ReadData(&temp, &hum)` 读取温湿度（浮点）。
- **信号量同步**：`osSemaphoreNew(4, 0, NULL)` 创建初始值为 0 的信号量；
  - Task1：每 3 秒 `osSemaphoreRelease` 释放一次；
  - Task2 / Task3：`osSemaphoreAcquire(osWaitForever)` 阻塞抢信号量，谁抢到谁执行（Task2 读温湿度打印、Task3 打印自己的标记）——实现任务间同步/互斥。

### AP3216C 光照/接近/红外 + LED 自动灯控（`Hi3861_Ap3216c_Light/`）

- **三合一传感器**：AP3216C 挂 I2C0（从机地址 0x1E，8 位地址 0x3C），`AP3216C_ReadData(&ir, &als, &ps)` 一次读出红外/光照/接近三个值。
- **灯光逻辑**（`LIGHT_MODE` 宏切换）：
  - 模式1（夜间自动路灯）：`als < 150`（光照低=晚上）→ LED 亮，白天灭；
  - 模式2（夜间人感灯）：`als < 150` **且** `ps > 50`（有人靠近）→ 亮，无人不亮；
  - 阈值 `ALS_NIGHT_TH` / `PS_NEAR_TH` 可调，LED 引脚 `LED_GPIO`（IO02）可改。
- **显示**：OLED 上半行 `ALS: 178 PS: 15`、下半行 `LED: OFF IR: 10`，每秒刷新；串口同步打印。

### GPIO 读取红外对管 + 双软件定时器（`Hi3861_TCRT_IR/`）

- **红外对管（TCRT 循迹模块）**：发光管发射红外线，地面颜色反射信号不同——白/亮色反射强（高电平=white），黑色吸收（低电平=black）；TC_OUT_L→GPIO13、TC_OUT_R→GPIO14 输入。
- **2 个软件定时器**（作业要求，各自独立触发）：
  - 定时器1：`osTimerStart(id1, 300)`（300U=3s）→ 打印 `hello QST`；
  - 定时器2：`osTimerStart(id2, 50)`（50U=0.5s）→ `GpioGetInputVal` 读左右红外并打印 `left/right black|white`。
- 现象：`hello QST` 每 3 秒穿插在 0.5 秒一次的扫描输出中，小车压黑线时对应侧输出 `black`——循迹的核心原理。

### 超声波避障 + 红外避黑胶带（`Hi3861_Obstacle_Avoid/`）

- **超声波避障**：HC-SR04（GPIO7 触发/GPIO8 回声）测前方距离，<20cm 判为障碍 → 急停→后退→原地左转绕行→继续；`GetDistance` 加了无回波超时保护。
- **红外避黑胶带**：GPIO13/14 读地面——左黑（左轮压带）右转、右黑左转、双黑（正对胶带）后退+掉头。
- **优先级状态机**：障碍 > 双黑 > 左黑 > 右黑 > 直行；阈值/速度/转向时长全部 `#define` 可调。

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
- 学会了 **增量式 PID 速度闭环** 的设计与调参（Kp/Ki、积分/输出限幅、软启动），用编码器反馈让小车自动保持直线行驶；
- 学会了**自定义通信协议**的设计与两端联调（帧头/帧尾、字段定义、Hi3861 与 STM32 串口交互），并完成 STM32 端协议解析与 PID 闭环执行；
- 学会了在 **OpenHarmony** 中用 **GPIO 软件模拟 PWM** 驱动 **SG90 舵机**，理解脉宽与舵机角度的对应关系；
- 学会了 **Hi3861 的 UART 收发**（`UartInit`/`UartRead`）与 **CMSIS-OS 多任务**（`osThreadNew`/`osMutexNew`），以及用串口命令控制外设；
- 学会了 **HC-SR04 超声波测距**的 GPIO 驱动原理（10µs 触发脉冲 + ECHO 高电平测时 + 距离换算）；
- 学会了 **CMSIS-OS 软件定时器**（`osTimerNew`/`osTimerStart`/`osTimerPeriodic`）与 **`hi_get_tick`** 系统 tick 的使用，理解 tick 与时间的关系；
- 学会了 **I2C 总线**概念与 **`I2cInit`/`I2cWrite`** 的使用，完成 **SSD1306 OLED** 字符/汉字显示；
- 学会了 **SHT20 温湿度传感器**的 I2C 读写与数据换算（`SHT20_ReadData`）；
- 学会了 **CMSIS-OS 信号量**（`osSemaphoreNew`/`osSemaphoreAcquire`/`osSemaphoreRelease`），理解用信号量进行任务间同步与共享资源互斥；
- 学会了 **AP3216C 三合一传感器**（光照/接近/红外）的 I2C 读取与数值含义（`AP3216C_ReadData`）；
- 学会了 **"传感器采集 → 逻辑判断 → 执行器输出"** 的完整闭环（AP3216C 判夜间/有人 → 控 LED 灯 + OLED 显示），并掌握阈值/模式可调的设计方法；
- 学会了**红外对管（循迹模块）**的工作原理（黑色吸光=低电平、白色反射=高电平）与 GPIO 输入读取（`GpioGetInputVal`）；
- 学会了**多软件定时器并行**的设计（不同周期回调、相互独立触发、串口现象交叉可见）；
- 学会了**双芯片协作**（Hi3861 经 UART2 0xFC 帧控制 STM32 电机：方向+速度、负值反转、前进/后退/原地转弯）与**状态机**设计（直行→急停→后退→掉头 的防掉落闭环）；
- 学会了**综合实验多级联动**：红外循线+舵机测距+蓝牙通信+**消息队列**（`osMessageQueueNew/Put/Get`）实现任务间数据传递与交替运行。
