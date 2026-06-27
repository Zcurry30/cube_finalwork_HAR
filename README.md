# HAR on STM32F767 — Human Activity Recognition with Cube AI

基于 STM32F767IGT6 + X-CUBE-AI 的**人体活动识别（HAR）**嵌入式系统。通过串口接收 6 轴 IMU 数据（加速度 + 陀螺仪），运行 1D-CNN 模型实时推理，输出 6 类活动标签。

**测试准确率: 91.74% | 模型参数: 30,598 | 推理框架: ST Edge AI Core v2.2.0**

---

## 快速导航

- [系统架构](#系统架构)
- [小组成员](#小组成员)
- [实验报告](#实验报告)
- [活动类别](#活动类别)
- [模型性能](#模型性能)
- [硬件连接](#硬件连接)
- [构建与烧录](#构建与烧录)
- [模型训练](#模型训练)
- [使用说明](#使用说明)
- [踩坑记录](#踩坑记录)
- [文件结构](#文件结构)
- [演示视频](#演示视频)

---

## 小组成员

| 姓名 | 学号 |
|------|------|
| 章琛 | 2453980 |
| 王科竣 | 2451761 |
| 王子悠 | 2452528 |
| 苟睿峰 | 2452764 |

---

## 实验报告

- [PDF 版](<Report/基于STM32H747I-DISCO的嵌入式人体活动识别系统.pdf>)
- [DOCX 版](<Report/基于STM32H747I-DISCO的嵌入式人体活动识别系统.docx>)

---

## 系统架构

```
  PC/XCOM (UART)
       │
       ▼
  CH340C USB-UART ──► USART1 (PA9/PA10)
       │
       ▼
  HAR_Process()   ← 滑动窗口 128 样本 × 6 轴，步长 64
       │
       ▼
  HAR_RunNetwork() → ai_network_3_run()  ← X-CUBE-AI 推理引擎
       │
       ▼
  USART1 TX ──► PC/XCOM 输出结果
```

**时钟**: HSI 16MHz（内部振荡器，避开 HSE 兼容问题）

**输入数据格式**:
```
ax,ay,az,gx,gy,gz
```

**输出格式**:
```
HAR,label=laying,La=100%,Si=0%,St=0%,Wa=0%,WD=0%,WU=0%
```

---

## 活动类别

| 编号 | 类别 | 英文名 |
|------|------|--------|
| 0 | 躺下 | LAYING |
| 1 | 坐着 | SITTING |
| 2 | 站立 | STANDING |
| 3 | 走路 | WALKING |
| 4 | 下楼 | WALKING_DOWNSTAIRS |
| 5 | 上楼 | WALKING_UPSTAIRS |

---

## 模型性能

| 指标 | 数值 |
|------|------|
| 模型架构 | Conv1D(32,5)→BN→MP→Conv1D(64,5)→BN→MP→Conv1D(96,3)→GAP→Dropout→Dense(6) |
| 参数量 | 30,598 (float32) |
| MACC | 1,407,840 |
| 输入尺寸 | 128 × 6 (3.00 KB) |
| 激活内存 | 25,216 B (24.6 KB) |
| Flash 占用 | ~119 KB |
| 训练集 | UCI HAR, 11,764 窗口 |
| 验证准确率 | 97.89% |
| **测试准确率 (holdout)** | **91.74%** |

| 类别 | 准确率 |
|------|--------|
| LAYING | 100.0% |
| WALKING_DOWNSTAIRS | 98.3% |
| WALKING | 94.7% |
| WALKING_UPSTAIRS | 91.6% |
| SITTING | 84.5% |
| STANDING | 82.0% |

---

## 硬件连接

**开发板**: 正点原子阿波罗 Apollo STM32F767 (核心板 E767)

```
[PC USB] ──── Mini-USB 线 ────► [底板 "USB UART" 接口]
                                  (CH340C → USART1 PA9/PA10)
```

- **TX**: PA9 (USART1_TX, AF7) → CH340C RX
- **RX**: PA10 (USART1_RX, AF7) → CH340C TX

### 烧录模式切换

- **烧录**: BOOT0 跳线 → **3V3** (高电平，进入 bootloader)
- **运行**: BOOT0 跳线 → **GND** (低电平，从 Flash 启动)

---

## 构建与烧录

### 软件环境

| 工具 | 版本 |
|------|------|
| STM32CubeIDE | 2.1.1 |
| GCC | 14.3.rel1 (arm-none-eabi) |
| X-CUBE-AI | 10.2.0 / ST Edge AI Core v2.2.0 |
| STM32Cube FW | F7 V1.17.4 |

### 编译

1. 用 CubeIDE 打开项目目录（项目名: `finalwork`）
2. 确认以下 CMSIS 目录已 **排除编译**（右键 → Resource Configurations → Exclude from Build）:
   - `Drivers/CMSIS/RTOS2`、`NN`、`DSP`、`Core_A`、`Core/Template`
3. `Ctrl+B` 编译

### 烧录（串口 bootloader，无需 ST-Link）

```bash
# 将 HEX 文件烧录到芯片 (BOOT0=3V3 进入烧录模式)
python tools/stm32_flash.py COM8 Debug/finalwork.hex

# 烧录完成后 BOOT0 切回 GND，按 RESET，程序启动
```

烧录脚本会自动擦除、写入、校验。如果中途失败，板子断电重启后重试。

---

## 模型训练

### 数据集

使用 [UCI HAR 公开数据集](https://archive.ics.uci.edu/dataset/240/human+activity+recognition+using+smartphones)（Anguita et al., 2013）。

### 训练步骤

```bash
# 1. 安装依赖
pip install -r training/requirements.txt

# 2. 确保 uci_har_csv/train.csv 就位（test.csv 移出 data-dir！）

# 3. 训练
python training/train_har.py \
    --data-dir uci_har_csv \
    --output-dir models/har \
    --epochs 30

# 4. 生成 C 代码
stedgeai generate \
    --target stm32f7 \
    --name network_3 \
    -m models/har/har_model.tflite \
    --compression none \
    --output .ai/network_output

# 5. 复制到 Src/ 和 Inc/
cp .ai/network_output/network_3*.c Src/
cp .ai/network_output/network_3*.h Inc/
```

---

## 使用说明

### 测试步骤

1. BOOT0 跳线 → GND，上电，按 RESET
2. 打开 XCOM 串口助手 (COM8, 115200, 8N1)
3. 看到 `HAR ready. Send CSV: ax,ay,az,gx,gy,gz`
4. 在发送框输入 6 轴数据，勾选「定时发送」10ms
5. 连续发送 128 条以上，接收区出现 `HAR,label=...` 结果

### 验证用例

```
# LAYING (躺下) → 预期: laying 100%
-0.185673,0.737995,0.663749,-0.016237,-0.166201,0.107479

# STANDING (站立) → 预期: standing 99%
0.996258,-0.259543,0.153025,-0.009863,-0.032731,0.054717

# WALKING_UP (上楼) → 预期: walking_up 92%
1.085438,-0.727962,-0.413233,-0.152783,0.727753,0.293054
```

---

## 踩坑记录

以下是本项目调试过程中遇到的实际问题和解决方案。

### 1. CH340C RX 引脚 ❗最重要
Apollo 底板的 USB-UART 芯片是 **CH340C**（不是 CH340G）。连在 **USART1 PA9/PA10**。TX 可发送但 RX 始终不工作——花了大量时间排查。原因是 `MX_GPIO_Init` 将 PA9 设成了 USB_VBUS，必须在 GPIO 初始化之后**重新初始化 USART1** 才能让 RX 生效。

### 2. HSE 时钟崩溃
`.ioc` 配置的 `HSE_BYPASS` 在 Apollo 板上会导致 `SystemClock_Config()` 挂死。改用 **HSI 16MHz** 内部振荡器直接运行，避开外部时钟兼容问题。16MHz 对 HAR 推理已足够。

### 3. nano.specs 不支持 %f
STM32CubeIDE 默认使用 `--specs=nano.specs`，裁剪了 `printf` 的浮点格式化。`%.3f` 输出空字符串。解决方法：将浮点数拆成整数部分 + 小数部分，用 `%d.%03d` 打印。

### 4. SD 卡 / USB 初始化挂死
`MX_SDMMC1_SD_Init()` 在无 SD 卡插入时会挂死；`MX_USB_DEVICE_Init()` 在 PA9 被 UART 占用后也会挂死。当前版本已跳过这两个初始化。

### 5. 串口 bootloader 写入偶发失败
Python 脚本写 Flash 时，约 200 次写入后 bootloader 可能超时。解决：失败后断电重启重试。另外命令字节和补码必须**一起发送**（如 `\x44\xBB`），分开发会被 bootloader 忽略。

### 6. CubeMX 代码生成覆盖
CubeMX 重新生成代码时会**重置**以下文件，每次必须手动恢复：
- `Src/app_x-cube-ai.c` → 网络初始化被清空
- `Inc/constants_ai.h` → 激活缓冲区大小恢复默认
- `Middlewares/ST/AI/` → 整个目录被删除

### 7. 归一化参数丢失
训练时生成的 `normalization.json` 未被提交。没有归一化时模型输出全在 12-17% 随机乱飘（7 类均匀分布）。重新训练后才修复。**一定要保留 normalization.json 并嵌入推理代码。**

### 8. 训练数据泄露
首次训练时 test.csv 放在 data-dir 子目录中，训练脚本用 `rglob` 递归搜索所有 CSV，导致测试数据混入训练集。**测试数据必须完全移出 data 目录。**

### 9. Keil → CubeIDE 迁移
项目最初使用 Keil MDK-ARM，遇到 ARM Compiler 5 与 X-CUBE-AI 库 ABI 不兼容 + MDK-Lite 32KB 代码限制。迁移到 CubeIDE (GCC) 后解决。

### 10. GCC 启动文件
CubeIDE 需要 GCC 语法的 `startup_stm32f767xx.s`（取自 STM32Cube_FW_F7 的 Templates/gcc/），Keil 的 ARMCC 语法启动文件不会被 GCC 链接。

---

## 文件结构

```
har_fresh/
├── finalwork.ioc              # CubeMX 项目配置
├── README.md                  # 本文件
├── Report/                    # 实验报告（PDF 和 DOCX）
│
├── Inc/                       # 头文件
│   ├── har_app.h              # HAR 应用常量
│   ├── har_io_uart.h          # UART 初始化声明
│   ├── network_3*.h           # AI 模型接口 (生成)
│   └── constants_ai.h         # 激活缓冲区大小
│
├── Src/                       # 源文件
│   ├── main.c                 # 主程序入口
│   ├── har_app.c              # HAR 核心逻辑（窗口/解析/归一化/推理）
│   ├── har_io_uart.c          # USART1 裸机驱动
│   ├── app_x-cube-ai.c        # X-CUBE-AI 网络生命周期
│   ├── network_3*.c           # AI 模型 C 代码 (生成)
│   ├── syscalls.c             # newlib 系统调用桩
│   └── startup_stm32f767xx.s  # GCC 启动文件
│
├── Middlewares/ST/AI/         # X-CUBE-AI 运行时
│   ├── Inc/                   # 93 个 AI 运行时头文件
│   └── Lib/                   # libNetworkRuntime1020_CM7_GCC.a
│
├── models/har/                # 训练产出
│   ├── har_model.keras        # Keras 模型
│   ├── har_model.tflite       # TensorFlow Lite 模型
│   ├── normalization.json     # 归一化参数
│   ├── labels.json            # 类别标签
│   └── training_summary.json  # 训练报告
│
├── training/                  # Python 训练
│   ├── train_har.py           # 训练脚本
│   ├── requirements.txt       # Python 依赖
│   └── README.md
│
├── tools/                     # 烧录工具
│   ├── stm32_flash.py         # 串口 HEX 烧录
│   └── stm32_unlock.py        # RDP 读保护解锁
│
├── .ai/network_output/        # ST Edge AI 生成文件
│
└── Debug/                     # CubeIDE 编译产物
    └── finalwork.elf / .hex
```

---

## 演示视频

[点击查看或下载演示视频](docs/demo.mp4)（48MB）

演示内容：硬件连接 → 系统启动 → laying 躺下识别 → standing 站立识别 → walking_up 上楼识别

---

## 参考资料

- [UCI HAR Dataset](https://archive.ics.uci.edu/dataset/240/human+activity+recognition+using+smartphones)
- [ST X-CUBE-AI Documentation](https://www.st.com/en/embedded-software/x-cube-ai.html)
- [STM32CubeIDE User Guide (UM2609)](https://www.st.com/resource/en/user_manual/um2609.pdf)
- [AN3155 - USART protocol used in STM32 bootloader](https://www.st.com/resource/en/application_note/an3155.pdf)

---

## License

MIT
