# STM32 Smart Car — 智能循迹小车 v4.0

基于 **STM32F103C8T6** (ARM Cortex-M3) + **FreeRTOS 实时操作系统**的智能循迹小车系统。采用 4 通道全 PWM 电机驱动、双路红外循迹传感器、HC-SR04 超声波测距（EXTI 外部中断）、七段数码管状态显示、USART1 串口通信, 通过 FreeRTOS 多任务调度实现自动循迹 + 丢线找回功能。

## 技术栈

| 类别 | 技术 / 工具 |
|------|------------|
| MCU | STM32F103C8T6 (LQFP48, 64KB Flash, 20KB SRAM, 72MHz) |
| RTOS | FreeRTOS V9.0.0 (抢占式调度, 1000Hz tick, 17KB 堆) |
| 开发环境 | Keil MDK-ARM v5.x (µVision5) |
| 驱动框架 | STM32 Standard Peripheral Library (标准外设库) |
| 外设接口 | GPIO / TIM(延时+PWM+中断) / USART1 / EXTI |
| 任务同步 | Queue (消息队列) + Binary Semaphore (二值信号量) |

## 功能特性

### 核心功能模块

| # | 模块 | 硬件 | 关键技术 | 状态 |
|---|------|------|---------|------|
| 1 | **FreeRTOS 调度** | — | 抢占式 RTOS, 2 激活任务 + 2 注释任务, 信号量+队列同步 | ✅ 运行 |
| 2 | **红外循迹** | TCRT5000 × 2 | PB12(右)/PB13(左), IPD 输入, `IRtracking_Lo/Ro()` 电平读取 | ✅ 运行 |
| 3 | **电机驱动** | 直流减速电机 × 2 | TIM4 4通道 PWM (PB6~PB9), 速度范围 -100~+100, 4通道全PWM无方向脚 | ✅ 运行 |
| 4 | **超声波测距** | HC-SR04 | PB15(Trig)+PB14(Echo), EXTI14 下降沿中断 + TIM3 10kHz 计时, `D=Time×1.7mm` | ⚠️ 任务已注释 |
| 5 | **数码管显示** | 七段共阳极数码管 × 1 | PA0~PA7 段选, `LED_show_num(0~9)`, 显示循迹方向 | ✅ 运行 |
| 6 | **串口通信** | USB-TTL | USART1 (PA9/PA10, 115200bps, 8N1), RXNE 中断接收, printf 重定向 | ✅ 运行 |
| 7 | **按键输入** | 独立按键 × 1 | PA15 (IPU), `Get_key()` 用 `vTaskDelay(40)` 消抖, 启动循迹 | ✅ 运行 |

---

## 系统架构

```
┌──────────────────────────────────────────────────────────────────────┐
│                    FreeRTOS — 抢占式调度内核                          │
│                                                                      │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐             │
│   │  FollowLine  │  │   FindLine   │  │ Sound_distance│ (注释)      │
│   │   跟线任务    │  │   找线任务    │  │   测距任务    │             │
│   │  prio=1      │  │  prio=1      │  │  prio=1      │             │
│   │  stack=256   │  │  stack=256   │  │  stack=128   │             │
│   └──────┬───────┘  └──────┬───────┘  └──────┬───────┘             │
│          │                 │                 │                      │
│          │   FindLine_Sem  │  Distance_SendQueue                     │
│          │ ──────────────→ │ ──────────────→ │ (注释)              │
│          │   getline_Sem   │                                        │
│          │ ←────────────── │                                        │
│          │                 │                                        │
├──────────┴─────────────────┴─────────────────┴──────────────────────┤
│                    STM32F103C8T6 @ 72 MHz                            │
│                                                                      │
│  ┌─────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐                 │
│  │  TIM2   │ │  TIM3    │ │  TIM4    │ │  USART1  │                 │
│  │ 1MHz    │ │ 10kHz    │ │ 100Hz    │ │ 115200bps│                 │
│  │ Delay_us│ │ Echo计时  │ │ PWM×4CH  │ │ RXNE IRQ │                 │
│  │ 无中断   │ │ IRQ:6/0  │ │ 无中断   │ │ IRQ:7/0  │                 │
│  └─────────┘ └──────────┘ └──────────┘ └──────────┘                 │
│                                                                      │
│  ┌─────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐                 │
│  │ EXTI14  │ │ 循迹×2   │ │ 数码管   │ │  按键    │                 │
│  │ Echo↓沿 │ │ PB12/13  │ │ PA0~PA7  │ │  PA15    │                 │
│  │ IRQ:5/0 │ │ IPD      │ │ Out_PP   │ │ IPU      │                 │
│  └─────────┘ └──────────┘ └──────────┘ └──────────┘                 │
│                                                                      │
│  NVIC Group 4 (4位抢占, 0位响应)  FreeRTOS: SVC/PendSV/SysTick       │
└──────────────────────────────────────────────────────────────────────┘
```

### 任务调度流程

```
main():
  ① NVIC_PriorityGroupConfig(Group_4)      ← FreeRTOS 必需配置
  ② 初始化全部硬件: sound/Serial/Timer(sound+delay+pwm)/IRtracking/Motor/Key/LED
  ③ 创建队列: Distance_SendQueue (3 × uint16_t)
  ④ 创建信号量: FindLine_Semaphore + FindLine_getline_Semaphore
  ⑤ 创建任务: FindLine + FollowLine (Sound_distance/SendMsg 已注释)
  ⑥ vTaskStartScheduler()                   ← 启动 RTOS 调度器

FollowLine 任务 (永久运行):
  while(1):
    LED_show_num(last_line_turn)             ← 数码管显示上次方向
    if 循迹传感器检测到线:
      双轮前进 (L=20, R=20)
      if 仅左传感器检测到: 左停右转 (L=0, R=20), 记录 last=left
      else:                   左转右停 (L=20, R=0), 记录 last=right
    else (丢线):
      根据 last_turn 反向找线
      若 last=right: 左转 (L=20, R=0)
      若 last=left:  右转 (L=0, R=20)
      若 last=none:  启动 FindLine (原地自旋 L=+20, R=-20)

FindLine 任务 (信号量触发):
  while(1):
    等待 FindLine_Semaphore
    原地自旋 (L=+20, R=-20)
    等待 FindLine_getline_Semaphore (3秒超时)
    停车
```

---

## 循迹逻辑详解

### 传感器布局

```
        车头方向 ↑
   ┌─────────────────┐
   │  STM32 + FreeRTOS │
   │                 │
   │  PB13(左) PB12(右) │  ← TCRT5000 红外循迹传感器
   └─────────────────┘
```

- `IRtracking_Lo()` 返回 1 = 左传感器检测到黑线
- `IRtracking_Ro()` 返回 1 = 右传感器检测到黑线
- 传感器为 IPD (下拉输入), 检测到线时输出高电平

### 循迹状态决策表

| 左传感器 | 右传感器 | 左轮 | 右轮 | 车身行为 | last_turn 更新 |
|---------|---------|------|------|---------|---------------|
| 1 | 1 | +20 | +20 | 直行前进 | 保持 |
| 1 | 0 | 0 | +20 | 右转 (左偏修正) | left |
| 0 | 1 | +20 | 0 | 左转 (右偏修正) | right |
| 0 | 0 (last=right) | +20 | 0 | 右转找线 | — |
| 0 | 0 (last=left) | 0 | +20 | 左转找线 | — |
| 0 | 0 (last=none) | +20 | -20 | 原地自旋 (启动 FindLine) | — |

### 丢线找回策略

1. **丢线瞬间**: 根据上次的 `last_turn` 反向寻找（上次右转就继续右转找）
2. **3 秒超时**: 若 3 秒内未找到线, 触发 `FindLine_Semaphore` 启动找线任务
3. **FindLine 任务**: 原地自旋 (左轮+20, 右轮-20), 直到循迹传感器再次检测到线

---

## 电机驱动说明

### 4 通道全 PWM 架构

传统 H 桥用 2 个 GPIO 方向脚 + 1 个 PWM 脚; 本项目采用 **4 通道独立 PWM**, 每个 H 桥输入都接一路 TIM4 PWM 输出:

```
         左轮 H 桥                    右轮 H 桥
   In1(PB6/CH1) ─┐              In3(PB8/CH3) ─┐
                  ├─ 电机 ──┐                   ├─ 电机 ──┐
   In2(PB7/CH2) ─┘          │      In4(PB9/CH4)┘          │
                             │                              │
                            GND                            GND
```

### 速度控制 API

```c
Motor_SetSpeedLeft(int8_t Speed);   // -100 ~ +100
Motor_SetSpeedRight(int8_t Speed);  // -100 ~ +100
```

| Speed 值 | 行为 | 实现原理 |
|----------|------|---------|
| `+1 ~ +100` | 正转, 占空比 = Speed% | CH1/CH3 = Speed, CH2/CH4 = 0 |
| `-1 ~ -100` | 反转, 占空比 = -Speed% | CH1/CH3 = 0, CH2/CH4 = -Speed |
| `0` | 停止 | 4 通道全 0 |

> **速度限幅**: API 内部会自动限幅到 ±100, 超出范围自动截断。

---

## 项目文件结构

```
stm32小车/
├── user/
│   ├── main.c                  # ★ 主程序: FreeRTOS 任务定义 + 循迹逻辑 + 硬件初始化
│   ├── stm32f10x_it.c          # ISR: TIM3(超声波计时) + EXTI15_10(距离计算) + USART1(串口接收)
│   ├── stm32f10x_it.h          # 中断处理函数声明 (SVC/PendSV/SysTick 已注释, FreeRTOS 接管)
│   └── stm32f10x_conf.h        # 外设库头文件使能配置
│
├── Hardware/                   # 外设驱动层
│   ├── Motor.c / .h            # 电机: Init + SetSpeedLeft/Right (-100~+100, TIM4 4通道PWM)
│   ├── sound.c / .h            # HC-SR04: Init(EXTI14) + Start(Trig) + GetValue (Echo=PB14)
│   ├── Irtracking.c / .h       # 循迹: Init + IRtracking_Lo/Ro (PB12/PB13)
│   ├── Serial.c / .h           # USART1: Init(115200bps) + Printf重定向 + SendByte/Array/String/Number
│   ├── Key.c / .h              # 按键: Init(PA15) + Get_key (vTaskDelay消抖)
│   ├── LED.c / .h              # 数码管: Init(PA0~PA7) + LED_show_num(0~9) + LED_OFF/All_ON
│   ├── IR.c / .h               # NEC 红外 (⚠️ 死代码, 未使用, 与 LED/TIM4 冲突)
│
├── system/                     # 系统基础层
│   ├── Timer.c / .h            # 三定时器初始化: Sound_Timer_Init(TIM3) + Delay_Timer_Init(TIM2) + PWM_Timer_Init(TIM4)
│   ├── Delay.c / .h            # Delay_us (TIM2 忙等, 仅 µs 级)
│   └── hardware.h              # ★ 全局引脚宏定义 (SOUND/MOTOR/IRTRACKING/KEY/LED/UART)
│
├── FREERTOS/                   # FreeRTOS 内核
│   ├── include/                # FreeRTOS 头文件 (task.h, queue.h, semphr.h, ...)
│   ├── src/                    # FreeRTOS 源码 (tasks.c, queue.c, list.c, ...)
│   └── port/                   # 移植层 (port.c, heap_4.c, FreeRTOSConfig.h)
│
├── Start/                      # CMSIS 启动代码 + 内核文件
├── library/                    # STM32 标准外设库源码
├── IO_TABLE.md                 # ★ 完整硬件资源分配表 v4.0
├── README.md                   # 本文件
└── *.uvprojx                   # Keil 工程文件
```

---

## 设计亮点

### 1. FreeRTOS 实时操作系统

- **抢占式调度**: 高优先级任务可抢占低优先级任务, 保证实时响应
- **任务隔离**: 循迹、测距、消息处理各自独立任务, 互不阻塞
- **同步原语**: 信号量 (任务启动) + 队列 (数据传递), 替代裸机的全局变量轮询
- **NVIC Group 4**: 4 位全用于抢占优先级, FreeRTOS 在 Cortex-M3 的推荐配置
- **17KB 堆**: 支持动态任务创建和队列分配

### 2. 4 通道全 PWM 电机驱动

- **无 GPIO 方向脚**: 每个电机正反转各用一路独立 PWM, 省去 GPIO 方向控制
- **平滑过渡**: 正反转切换时 PWM 占空比独立控制, 避免传统 GPIO 切换的瞬时电流冲击
- **硬件映射**: TIM4_CH1~CH4 完美对应 PB6~PB9 (STM32F103 默认复用映射)
- **零 CPU 开销**: PWM 由硬件自动产生, 无需中断干预

### 3. 超声波 EXTI 中断驱动

- **外部中断**: PB14(Echo) 下降沿触发 EXTI15_10, 在回波结束瞬间立即计算距离
- **双定时器协作**: TIM3 负责 0.1ms 精确计时, EXTI 负责触发计算
- **临界区保护**: `sound_Start()` 用 `taskENTER_CRITICAL()` 包裹触发脉冲, 防止被打断
- **优于轮询**: 相比旧版的 TIM3 轮询电平, EXTI 中断响应更快、CPU 占用更低

### 4. 循迹丢线找回策略

- **记忆上次方向**: `last_line_turn` 记录最后一次检测到线的方向
- **反向寻找**: 丢线时根据 `last_turn` 继续转向, 利用惯性找回线
- **超时兜底**: 3 秒找不到线则启动 FindLine 任务原地自旋
- **任务间协作**: FollowLine 通过信号量启动 FindLine, FindLine 找到线后通过信号量通知

### 5. NVIC 优先级分层 (FreeRTOS 友好)

| 层级 | 优先级 | 中断 | 设计考量 |
|------|--------|------|---------|
| L1 (最高) | 5 | EXTI14 (Echo) | 测距实时性最关键, 延迟直接导致距离误差 |
| L2 | 6 | TIM3 (计时) | 0.1ms 累加, 容错率高 |
| L3 | 7 | USART1 (串口) | 有硬件缓冲, 容忍延迟 |
| L4 (最低) | 15 | FreeRTOS (SVC/PendSV/SysTick) | RTOS 调度, 空闲时运行 |

> 所有外设 ISR 优先级 (5/6/7) 均 < 11 (`configMAX_SYSCALL_INTERRUPT_PRIORITY`), 可安全调用 `xxxFromISR()` API。

---

## 已知问题 & 待改进

### 🟡 待改进项

- [ ] **TIM4 PWM 频率偏低 (100Hz)**: `PSC=7199, ARR=99` → 100Hz, 会产生可闻电磁噪声。建议改为 `PSC=71, ARR=99` → 10kHz, 超出音频范围。
- [ ] **`sound_GetValue()` 忙等阻塞**: `while(get_distance_flag==0){}` 不释放 CPU, 在 FreeRTOS 中应改用 `xSemaphoreTake()` 阻塞等待。
- [x] **`Delay_us()` 忙等**: `while(TIM2->CNT != xus)` 不释放 CPU, 长时间延时会影响其他任务。
- [ ] **Sound_distance / SendMsg 任务被注释**: 当前仅循迹功能激活, 超声波测距和串口消息处理未运行。
- [x] **Motor.h 注释过时**: 仍写 "TIM2_CH3/PA2, 方向PA6+PA1", 与实际 TIM4 4通道代码不符。
- [x] **IR.c 死代码**: NEC 红外模块未使用, 且与 LED(PA0) 和电机(TIM4) 冲突, 建议删除或重新分配资源。
- [x] **`Delay.h` 参数类型不一致**: 声明 `uint32_t`, 实现为 `uint16_t`。
- [ ] **无 OLED 状态显示**: 旧版 OLED 已移除, 当前仅数码管显示 0~9, 调试信息有限。
- [ ] 可扩展: 蓝牙/WiFi 通信、MPU6050 姿态控制、PID 闭环调速、路径规划算法。

---

## 使用方法

### 编译与烧录

```text
1. 打开工程: Keil µVision5 → Open Project → *.uvprojx
2. 编译: F7 (Build Target)
3. 烧录: F8 (Download) — 通过 SWD (推荐, 因 PA15 占用了 JTDI)
4. 串口调试 (可选):
   波特率 115200, 数据位 8, 停止位 1, 校验 None
   连接 PA9(TX) → USB-TTL RX, PA10(RX) → USB-TTL TX
```

> ⚠️ **调试接口注意**: 因 PA15 作为按键输入, 必须禁用 JTAG 仅保留 SWD。Keil 烧录器配置选 SWD 模式, 勿选 JTAG。

### 快速上手

1. **硬件准备**:
   - 循迹传感器接 PB12(右)/PB13(左), 调整高度使黑线上方输出高电平
   - 电机驱动板 PB6~PB9 对应 H 桥 In1~In4
   - 超声波 PB15(Trig)/PB14(Echo)
   - 数码管 PA0~PA7 对应 A~G+DP (共阳极)
   - 按键接 PA15 (按下接地)

2. **上电**: FreeRTOS 启动, FollowLine 任务开始运行, 数码管显示 0

3. **放置在黑线上**: 车辆自动循迹前进, 数码管显示转向方向 (1=左, 2=右)

4. **丢线**: 车辆根据上次方向反向寻找, 3 秒超时后原地自旋

5. **按键启动**: 按 PA15 按键触发 `SendMsg` 任务 (当前已注释, 需取消注释激活)

---

## 详细文档

- 📋 [**IO 分配表**](./IO_TABLE.md) — 完整引脚分配 / 定时器参数计算 / FreeRTOS 配置 / NVIC 优先级表 / H桥逻辑表 / 循迹状态机 / 数码管编码 / 与旧版差异对比

---

*STM32 Smart Car v4.0 | Author: Steven Lin | License: MIT*
*GitHub: https://github.com/Stevenlam604054442/STM32-SmartCar*
