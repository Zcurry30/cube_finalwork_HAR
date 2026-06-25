# STM32F767IGT6 Cube AI HAR

Human Activity Recognition (HAR) project for STM32F767IGT6. The model is
trained in Python, exported as Keras/TFLite, then deployed to the MCU with
STM32Cube.AI / X-CUBE-AI.

## Scope

- Motion data input through UART, SD card, USB CDC, and Ethernet.
- HAR inference through a Cube AI generated network.
- Recognition result output through UART.
- No touch screen is required.

## Current Status

- STM32CubeMX / MDK project is present.
- `finalwork.ioc` is prepared for USART3, SDMMC1, USB OTG FS, Ethernet RMII,
  FATFS, USB_DEVICE CDC, LwIP, GPIO, and X-CUBE-AI.
- HAR application skeleton is in `Inc/har_app.h` and `Src/har_app.c`.
- Python training workflow is in `training/train_har.py`.
- The project has no connected development board yet, so hardware hooks are
  still weak stubs.

## UART Debug

The current board serial stream uses the onboard CH340G USB-UART bridge on
USART1:

- PA9: USART1_TX
- PA10: USART1_RX
- 115200 baud, 8 data bits, no parity, 1 stop bit

Send one CSV sample line from the serial assistant:

```text
0.01,0.02,0.98,0.1,0.0,0.2
```

The firmware immediately echoes a completed line:

```text
RX,0.01,0.02,0.98,0.1,0.0,0.2
```

If the line cannot be parsed, it returns an `ERR,...` message. A classification
result appears after the HAR window receives 128 valid samples.

## Data Format

Training CSV:

```csv
timestamp,ax,ay,az,gx,gy,gz,label
0.00,0.01,0.02,0.98,0.1,0.0,0.2,standing
0.02,0.02,0.01,1.01,0.1,0.1,0.2,standing
```

Runtime input lines may omit timestamp and label:

```text
ax,ay,az,gx,gy,gz
```

## Train

Install dependencies:

```powershell
py -3 -m pip install -r training/requirements.txt
```

Put CSV files in `data/raw/`, then train:

```powershell
py -3 training/train_har.py --data-dir data/raw --output-dir models/har
```

Outputs:

- `models/har/har_model.keras`
- `models/har/har_model.tflite`
- `models/har/labels.json`
- `models/har/normalization.json`

Import `har_model.keras` or `har_model.tflite` into STM32CubeMX X-CUBE-AI,
generate `network.c/.h`, then replace `HAR_RunNetwork()` with the generated
network call.

## STM32 Integration Points

Weak hooks to implement after CubeMX generates the real drivers:

- `HAR_UART_ReadLine`
- `HAR_SD_ReadLine`
- `HAR_USB_ReadLine`
- `HAR_ETH_ReadLine`
- `HAR_OutputSerial`
- `HAR_RunNetwork`

See `HAR_DEPLOYMENT.md` for the detailed deployment checklist.
