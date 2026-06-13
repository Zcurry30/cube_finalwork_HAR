#!/usr/bin/env python
"""Train and export a HAR model for STM32Cube.AI.

Input CSV files must contain ax, ay, az, gx, gy, gz and a label column named
label, activity, or class. Optional timestamp/session columns are ignored.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
import pandas as pd
import tensorflow as tf


SENSOR_COLUMNS = ["ax", "ay", "az", "gx", "gy", "gz"]
LABEL_COLUMNS = ["label", "activity", "class"]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Train a HAR model for Cube AI")
    parser.add_argument("--data-dir", type=Path, required=True, help="Folder containing CSV files")
    parser.add_argument("--output-dir", type=Path, required=True, help="Folder for exported models")
    parser.add_argument("--window-size", type=int, default=128, help="Samples per inference window")
    parser.add_argument("--stride", type=int, default=64, help="Sliding-window stride in samples")
    parser.add_argument("--epochs", type=int, default=50)
    parser.add_argument("--batch-size", type=int, default=64)
    parser.add_argument("--validation-ratio", type=float, default=0.2)
    parser.add_argument("--seed", type=int, default=42)
    return parser.parse_args()


def find_label_column(columns: list[str]) -> str:
    normalized = {column.lower(): column for column in columns}
    for candidate in LABEL_COLUMNS:
        if candidate in normalized:
            return normalized[candidate]
    raise ValueError(f"Missing label column. Expected one of: {', '.join(LABEL_COLUMNS)}")


def load_windows(data_dir: Path, window_size: int, stride: int) -> tuple[np.ndarray, np.ndarray]:
    windows: list[np.ndarray] = []
    labels: list[str] = []
    csv_files = sorted(data_dir.rglob("*.csv"))

    if not csv_files:
        raise FileNotFoundError(f"No CSV files found in {data_dir}")

    for csv_path in csv_files:
        frame = pd.read_csv(csv_path)
        missing = [column for column in SENSOR_COLUMNS if column not in frame.columns]
        if missing:
            raise ValueError(f"{csv_path} is missing sensor columns: {missing}")

        label_column = find_label_column(list(frame.columns))
        frame = frame.dropna(subset=SENSOR_COLUMNS + [label_column]).reset_index(drop=True)

        if len(frame) < window_size:
            continue

        sensor_values = frame[SENSOR_COLUMNS].to_numpy(dtype=np.float32)
        label_values = frame[label_column].astype(str).to_numpy()

        for start in range(0, len(frame) - window_size + 1, stride):
            end = start + window_size
            window_labels = label_values[start:end]
            values, counts = np.unique(window_labels, return_counts=True)
            majority_label = str(values[np.argmax(counts)])
            windows.append(sensor_values[start:end])
            labels.append(majority_label)

    if not windows:
        raise ValueError("No training windows were created. Check window size and dataset length.")

    return np.stack(windows).astype(np.float32), np.asarray(labels)


def encode_labels(labels: np.ndarray) -> tuple[np.ndarray, list[str]]:
    class_names = sorted(np.unique(labels).tolist())
    class_to_index = {name: index for index, name in enumerate(class_names)}
    encoded = np.asarray([class_to_index[label] for label in labels], dtype=np.int64)
    return encoded, class_names


def split_dataset(
    x: np.ndarray,
    y: np.ndarray,
    validation_ratio: float,
    seed: int,
) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    rng = np.random.default_rng(seed)
    train_indices: list[int] = []
    val_indices: list[int] = []

    for class_id in np.unique(y):
        indices = np.where(y == class_id)[0]
        rng.shuffle(indices)
        val_count = max(1, int(round(len(indices) * validation_ratio)))
        val_indices.extend(indices[:val_count].tolist())
        train_indices.extend(indices[val_count:].tolist())

    rng.shuffle(train_indices)
    rng.shuffle(val_indices)

    if not train_indices or not val_indices:
        raise ValueError("Dataset split failed. Add more samples per class.")

    return x[train_indices], y[train_indices], x[val_indices], y[val_indices]


def normalize(
    x_train: np.ndarray,
    x_val: np.ndarray,
) -> tuple[np.ndarray, np.ndarray, dict[str, list[float]]]:
    mean = x_train.reshape(-1, x_train.shape[-1]).mean(axis=0)
    std = x_train.reshape(-1, x_train.shape[-1]).std(axis=0)
    std = np.maximum(std, 1e-6)

    x_train_norm = (x_train - mean) / std
    x_val_norm = (x_val - mean) / std
    metadata = {
        "columns": SENSOR_COLUMNS,
        "mean": mean.astype(float).tolist(),
        "std": std.astype(float).tolist(),
    }
    return x_train_norm.astype(np.float32), x_val_norm.astype(np.float32), metadata


def build_model(window_size: int, axis_count: int, class_count: int) -> tf.keras.Model:
    inputs = tf.keras.Input(shape=(window_size, axis_count), name="sensor_window")
    x = tf.keras.layers.Conv1D(32, kernel_size=5, padding="same", activation="relu")(inputs)
    x = tf.keras.layers.BatchNormalization()(x)
    x = tf.keras.layers.MaxPooling1D(pool_size=2)(x)
    x = tf.keras.layers.Conv1D(64, kernel_size=5, padding="same", activation="relu")(x)
    x = tf.keras.layers.BatchNormalization()(x)
    x = tf.keras.layers.MaxPooling1D(pool_size=2)(x)
    x = tf.keras.layers.Conv1D(96, kernel_size=3, padding="same", activation="relu")(x)
    x = tf.keras.layers.GlobalAveragePooling1D()(x)
    x = tf.keras.layers.Dropout(0.25)(x)
    outputs = tf.keras.layers.Dense(class_count, activation="softmax", name="activity")(x)

    model = tf.keras.Model(inputs=inputs, outputs=outputs, name="har_cnn")
    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=1e-3),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"],
    )
    return model


def export_tflite(model: tf.keras.Model, output_path: Path) -> None:
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    tflite_model = converter.convert()
    output_path.write_bytes(tflite_model)


def main() -> None:
    args = parse_args()
    tf.keras.utils.set_random_seed(args.seed)
    args.output_dir.mkdir(parents=True, exist_ok=True)

    x, label_names = load_windows(args.data_dir, args.window_size, args.stride)
    y, class_names = encode_labels(label_names)
    x_train, y_train, x_val, y_val = split_dataset(x, y, args.validation_ratio, args.seed)
    x_train, x_val, normalization = normalize(x_train, x_val)

    model = build_model(args.window_size, len(SENSOR_COLUMNS), len(class_names))
    callbacks = [
        tf.keras.callbacks.EarlyStopping(
            monitor="val_accuracy",
            patience=10,
            restore_best_weights=True,
        )
    ]
    history = model.fit(
        x_train,
        y_train,
        validation_data=(x_val, y_val),
        epochs=args.epochs,
        batch_size=args.batch_size,
        callbacks=callbacks,
        verbose=2,
    )

    keras_path = args.output_dir / "har_model.keras"
    tflite_path = args.output_dir / "har_model.tflite"
    model.save(keras_path)
    export_tflite(model, tflite_path)

    (args.output_dir / "labels.json").write_text(
        json.dumps({"classes": class_names}, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    (args.output_dir / "normalization.json").write_text(
        json.dumps(normalization, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    val_loss, val_accuracy = model.evaluate(x_val, y_val, verbose=0)
    summary = {
        "window_size": args.window_size,
        "stride": args.stride,
        "axis_count": len(SENSOR_COLUMNS),
        "class_count": len(class_names),
        "classes": class_names,
        "train_windows": int(len(x_train)),
        "validation_windows": int(len(x_val)),
        "validation_loss": float(val_loss),
        "validation_accuracy": float(val_accuracy),
        "epochs_ran": len(history.history["loss"]),
    }
    (args.output_dir / "training_summary.json").write_text(
        json.dumps(summary, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    print(json.dumps(summary, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
