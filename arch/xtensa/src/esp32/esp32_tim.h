/****************************************************************************
 * arch/xtensa/src/esp32/esp32_tim.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __ARCH_XTENSA_SRC_ESP32_ESP32_TIM_H
#define __ARCH_XTENSA_SRC_ESP32_ESP32_TIM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Helpers ******************************************************************/

#define ESP32_TIM_START(d)                         ((d)->ops->start(d))
#define ESP32_TIM_STOP(d)                          ((d)->ops->stop(d))
#define ESP32_TIM_CLEAR(d)                         ((d)->ops->clear(d))
#define ESP32_TIM_SETMODE(d, m)                    ((d)->ops->setmode(d, m))
#define ESP32_TIM_SETPRE(d, p)                     ((d)->ops->setpre(d, p))
#define ESP32_TIM_GETCTR(d, v)                     ((d)->ops->getcounter(d, v))
#define ESP32_TIM_SETCTR(d, v)                     ((d)->ops->setcounter(d, v))
#define ESP32_TIM_RLD_NOW(d)                       ((d)->ops->reloadnow(d))
#define ESP32_TIM_GETALRVL(d, v)                   ((d)->ops->getalarmvalue(d, v))
#define ESP32_TIM_SETALRVL(d, v)                   ((d)->ops->setalarmvalue(d, v))
#define ESP32_TIM_SETALRM(d, e)                    ((d)->ops->setalarm(d, e))
#define ESP32_TIM_SETARLD(d, e)                    ((d)->ops->setautoreload(d, e))
#define ESP32_TIM_SETISR(d, hnd, arg)              ((d)->ops->setisr(d, hnd, arg))
#define ESP32_TIM_ENABLEINT(d)                     ((d)->ops->enableint(d))
#define ESP32_TIM_DISABLEINT(d)                    ((d)->ops->disableint(d))
#define ESP32_TIM_ACKINT(d)                        ((d)->ops->ackint(d))

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Timer mode */

enum esp32_tim_mode_e
{
  ESP32_TIM_MODE_DOWN,
  ESP32_TIM_MODE_UP,
};

/* ESP32 TIM device */

struct esp32_tim_dev_s
{
  struct esp32_tim_ops_s *ops;
};

/* ESP32 TIM ops */

/* This is a struct containing the pointers to the timer operations */

struct esp32_tim_ops_s
{
  /* Timer tasks */

  CODE void (*start)(FAR struct esp32_tim_dev_s *dev);
  CODE void (*stop)(FAR struct esp32_tim_dev_s *dev);
  CODE void (*clear)(FAR struct esp32_tim_dev_s *dev);

  /* Timer operations */

  CODE void (*setmode)(FAR struct esp32_tim_dev_s *dev, uint8_t mode);
  CODE void (*setpre)(FAR struct esp32_tim_dev_s *dev, uint16_t pre);
  CODE void (*getcounter)(FAR struct esp32_tim_dev_s *dev, uint64_t *value);
  CODE void (*setcounter)(FAR struct esp32_tim_dev_s *dev, uint64_t value);
  CODE void (*reloadnow)(FAR struct esp32_tim_dev_s *dev);
  CODE void (*getalarmvalue)(FAR struct esp32_tim_dev_s *dev,
                            uint64_t *value);
  CODE void (*setalarmvalue)(FAR struct esp32_tim_dev_s *dev,
                             uint64_t value);
  CODE void (*setalarm)(FAR struct esp32_tim_dev_s *dev, bool enable);
  CODE void (*setautoreload)(FAR struct esp32_tim_dev_s *dev, bool enable);

  /* Timer interrupts */

  CODE int (*setisr)(FAR struct esp32_tim_dev_s *dev, xcpt_t handler,
                     FAR void * arg);
  CODE void (*enableint)(FAR struct esp32_tim_dev_s *dev);
  CODE void (*disableint)(FAR struct esp32_tim_dev_s *dev);
  CODE void (*ackint)(FAR struct esp32_tim_dev_s *dev);
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

FAR struct esp32_tim_dev_s *esp32_tim_init(int timer);
void esp32_tim_deinit(FAR struct esp32_tim_dev_s *dev);
FAR struct esp32_tim_dev_s *esp32_tim0_init(void);

#endif /* __ARCH_XTENSA_SRC_ESP32_ESP32_TIM_H */
