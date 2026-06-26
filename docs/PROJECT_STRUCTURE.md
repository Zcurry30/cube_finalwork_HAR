# Repository Organization

This repository keeps source code, generated STM32 dependencies, training code,
and public-facing documentation in separate top-level areas.

## Source And Firmware

- `Src/`: STM32 application source files and Cube.AI generated source files.
- `Inc/`: STM32 application headers and Cube.AI generated headers.
- `Drivers/`: STM32 HAL and CMSIS dependencies.
- `Middlewares/`: ST middleware, including USB device support and X-CUBE-AI
  runtime files.
- `finalwork.ioc`: STM32CubeMX configuration.
- `MDK-ARM/`: Keil MDK project files.

## Model Training

- `training/`: Python workflow for preparing HAR datasets and exporting models.
- `data/`: local dataset directory, ignored by Git.
- `models/`: local trained model output directory, ignored by Git.

## Documentation And Media

- `README.md`: GitHub project entry point.
- `docs/HAR_DEPLOYMENT.md`: deployment and model integration checklist.
- `docs/CUBEMX_PINOUT.md`: STM32CubeMX interface and pinout notes.
- `media/demo/`: reserved place for the final demonstration video.

## Ignored Outputs

The repository ignores build products, local training data, generated model
artifacts, virtual environments, editor metadata, and operating system files.
