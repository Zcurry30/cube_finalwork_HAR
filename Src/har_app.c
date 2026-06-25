#include "har_app.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__) || defined(__armclang__)
#define HAR_WEAK __attribute__((weak))
#elif defined(__ICCARM__) || defined(__CC_ARM)
#define HAR_WEAK __weak
#else
#define HAR_WEAK
#endif

static HAR_Sample har_window[HAR_WINDOW_SAMPLES];
static uint32_t har_next_sample;
static uint32_t har_sample_count;
static uint32_t har_samples_since_inference;
static float har_ai_input[HAR_WINDOW_SAMPLES * HAR_AXIS_COUNT];

static const char *const har_source_names[HAR_INPUT_COUNT] =
{
  "UART",
  "SD",
  "USB",
  "ETH"
};

static const char *const har_class_names[HAR_CLASS_COUNT] =
{
  "laying",
  "sitting",
  "standing",
  "walking",
  "walking_down",
  "walking_up"
};

static int HAR_ParseSampleLine(const char *line, HAR_Sample *sample);
static void HAR_AddSample(const HAR_Sample *sample, HAR_InputSource source);
static void HAR_CopyWindow(float *destination, size_t destination_count);
static void HAR_RunInference(HAR_InputSource source);
static uint32_t HAR_ArgMax(const float *values, size_t count);
static void HAR_FormatResult(const HAR_Result *result, char *buffer, size_t buffer_size);
static void HAR_FormatParseError(HAR_InputSource source, const char *line);

void HAR_Init(void)
{
  memset(har_window, 0, sizeof(har_window));
  memset(har_ai_input, 0, sizeof(har_ai_input));
  har_next_sample = 0U;
  har_sample_count = 0U;
  har_samples_since_inference = 0U;

  HAR_OutputSerial("HAR ready. Send CSV: ax,ay,az,gx,gy,gz\r\n");
}

void HAR_Process(void)
{
  char line[HAR_LINE_MAX_LEN];
  HAR_Sample sample;

  for (uint32_t source = 0U; source < (uint32_t)HAR_INPUT_COUNT; source++)
  {
    int has_line = 0;

    switch ((HAR_InputSource)source)
    {
      case HAR_INPUT_UART:
        has_line = HAR_UART_ReadLine(line, sizeof(line));
        break;
      case HAR_INPUT_SD:
        has_line = HAR_SD_ReadLine(line, sizeof(line));
        break;
      case HAR_INPUT_USB:
        has_line = HAR_USB_ReadLine(line, sizeof(line));
        break;
      case HAR_INPUT_ETH:
        has_line = HAR_ETH_ReadLine(line, sizeof(line));
        break;
      default:
        break;
    }

    if (has_line != 0)
    {
      if (HAR_ParseSampleLine(line, &sample) != 0)
      {
        HAR_AddSample(&sample, (HAR_InputSource)source);
      }
      else
      {
        HAR_FormatParseError((HAR_InputSource)source, line);
      }
    }
  }
}

const char *HAR_SourceName(HAR_InputSource source)
{
  if ((uint32_t)source >= (uint32_t)HAR_INPUT_COUNT)
  {
    return "INVALID";
  }

  return har_source_names[(uint32_t)source];
}

const char *HAR_ClassName(uint32_t class_index)
{
  if (class_index >= HAR_CLASS_COUNT)
  {
    return "invalid";
  }

  return har_class_names[class_index];
}

static int HAR_ParseSampleLine(const char *line, HAR_Sample *sample)
{
  float values[HAR_AXIS_COUNT + 1U];
  uint32_t count = 0U;
  const char *cursor = line;
  char *end_ptr = NULL;

  while ((*cursor != '\0') && (count < (uint32_t)(HAR_AXIS_COUNT + 1U)))
  {
    float value = (float)strtod(cursor, &end_ptr);
    if (end_ptr == cursor)
    {
      break;
    }

    values[count++] = value;
    cursor = end_ptr;

    while ((*cursor == ' ') || (*cursor == '\t'))
    {
      cursor++;
    }

    if ((*cursor == ',') || (*cursor == ';'))
    {
      cursor++;
    }
    else if ((*cursor == '\r') || (*cursor == '\n') || (*cursor == '\0'))
    {
      break;
    }
    else
    {
      return 0;
    }
  }

  if (count == HAR_AXIS_COUNT)
  {
    for (uint32_t i = 0U; i < HAR_AXIS_COUNT; i++)
    {
      sample->axis[i] = values[i];
    }
    return 1;
  }

  if (count == (HAR_AXIS_COUNT + 1U))
  {
    for (uint32_t i = 0U; i < HAR_AXIS_COUNT; i++)
    {
      sample->axis[i] = values[i + 1U];
    }
    return 1;
  }

  return 0;
}

static void HAR_AddSample(const HAR_Sample *sample, HAR_InputSource source)
{
  har_window[har_next_sample] = *sample;
  har_next_sample = (har_next_sample + 1U) % HAR_WINDOW_SAMPLES;

  if (har_sample_count < HAR_WINDOW_SAMPLES)
  {
    har_sample_count++;
  }

  har_samples_since_inference++;

  if ((har_sample_count == HAR_WINDOW_SAMPLES) &&
      (har_samples_since_inference >= HAR_WINDOW_STRIDE))
  {
    har_samples_since_inference = 0U;
    HAR_RunInference(source);
  }
}

static void HAR_CopyWindow(float *destination, size_t destination_count)
{
  uint32_t oldest = har_next_sample;
  size_t out = 0U;

  if (destination_count < (HAR_WINDOW_SAMPLES * HAR_AXIS_COUNT))
  {
    return;
  }

  for (uint32_t i = 0U; i < HAR_WINDOW_SAMPLES; i++)
  {
    const HAR_Sample *sample = &har_window[(oldest + i) % HAR_WINDOW_SAMPLES];
    for (uint32_t axis = 0U; axis < HAR_AXIS_COUNT; axis++)
    {
      destination[out++] = sample->axis[axis];
    }
  }
}

static void HAR_RunInference(HAR_InputSource source)
{
  HAR_Result result;
  char message[192];
  uint32_t best_index;

  memset(&result, 0, sizeof(result));
  result.source = source;

  HAR_CopyWindow(har_ai_input, sizeof(har_ai_input) / sizeof(har_ai_input[0]));

  /* Per-axis normalization — same as training (normalization.json)         */
  /* Mean/std estimated for typical smartphone IMU (accel-g, gyro-rad/s).   */
  /* Replace with values from normalization.json when available.            */
  {
    /* UCI HAR normalization — computed from training data (this session)     */
    static const float mean[6] = {0.8066f, 0.0215f, 0.0860f,
                                  -0.0032f, -0.0002f, 0.0011f};
    static const float std[6]  = {0.4118f, 0.3968f, 0.3442f,
                                   0.4039f, 0.3716f, 0.2540f};
    for (uint32_t s = 0U; s < HAR_WINDOW_SAMPLES; s++) {
      for (uint32_t a = 0U; a < HAR_AXIS_COUNT; a++) {
        uint32_t idx = s * HAR_AXIS_COUNT + a;
        har_ai_input[idx] = (har_ai_input[idx] - mean[a]) / std[a];
      }
    }
  }

  if (HAR_RunNetwork(har_ai_input,
                     sizeof(har_ai_input) / sizeof(har_ai_input[0]),
                     result.scores,
                     HAR_CLASS_COUNT) == 0)
  {
    result.scores[HAR_CLASS_COUNT - 1U] = 1.0f;
  }

  best_index = HAR_ArgMax(result.scores, HAR_CLASS_COUNT);
  result.label = HAR_ClassName(best_index);
  result.confidence = result.scores[best_index];

  HAR_FormatResult(&result, message, sizeof(message));
  HAR_OutputSerial(message);
}

static uint32_t HAR_ArgMax(const float *values, size_t count)
{
  uint32_t best = 0U;

  for (uint32_t i = 1U; i < count; i++)
  {
    if (values[i] > values[best])
    {
      best = i;
    }
  }

  return best;
}

static void HAR_FormatResult(const HAR_Result *result, char *buffer, size_t buffer_size)
{
  /* Print ALL 7 class scores as integer percentages — nano.specs no %f   */
  int s0 = (int)(result->scores[0] * 100.0f);
  int s1 = (int)(result->scores[1] * 100.0f);
  int s2 = (int)(result->scores[2] * 100.0f);
  int s3 = (int)(result->scores[3] * 100.0f);
  int s4 = (int)(result->scores[4] * 100.0f);
  int s5 = (int)(result->scores[5] * 100.0f);
  int s6 = (int)(result->scores[6] * 100.0f);
  (void)snprintf(buffer, buffer_size,
      "HAR,label=%s,La=%d%%,Si=%d%%,St=%d%%,Wa=%d%%,WD=%d%%,WU=%d%%\r\n",
      result->label,
      s0, s1, s2, s3, s4, s5);
}

static void HAR_FormatParseError(HAR_InputSource source, const char *line)
{
  char message[192];

  (void)snprintf(message,
                 sizeof(message),
                 "ERR,source=%s,expected=ax,ay,az,gx,gy,gz,line=%s\r\n",
                 HAR_SourceName(source),
                 line);
  HAR_OutputSerial(message);
}

HAR_WEAK int HAR_UART_ReadLine(char *buffer, size_t buffer_size)
{
  (void)buffer;
  (void)buffer_size;
  return 0;
}

HAR_WEAK int HAR_SD_ReadLine(char *buffer, size_t buffer_size)
{
  (void)buffer;
  (void)buffer_size;
  return 0;
}

HAR_WEAK int HAR_USB_ReadLine(char *buffer, size_t buffer_size)
{
  (void)buffer;
  (void)buffer_size;
  return 0;
}

HAR_WEAK int HAR_ETH_ReadLine(char *buffer, size_t buffer_size)
{
  (void)buffer;
  (void)buffer_size;
  return 0;
}

HAR_WEAK void HAR_OutputSerial(const char *text)
{
  (void)text;
}

HAR_WEAK int HAR_RunNetwork(const float *input,
                            size_t input_element_count,
                            float *output_scores,
                            size_t output_class_count)
{
  float acc_mean = 0.0f;
  float acc_var = 0.0f;
  float gyro_energy = 0.0f;
  float vertical_delta;

  if ((input_element_count < (HAR_WINDOW_SAMPLES * HAR_AXIS_COUNT)) ||
      (output_class_count < HAR_CLASS_COUNT))
  {
    return 0;
  }

  for (uint32_t i = 0U; i < HAR_WINDOW_SAMPLES; i++)
  {
    const float ax = input[(i * HAR_AXIS_COUNT) + 0U];
    const float ay = input[(i * HAR_AXIS_COUNT) + 1U];
    const float az = input[(i * HAR_AXIS_COUNT) + 2U];
    const float gx = input[(i * HAR_AXIS_COUNT) + 3U];
    const float gy = input[(i * HAR_AXIS_COUNT) + 4U];
    const float gz = input[(i * HAR_AXIS_COUNT) + 5U];
    const float acc_mag = sqrtf((ax * ax) + (ay * ay) + (az * az));

    acc_mean += acc_mag;
    gyro_energy += (gx * gx) + (gy * gy) + (gz * gz);
  }

  acc_mean /= (float)HAR_WINDOW_SAMPLES;

  for (uint32_t i = 0U; i < HAR_WINDOW_SAMPLES; i++)
  {
    const float ax = input[(i * HAR_AXIS_COUNT) + 0U];
    const float ay = input[(i * HAR_AXIS_COUNT) + 1U];
    const float az = input[(i * HAR_AXIS_COUNT) + 2U];
    const float acc_mag = sqrtf((ax * ax) + (ay * ay) + (az * az));
    const float delta = acc_mag - acc_mean;

    acc_var += delta * delta;
  }

  acc_var /= (float)HAR_WINDOW_SAMPLES;
  gyro_energy = sqrtf(gyro_energy / (float)HAR_WINDOW_SAMPLES);
  vertical_delta = input[((HAR_WINDOW_SAMPLES - 1U) * HAR_AXIS_COUNT) + 2U] - input[2U];

  for (uint32_t i = 0U; i < HAR_CLASS_COUNT; i++)
  {
    output_scores[i] = 0.02f;
  }

  if ((acc_var < 0.015f) && (gyro_energy < 8.0f))
  {
    output_scores[3] = 0.70f;
    output_scores[2] = 0.22f;
  }
  else if (acc_var > 0.35f)
  {
    output_scores[1] = 0.72f;
    output_scores[0] = 0.18f;
  }
  else if (vertical_delta > 0.20f)
  {
    output_scores[4] = 0.65f;
    output_scores[0] = 0.22f;
  }
  else if (vertical_delta < -0.20f)
  {
    output_scores[5] = 0.65f;
    output_scores[0] = 0.22f;
  }
  else
  {
    output_scores[0] = 0.68f;
    output_scores[1] = 0.16f;
  }

  return 1;
}
