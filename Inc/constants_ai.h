/**
  ******************************************************************************
  * @file    constants_ai.h
  * @author  X-CUBE-AI C code generator
  * @brief   AI constants definitions for network_3
  ******************************************************************************
  */

#ifndef __CONSTANTS_AI_H
#define __CONSTANTS_AI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Network_3 model: har_model.tflite (float32, 1016 params)                   */
/* Input:  128 x 6 (H x CH), 3.00 KB, f32                                    */
/* Output: 7 classes, 28 B, f32 + softmax                                     */
/* MACC:   76,017                                                              */
/* Weights (ro): 4,064 B   |  Activations (rw): 7,288 B                       */
/* RAM total:    7,288 B                                                       */

#define AI_NETWORK_3_DATA_ACTIVATIONS_SIZE  25216

#ifdef __cplusplus
}
#endif

#endif /* __CONSTANTS_AI_H */
