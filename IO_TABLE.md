# STM32 智能小车 — 硬件资源分配表 v4.0

> **MCU**: STM32F103C8T6 (LQFP48, ARM Cortex-M3, 64KB Flash, 20KB SRAM)
> **主频**: 72 MHz
> **开发环境**: Keil MDK-ARM v5 / STM32 标准外设库
> **RTOS**: FreeRTOS V9.0.0 (抢占式调度, 1000Hz tick, 17KB 堆)
> **最后更新**: 2026-07-14 (架构大改: FreeRTOS + 循迹 + 4通道PWM + EXTI中断)

---

## 一、资源总览

| 外设模块 | 使用引脚 | GPIO 模式 | 复用功能 | 定时器 / 通信 |
|----------|----------|-----------|----------|---------------|
| 超声波 Trig | PB15 | Out_PP (推挽) | — | GPIO + Delay_us |
| 超声波 Echo | PB14 | IPD (下拉输入) | EXTI_Line14 | EXTI15_10 下降沿中断 |
| 左电机 PWM (正转) | PB6 | AF_PP (复用推挽) | TIM4_CH1 | TIM4 输出比较 |
| 左电机 PWM (反转) | PB7 | AF_PP (复用推挽) | TIM4_CH2 | TIM4 输出比较 |
| 右电机 PWM (正转) | PB8 | AF_PP (复用推挽) | TIM4_CH3 | TIM4 输出比较 |
| 右电机 PWM (反转) | PB9 | AF_PP (复用推挽) | TIM4_CH4 | TIM4 输出比较 |
| 循迹左传感器 | PB13 | IPD (下拉输入) | — | GPIO 电平读取 |
| 循迹右传感器 | PB12 | IPD (下拉输入) | — | GPIO 电平读取 |
| 串口 TX | PA9 | AF_PP (复用推挽) | USART1_TX | USART1 |
| 串口 RX | PA10 | IPU (上拉输入) | USART1_RX | USART1 + RXNE 中断 |
| 按键 KEY | PA15 | IPU (上拉输入) | — | FreeRTOS vTaskDelay 消抖 |
| 数码管段选 A~G,DP | PA0~PA7 | Out_PP (推挽) | — | 共阳极七段数码管 |

> **已占用 19/44 个可用 GPIO**, 剩余 25 个空闲引脚可扩展。
> **注**: PA15 默认为 JTDI, 需禁用 JTAG (仅保留 SWD) 才能作为普通 GPIO 使用。

---

## 二、定时器资源分配

STM32F103C8T6 定时器总览: TIM1(高级)/TIM2~TIM4(通用, 16bit), 本项目使用其中 **3 个通用定时器**

| 参数 | TIM2 (延时计数器) | TIM3 (超声波计时) | TIM4 (电机 PWM) |
|------|-------------------|-------------------|-----------------|
| **功能** | Delay_us 精确微秒延时 | Echo 高电平脉宽计时 | 4 通道 PWM 输出 |
| **工作模式** | 向上计数 (无中断) | 更新中断 (向上计数) | 输出比较 PWM1 × 4 通道 |
| **PSC (预分频)** | 71 | 0 | 7199 |
| **ARR (自动重装)** | 0xFFFF (65535) | 7199 | 99 |
| **计数频率** | 72M / 72 = **1 MHz** | 72M / 1 = **72 MHz** | 72M / 7200 = **10 kHz** |
| **分辨率 / 中断频率** | 1 µs/tick, 最大 65.5 ms | **10 kHz** (每 0.1 ms) | PWM 频率 = 10k / 100 = **100 Hz** |
| **使用通道** | 无 | 仅更新中断 | CH1~CH4 (PB6~PB9) |
| **是否使用中断** | ❌ 否 | ✅ 是 | ❌ 否 |
| **抢占优先级 / 响应优先级** | — | **6 / 0** | — |
| **源文件** | `system/Timer.c` → `Delay_Timer_Init()` | `system/Timer.c` → `Sound_Timer_Init()` | `system/Timer.c` → `PWM_Timer_Init()` |

### Delay_us 工作原理 (TIM2)

```
PSC=71 → 计数频率 = 72MHz / 72 = 1MHz (1µs/tick)
Delay_us(xus) 实现:
  TIM2->CNT = 0;                    // 清零计数器
  while(TIM2->CNT != xus);          // 忙等至 CNT 到达目标值
```

> ⚠️ **注意**: `Delay_us` 是**忙等阻塞**, 不会释放 CPU 给 FreeRTOS。在 FreeRTOS 任务中调用会阻塞当前任务但不影响其他任务（因为 FreeRTOS 抢占式调度）。但 `sound_Start()` 中用 `taskENTER_CRITICAL()` 包裹了 `Delay_us(45)`, 会临时关闭中断, 影响 EXTI 响应。

### 超声波计时精度 (TIM3)

```
f_ovf = 72,000,000 / (0 + 1) / (7199 + 1) = 10,000 Hz
T_ovf = 1 / 10,000 = 0.1 ms   ← 每次 Sound_Time++ 代表经过 0.1ms
距离公式: Distance(mm) = Sound_Time × 0.1ms × 34mm/ms / 2 = Sound_Time × 1.7 mm
       = (Sound_Time × 17) / 10  mm   ← EXTI15_10_IRQHandler 中的计算
```

### 电机 PWM 频率 (TIM4)

```
f_PWM = f_CK / (PSC + 1) / (ARR + 1)
      = 72,000,000 / (7199 + 1) / (99 + 1)
      = 72,000,000 / 7200 / 100
      = 100 Hz
```

> ⚠️ **100Hz 偏低**: 典型电机 PWM 频率为 10kHz~20kHz。100Hz 会产生可闻电磁噪声, 且低频 PWM 在低占空比时电机响应不平滑。若需优化, 将 PSC 改为 71 (即 72MHz/72=1MHz 计数), ARR 保持 99, 则 PWM 频率 = 1MHz/100 = 10kHz。

---

## 三、FreeRTOS 配置

> 配置文件: `FREERTOS/port/FreeRTOSConfig.h`

| 参数 | 值 | 说明 |
|------|-----|------|
| `configUSE_PREEMPTION` | 1 | 抢占式调度 |
| `configCPU_CLOCK_HZ` | 72,000,000 | 72 MHz |
| `configTICK_RATE_HZ` | 1000 | 1ms tick 周期 |
| `configMAX_PRIORITIES` | 10 | 任务优先级范围 0~9 (0=最低) |
| `configMINIMAL_STACK_SIZE` | 128 | 空闲任务栈大小 (字) |
| `configTOTAL_HEAP_SIZE` | 17 × 1024 = 17,408 | 堆大小 17KB |
| `configPRIO_BITS` | 4 | NVIC 优先级位数 (0~15) |
| `configMAX_SYSCALL_INTERRUPT_PRIORITY` | 191 (0xB0) | 优先级 11; ≥11 的 ISR 不能调 FreeRTOS API |

### NVIC 分组配置

> **Group 4** (4位抢占优先级, 0位响应优先级) — 见 `user/main.c:L155`
>
> STM32F103 仅 4 位优先级, Group 4 = 全部用于抢占, 无响应优先级。这是 FreeRTOS 在 Cortex-M3 上的推荐配置。

### FreeRTOS 接管的中断

以下 3 个 Cortex-M3 系统异常由 FreeRTOS 接管 (见 `FreeRTOSConfig.h:L131-133`), `stm32f10x_it.c` 中已注释掉:

| 异常 | FreeRTOS 处理函数 | 用途 |
|------|------------------|------|
| SVC | `vPortSVCHandler` | 系统服务调用, 启动第一个任务 |
| PendSV | `xPortPendSVHandler` | 任务上下文切换 |
| SysTick | `xPortSysTickHandler` | 1ms tick 中断, 驱动调度器 |

---

## 四、中断向量与 NVIC 配置

| 中断通道 | ISR 函数位置 | 抢占优先级 | 触发条件 | 功能 |
|---------|-------------|-----------|---------|------|
| **EXTI15_10** | `user/stm32f10x_it.c` → `EXTI15_10_IRQHandler()` | 5 | PB14 下降沿 (Echo 结束) | 计算 `distance = Sound_Time × 17 / 10`, 置位 `get_distance_flag` |
| **TIM3** (全局) | `user/stm32f10x_it.c` → `TIM3_IRQHandler()` | 6 | 每 0.1 ms (更新溢出) | Echo 高电平时 `Sound_Time++` |
| **USART1** | `user/stm32f10x_it.c` → `USART1_IRQHandler()` | 7 | RXNE (收到字节) | 接收字节到 `Serial_RxData`, 置位 `Serial_RxFlag` |
| SVC / PendSV / SysTick | FreeRTOS port | 15 (最低) | RTOS 调度 | 任务切换 + tick |

### 中断优先级分析

| 优先级 | 中断 | 含义 |
|--------|------|------|
| **5 (EXTI)** | 超声波 Echo 下降沿 | 最关键 — 必须在 Echo 结束瞬间计算距离, 延迟会导致测距误差 |
| **6 (TIM3)** | 超声波计时累加 | 0.1ms 一次, 容错率高, 但优先级需高于 USART |
| **7 (USART1)** | 串口接收 | 有硬件缓冲, 容忍短暂延迟 |
| **15 (RTOS)** | FreeRTOS 调度 | 最低优先级, 空闲时才调度 |

> **FreeRTOS API 限制**: 所有 ISR 优先级 (5/6/7) 均 < 11 (`configMAX_SYSCALL_INTERRUPT_PRIORITY`), 因此都可以安全调用 `xxxFromISR()` API。但当前代码中 EXTI/TIM3 ISR 未调用 FreeRTOS API, 仅通过全局变量 `get_distance_flag` 通信。

---

## 五、GPIO 引脚详细分配表

### GPIOA (16 pins)

| 引脚 | 功能 | 方向 | GPIO 模式 | 初始电平 | 所属模块 | 备注 |
|------|------|------|-----------|----------|----------|------|
| **PA0** | LED 段选 A | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮; 显示数字"0"时亮 |
| **PA1** | LED 段选 B | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮; 显示数字"1"时亮 |
| **PA2** | LED 段选 C | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮 |
| **PA3** | LED 段选 D | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮 |
| **PA4** | LED 段选 E | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮 |
| **PA5** | LED 段选 F | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮 |
| **PA6** | LED 段选 G | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮 |
| **PA7** | LED 段选 DP (小数点) | Output | 推挽 (Out_PP) | HIGH | 数码管显示 | 共阳极, LOW 点亮; 当前未使用 |
| **PA8** | *空闲* | — | — | — | 预留扩展 | |
| **PA9** | 串口 TX (发送端) | Output | 复用推挽 (AF_PP) | HIGH | 串口通信 | USART1_TX, 115200bps |
| **PA10** | 串口 RX (接收端) | Input | 上拉 (IPU) | HIGH | 串口通信 | USART1_RX + RXNE 中断 |
| PA11 | *空闲* | — | — | — | 预留扩展 | USB_DM (若用 USB 需注意) |
| PA12 | *空闲* | — | — | — | 预留扩展 | USB_DP (若用 USB 需注意) |
| PA13 | *空闲* | — | — | — | 预留扩展 | SWDIO (调试接口, 勿复用) |
| PA14 | *空闲* | — | — | — | 预留扩展 | SWCLK (调试接口, 勿复用) |
| **PA15** | 按键 KEY | Input | 上拉 (IPU) | HIGH | 按键输入 | 按下=LOW; 需禁用 JTAG (仅用 SWD); `Get_key()` 用 vTaskDelay(40) 消抖 |

---

### GPIOB (16 pins)

| 引脚 | 功能 | 方向 | GPIO 模式 | 初始电平 | 所属模块 | 备注 |
|------|------|------|-----------|----------|----------|------|
| PB0 | *空闲* | — | — | — | 预留扩展 | |
| PB1 | *空闲* | — | — | — | 预留扩展 | |
| PB2 | *空闲* | — | — | — | 预留扩展 | Boot1 引脚, 注意勿在启动阶段拉高 |
| PB3 | *空闲* | — | — | — | 预留扩展 | JTDO (JTAG), 需禁用 JTAG 后可用 |
| PB4 | *空闲* | — | — | — | 预留扩展 | JNTRST, 需禁用 JTAG 后可用 |
| PB5 | *空闲* | — | — | — | 预留扩展 | |
| **PB6** | 左电机 PWM 正转 (TIM4_CH1) | Output | 复用推挽 (AF_PP) | LOW | 电机驱动 | Motor_SetSpeedLeft(Speed>0): CCR1=Speed, CCR2=0 |
| **PB7** | 左电机 PWM 反转 (TIM4_CH2) | Output | 复用推挽 (AF_PP) | LOW | 电机驱动 | Motor_SetSpeedLeft(Speed<0): CCR1=0, CCR2=-Speed |
| **PB8** | 右电机 PWM 正转 (TIM4_CH3) | Output | 复用推挽 (AF_PP) | LOW | 电机驱动 | Motor_SetSpeedRight(Speed>0): CCR3=Speed, CCR4=0 |
| **PB9** | 右电机 PWM 反转 (TIM4_CH4) | Output | 复用推挽 (AF_PP) | LOW | 电机驱动 | Motor_SetSpeedRight(Speed<0): CCR3=0, CCR4=-Speed |
| PB10 | *空闲* | — | — | — | 预留扩展 | |
| PB11 | *空闲* | — | — | — | 预留扩展 | |
| **PB12** | 循迹右传感器 (IRTRACKING_Ro) | Input | 下拉 (IPD) | LOW | 循迹模块 | `IRtracking_Ro()`: 检测到线=1, 无线=0 |
| **PB13** | 循迹左传感器 (IRTRACKING_Lo) | Input | 下拉 (IPD) | LOW | 循迹模块 | `IRtracking_Lo()`: 检测到线=1, 无线=0 |
| **PB14** | 超声波 Echo (信号输入) | Input | 下拉 (IPD) | LOW | 超声波测距 | HC-SR04 回响; EXTI_Line14 下降沿中断 |
| **PB15** | 超声波 Trig (触发脉冲) | Output | 推挽 (Out_PP) | LOW | 超声波测距 | 发送 45µs 高电平触发 |

### GPIOC (仅 PC13 可用)

| 引脚 | 功能 | 方向 | GPIO 模式 | 初始电平 | 所属模块 | 备注 |
|------|------|------|-----------|----------|----------|------|
| PC13 | *空闲* | — | — | — | 预留扩展 | 通常连板载 LED, 需查原理图确认 |

---

## 六、H桥电机驱动逻辑表 (4通道全 PWM)

> **架构**: 传统 H 桥用 GPIO 方向脚 + 单通道 PWM; 本项目用 **4 通道独立 PWM**, 每个 H 桥输入都接一路 PWM。

### 左轮 (H 桥 In1=PB6/CH1, In2=PB7/CH2)

| `Motor_SetSpeedLeft(Speed)` | CCR1 (CH1/In1) | CCR2 (CH2/In2) | 左轮行为 |
|------|-----|-----|---------|
| `Speed > 0` (正转, 1~100) | Speed | 0 | 正转, 占空比 = Speed% |
| `Speed < 0` (反转, -1~-100) | 0 | -Speed | 反转, 占空比 = -Speed% |
| `Speed = 0` (停止) | 0 | 0 | 自由停止 |

### 右轮 (H 桥 In3=PB8/CH3, In4=PB9/CH4)

| `Motor_SetSpeedRight(Speed)` | CCR3 (CH3/In3) | CCR4 (CH4/In4) | 右轮行为 |
|------|-----|-----|---------|
| `Speed > 0` (正转, 1~100) | Speed | 0 | 正转, 占空比 = Speed% |
| `Speed < 0` (反转, -1~-100) | 0 | -Speed | 反转, 占空比 = -Speed% |
| `Speed = 0` (停止) | 0 | 0 | 自由停止 |

### 差速转向逻辑

| 左轮 | 右轮 | 车身运动 |
|------|------|---------|
| +20 | +20 | 直行前进 (慢速) |
| +60 | +60 | 直行前进 (快速) |
| -60 | -60 | 直行后退 |
| 0 | +20 | **原地左转** (左轮停, 右轮正转) |
| +20 | 0 | **原地右转** (左轮正转, 右轮停) |
| +20 | -20 | **原地自旋** (左右轮反向) |

> **注**: `FindLine` 任务使用 `Left=+20, Right=-20` 实现原地自旋找线。

---

## 七、FreeRTOS 任务与同步

> 配置见 `user/main.c`

### 任务列表

| 任务名 | 栈大小 | 优先级 | 状态 | 功能 |
|--------|--------|--------|------|------|
| `FindLine` | 256 字 | 1 | ✅ 激活 | 找线: 收到信号量后原地自旋, 等待循迹传感器检测到线 |
| `FollowLine` | 256 字 | 1 | ✅ 激活 | 跟线: 根据左右循迹传感器状态差速跟随, 丢线后调用 FindLine |
| `Sound_distance` | 128 字 | 1 | ❌ 注释 | 超声波测距: 每 100ms 采集一次距离, 发送到队列 |
| `SendMsg` | 256 字 | 1 | ❌ 注释 | 消息处理: 按键检测 + 循迹启动逻辑 |

### 任务间通信

| 同步原语 | 类型 | 用途 |
|---------|------|------|
| `Distance_SendQueue` | Queue (3 × uint16_t) | 距离数据传递 (Sound_distance → SendMsg) |
| `FindLine_Semaphore` | Binary Semaphore | 启动找线任务 (SendMsg → FindLine) |
| `FindLine_getline_Semaphore` | Binary Semaphore | 通知已找到线 (SendMsg → FindLine) |

### 循迹状态机 (FollowLine 任务)

```
                  ┌─────────────────┐
                  │  检测循迹传感器  │
                  └────────┬────────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
        Lo=1||Ro=1     Lo=1,Ro=0    Lo=0,Ro=0
        (检测到线)    (左偏, 需右转) (丢线)
              │            │            │
              ▼            ▼            ▼
        双轮前进     左停右转      查 last_turn
        L=20,R=20   L=0,R=20     ┌─────┴─────┐
                                 ▼           ▼
                            last=right   last=left
                            L=20,R=0     L=0,R=20
                            (右转找线)   (左转找线)
                                 │           │
                                 └─────┬─────┘
                                       ▼
                              仍丢线 3s → 启动 FindLine
                              (原地自旋 L=+20,R=-20)
```

---

## 八、数码管显示编码 (LED.c)

> 1 位七段共阳极数码管, PA0~PA7 对应 A~G + DP, LOW=点亮

```
    A
   ───
  │   │
F │ G │ B
   ───
  │   │
E │   │ C
   ─── ● DP
    D
```

| 数字 | 点亮的段 (LOW) | 显示效果 |
|------|---------------|---------|
| 0 | A,B,C,D,E,F | ▢ |
| 1 | B,C | │ |
| 2 | A,B,D,E,G | S |
| 3 | A,B,C,D,G | ⊓ |
| 4 | B,C,F,G | ⋔ |
| 5 | A,C,D,F,G | ⊓ |
| 6 | A,C,D,E,F,G | ⊓ |
| 7 | A,B,C | ⋐ |
| 8 | A,B,C,D,E,F,G | ▦ |
| 9 | A,B,C,D,F,G | ⊓ |

> **当前用途**: `FollowLine` 任务中 `LED_show_num(last_line_turn)` 显示上次转向方向 (0=无/1=左/2=右)。

---

## 九、通信接口汇总

| 协议 | 引脚 | 时钟速率 / 参数 | 数据格式 | 角色 | 对接设备 |
|------|------|----------------|---------|------|---------|
| **USART1** | PA9(TX) / PA10(RX) | **115200 bps** | 8N1 (无校验, 1 停止位) | DTE | USB-TTL / 串口调试助手 |
| **超声波 HC-SR04** | PB15(Trig) / PB14(Echo) | 触发脉冲 45µs | Echo 脉宽 = 距离×5.88µs/mm | Master | HC-SR04 模块 |
| **红外循迹** | PB12(右) / PB13(左) | GPIO 电平 | 数字量 0/1 | Reader | TCRT5000 循迹模块 |

---

## 十、电源与功耗估算

| 模块 | 工作电压 | 典型电流 | 功耗 | 备注 |
|------|---------|---------|------|------|
| STM32F103C8T6 | 3.3 V | ~36 mA (72MHz 全速) | ~120 mW | 内核 + 全部外设激活 |
| HC-SR04 超声波 | 5 V | 2 mA (待机) / 15 mA (触发中) | 10~75 mW | 需外部 5V 供电 |
| 直流电机 × 2 | 5~12 V | 100 mA ~ 2 A × 2 | 1 W ~ 48 W | 取决于机械负载和 PWM 占空比 |
| TCRT5000 循迹 × 2 | 3.3~5 V | ~10 mA × 2 | ~66 mW | 红外发射+接收 |
| 七段数码管 × 1 | 3.3 V | ~8 mA × 7 段 (全亮) | ~52 mW | 共阳极, 每段 ~4mA |
| **系统总计** | — | **~200 mA ~ 4 A+** | — | 电机占绝对主导 |

> **供电建议**: 电机独立供电 (5~12V, ≥2A), MCU+外设独立 3.3V (≥500mA), 共地连接。

---

## 十一、遗留代码 / 死代码

| 文件 | 状态 | 说明 |
|------|------|------|
| `Hardware/IR.c` / `IR.h` | ❌ **未使用** | NEC 红外遥控解码模块, main.c 未调用 `IR_Init()`。该模块配置 PA0 + TIM4, 与当前 LED(PA0) 和电机 PWM(TIM4) 冲突。若要启用需重新分配引脚和定时器。 |
| `Hardware/Motor.h` 注释 | ⚠️ **文档错误** | Motor.h:L7 注释写 "TIM2_CH3/PA2, 方向PA6+PA1", 但实际代码用 TIM4 4通道 PB6~PB9。注释未同步更新。 |

---

## 十二、与旧版 (v3.1) 的架构差异

| 维度 | v3.1 (旧) | v4.0 (新) |
|------|-----------|-----------|
| **调度模型** | 裸机 main 循环 (7步) | FreeRTOS 抢占式 RTOS (4任务) |
| **NVIC 分组** | Group 2 (2+2) | Group 4 (4+0, FreeRTOS 要求) |
| **电机驱动** | TIM2 2通道 PWM + GPIO 方向脚 | TIM4 4通道全 PWM (无 GPIO 方向脚) |
| **电机引脚** | PA2/PA3 (PWM) + PA6/PA1/PA8/PA9 (方向) | PB6/PB7/PB8/PB9 (4×PWM) |
| **超声波 Echo** | PA7 + TIM3 轮询电平 | PB14 + EXTI 下降沿中断 + TIM3 计时 |
| **超声波 Trig** | PB0 | PB15 |
| **显示** | OLED SSD1306 (PB8/PB9 软件 I2C) | 七段数码管 (PA0~PA7) |
| **红外遥控** | PA0 + TIM4 CH1 输入捕获 (NEC 解码) | ❌ 已移除 (IR.c 保留为死代码) |
| **循迹** | 无 | PB12/PB13 (TCRT5000) |
| **按键** | PB12/PB13 (2个, 非阻塞状态机) | PA15 (1个, vTaskDelay 消抖) |
| **串口** | USART3 PB10/PB11 (9600bps) | USART1 PA9/PA10 (115200bps) |
| **延时** | SysTick (Delay_us/ms/s) | TIM2 (Delay_us, 仅 µs 级) |
| **功能模式** | STOP/AUTO避障/MANUAL手控 | 循迹跟随 + 找线 (自动) |

---

*文档版本: v4.0 | 最后更新: 2026-07-14 | 基于 main.c/Motor.c/Timer.c/sound.c/Serial.c/IR.c/Irtracking.c/Key.c/LED.c/Delay.c/stm32f10x_it.c/hardware.h/FreeRTOSConfig.h 全量源码审计*
