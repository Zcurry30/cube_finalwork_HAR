# HAR training workflow

This folder trains a Human Activity Recognition model for STM32Cube.AI.

## Dataset

Place CSV files in `data/raw/`. Each file should contain these sensor columns:

- `ax`
- `ay`
- `az`
- `gx`
- `gy`
- `gz`

For training, each row also needs a label column named `label`, `activity`, or
`class`.

Optional columns such as `timestamp`, `subject`, or `session` are ignored by the
model. Windows never cross file boundaries.

## Train

```powershell
py -3 training/train_har.py --data-dir data/raw --output-dir models/har
```

Useful options:

```powershell
py -3 training/train_har.py `
  --data-dir data/raw `
  --output-dir models/har `
  --window-size 128 `
  --stride 64 `
  --epochs 60 `
  --batch-size 64
```

## Outputs

- `har_model.keras`: Keras model for STM32Cube.AI import
- `har_model.tflite`: float32 TensorFlow Lite model for STM32Cube.AI import
- `labels.json`: output class order
- `normalization.json`: per-axis mean and standard deviation used in training
- `training_summary.json`: dataset and validation metrics summary

Keep the class order and normalization parameters synchronized with the STM32
application.
