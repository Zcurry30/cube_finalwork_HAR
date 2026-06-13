# STM32F767IGT6 Cube AI HAR

基于 STM32F767IGT6 的人体运动识别系统，使用 Python 训练 HAR 模型，再通过
STM32Cube.AI/X-CUBE-AI 部署到 MCU。

## 功能目标

- 通过串口、SD 卡、USB、以太网输入运动传感器数据
- 使用 Cube AI 部署的模型识别运动状态
- 通过串口或触摸屏输出识别结果

当前工程已经包含：

- STM32CubeMX/MDK 工程骨架
- HAR 应用层窗口缓存、CSV 解析、推理入口
- Python 训练与导出脚本
- CubeMX 外设与 Cube AI 对接说明

## 数据格式

训练集和板端输入都建议使用同一种 CSV 格式：

```csv
timestamp,ax,ay,az,gx,gy,gz,label
0.00,0.01,0.02,0.98,0.1,0.0,0.2,standing
0.02,0.02,0.01,1.01,0.1,0.1,0.2,standing
```

板端实时输入可以省略 `timestamp` 和 `label`：

```text
ax,ay,az,gx,gy,gz
```

## Python 训练

安装依赖：

```powershell
py -3 -m pip install -r training/requirements.txt
```

把 CSV 数据放入 `data/raw/`，然后训练：

```powershell
py -3 training/train_har.py --data-dir data/raw --output-dir models/har
```

训练完成后会生成：

- `models/har/har_model.keras`
- `models/har/har_model.tflite`
- `models/har/labels.json`
- `models/har/normalization.json`

将 `har_model.keras` 或 `har_model.tflite` 导入 STM32CubeMX 的 X-CUBE-AI，
生成 `network.c/.h` 后，把 `HAR_RunNetwork()` 替换为真实网络调用。

## STM32 工程

主要应用层文件：

- `Inc/har_app.h`
- `Src/har_app.c`
- `Src/app_x-cube-ai.c`
- `HAR_DEPLOYMENT.md`

`Src/har_app.c` 中的弱函数用于对接 CubeMX 生成的外设：

- `HAR_UART_ReadLine`
- `HAR_SD_ReadLine`
- `HAR_USB_ReadLine`
- `HAR_ETH_ReadLine`
- `HAR_OutputSerial`
- `HAR_OutputDisplay`
- `HAR_RunNetwork`

当前本地没有连接开发板，因此仓库侧先完成代码结构、训练流程和部署说明。
上板前请先在 CubeMX 里启用 UART、SDMMC/FATFS、USB、ETH/LwIP 和显示屏驱动。
