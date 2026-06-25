/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HAR_APP_H
#define __HAR_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define HAR_AXIS_COUNT              6U
#define HAR_WINDOW_SAMPLES          128U
#define HAR_WINDOW_STRIDE           64U
#define HAR_CLASS_COUNT             6U
#define HAR_LINE_MAX_LEN            160U

typedef enum
{
  HAR_INPUT_UART = 0,
  HAR_INPUT_SD,
  HAR_INPUT_USB,
  HAR_INPUT_ETH,
  HAR_INPUT_COUNT
} HAR_InputSource;

typedef struct
{
  float axis[HAR_AXIS_COUNT];
} HAR_Sample;

typedef struct
{
  HAR_InputSource source;
  const char *label;
  float confidence;
  float scores[HAR_CLASS_COUNT];
} HAR_Result;

void HAR_Init(void);
void HAR_Process(void);
const char *HAR_SourceName(HAR_InputSource source);
const char *HAR_ClassName(uint32_t class_index);

/*
 * Board integration hooks.
 *
 * Implement these functions in a board-specific file after CubeMX generates
 * USART, SD/FATFS, USB CDC, and LwIP drivers. Each read hook must
 * return 1 when it copied one complete ASCII line into buffer, otherwise 0.
 * Supported data line format:
 *   ax,ay,az,gx,gy,gz
 *   timestamp,ax,ay,az,gx,gy,gz
 */
int HAR_UART_ReadLine(char *buffer, size_t buffer_size);
int HAR_SD_ReadLine(char *buffer, size_t buffer_size);
int HAR_USB_ReadLine(char *buffer, size_t buffer_size);
int HAR_ETH_ReadLine(char *buffer, size_t buffer_size);
void HAR_OutputSerial(const char *text);

/*
 * Cube AI bridge.
 *
 * Override this function with the generated X-CUBE-AI network call. The input
 * buffer is flattened as HAR_WINDOW_SAMPLES x HAR_AXIS_COUNT in chronological
 * order. Return 1 on success and fill output_scores with HAR_CLASS_COUNT values.
 */
int HAR_RunNetwork(const float *input,
                   size_t input_element_count,
                   float *output_scores,
                   size_t output_class_count);

#ifdef __cplusplus
}
#endif

#endif /* __HAR_APP_H */
