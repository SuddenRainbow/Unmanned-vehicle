# test6-way

巡线实验工程。黑线应位于左右两个红外传感器中间，地板返回 `0`，黑线返回 `1`。

## Hi3861

- 主程序：`way.c`
- 构建目标：`static_library("way")`
- 父级 `applications/sample/wifi-iot/app/BUILD.gn` 中启用：

```gn
"test6-way:way",
```

已移除 WiFi、蓝牙、云端、OLED、SHT20、AP3216C、超声波和舵机依赖。

引脚：

- 左红外：GPIO13
- 右红外：GPIO14
- 到 STM32：UART2，GPIO11 TX，GPIO12 RX，115200

判断：

- `0,0`：黑线在两传感器中间，直行
- `1,0`：左传感器压到黑线，立即向左短修正
- `0,1`：右传感器压到黑线，立即向右短修正
- `1,1`：横向黑线标记

终点按“黑-白-黑”判定：第一根横向黑线宽约 `1.9cm`，中间白色间隔约 `1.5cm`，再遇到第二根横向黑线才算终点。检测终点时，只要传感器从第一根黑线进入非双黑状态，就立刻开始寻找第二根黑线，避免中间白线太短导致漏检。检测到终点后停车并发送闪灯命令。

主循环每 `10ms` 轮询一次红外状态。修正不是固定长时间转向，而是每次按最新红外值发一帧短修正命令，下一轮马上重新判断，避免压线后继续转过头。

如果双黑不是终点，就按岔路口处理，不再倒车。第 `1` 次双黑向左偏，第 `2` 次双黑向右偏，后续按奇偶交替。偏向动作结束后继续进入 `10ms` 轮询巡线。

## STM32

Keil 工程：`TIMER/USER/TIMER.uvprojx`

保留模块：

- USART1 接收 Hi3861 控制帧
- TIM4 PWM 电机控制
- WS2812 车灯控制

已移除 PID/编码器工程引用。收到 `E\n` 后停车并循环闪烁所有车灯。

## 调参记录

### 参数速查

| 参数 | 值 | 作用 | 调法 |
| --- | --- | --- | --- |
| `CRUISE_SPEED` | 125 | 直行速度 | 起步无力就加大，冲线就减小 |
| `CORRECT_FAST_SPEED` | 145 | 纠偏时外侧轮速度 | 决定纠偏力度 |
| `CORRECT_SLOW_SPEED` | 55 | 纠偏时内侧轮速度 | 与快轮差值越大拐得越急 |
| `SAMPLE_PERIOD_MS` | 10 | 红外轮询周期 | 越小响应越快 |
| `START_BOOST_MS` / `START_BOOST_SPEED` | 250 / 135 | 起步加速 | 起步打滑或起不动时调 |
| `MARKER_CONFIRM_MS` | 50 | 双黑确认时长 | 防止单次误读触发标记 |
| `JUNCTION_BIAS_MS` | 260 | 岔路偏向时长 | 拐不足调大，拐过头调小 |
| `JUNCTION_LEFT/RIGHT_FAST/SLOW_SPEED` | 145 / 55 | 岔路差速 | 配合 `JUNCTION_BIAS_MS` 用 |
| `JUNCTION_RELEASE_TIMEOUT_MS` | 700 | 离开岔路超时 | 一般不动 |
| `FINISH_DETECT_SPEED` | 80 | 找终点时的慢速 | 太快会冲过终点 |
| `FINISH_FIRST_RELEASE_TIMEOUT_MS` | 350 | 等第一根黑线离开 | 一般不动 |
| `FINISH_WHITE_GAP_MAX_MS` | 350 | 黑白黑间隔上限 | 漏终点就调大 |
| `FINISH_SECOND_CONFIRM_MS` | 10 | 第二根黑线确认时长 | 抗干扰 |

### 实车调试踩过的坑

1. **STM32 上电就卡死（车完全不动）**：现象是串口只打印 `QST motor a` 就停了。原因是 `main.c` 里开了 `SysTick_Config` 的 1ms 中断，但精简掉编码器/PID 时把 `SysTick_Handler` 一起删了，上电 1ms 就跳进启动文件默认死循环。删掉那行 `SysTick_Config` 即可——本工程的 `delay_ms` 是忙等实现，根本不需要 SysTick。
2. **Hi3861 串口发不出**：`UartInit(..., NULL)` 不会配置发送参数，改成完整 `extraAttr`（256 缓冲 / 非阻塞 / FIFO 1/8）后才正常发帧。
3. **车完全不动先查电源**：`M-5V` 电机电源开关没开时，STM32 程序跑着车也不动，别先怀疑代码。
4. **左右修正方向反了**：交换 `CarCorrectLeft()` / `CarCorrectRight()` 里两个轮子的速度即可。

### 调参建议

- 岔路口拐不足：加大 `JUNCTION_BIAS_MS`（比如 320、380 这样逐步试），拐过头再减回来。
- 漏终点：优先查 `FINISH_WHITE_GAP_MAX_MS` 和 `FINISH_DETECT_SPEED`。
- 直线走不稳：调 `CRUISE_SPEED` 和两个纠偏速度。
