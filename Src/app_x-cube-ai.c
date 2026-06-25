
/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @author  X-CUBE-AI C code generator
  * @brief   AI program body — integrated with network_3 (GCC / CubeIDE)
  ******************************************************************************
  */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "network_3.h"
#include "network_3_data.h"
#include "har_app.h"

/* Private variables ---------------------------------------------------------*/
static ai_handle network_3 = AI_HANDLE_NULL;

#define AI_NETWORK_3_ACTIVATIONS_SIZE  25216
static ai_u8 activations[AI_NETWORK_3_ACTIVATIONS_SIZE];

/* Entry points --------------------------------------------------------------*/

void MX_X_CUBE_AI_Init(void)
{
  ai_error err;
  const ai_handle acts[] = { activations };
  const ai_handle weights[] = { AI_HANDLE_PTR(s_network_3_weights_array_u64) };

  err = ai_network_3_create_and_init(&network_3, acts, weights);
  if (err.type != AI_ERROR_NONE)
  {
    while (1) {}
  }
}

void MX_X_CUBE_AI_Process(void)
{
  /* USER CODE BEGIN 6 */
  HAR_Process();
  /* USER CODE END 6 */
}

/*
 * Override the weak HAR_RunNetwork() from har_app.c.
 * Uses the real X-CUBE-AI network_3 inference engine.
 */
int HAR_RunNetwork(const float *input,
                   size_t input_element_count,
                   float *output_scores,
                   size_t output_class_count)
{
  ai_buffer *ai_input;
  ai_buffer *ai_output;
  ai_i32 batch;

  if (network_3 == AI_HANDLE_NULL)
    return 0;

  if ((input_element_count < (size_t)AI_NETWORK_3_IN_1_SIZE) ||
      (output_class_count < (size_t)AI_NETWORK_3_OUT_1_SIZE))
    return 0;

  ai_input  = ai_network_3_inputs_get(network_3, NULL);
  ai_output = ai_network_3_outputs_get(network_3, NULL);

  ai_input[0].data  = AI_HANDLE_PTR(input);
  ai_output[0].data = AI_HANDLE_PTR(output_scores);

  batch = ai_network_3_run(network_3, ai_input, ai_output);

  return (batch > 0) ? 1 : 0;
}

#ifdef __cplusplus
}
#endif
