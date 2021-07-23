/*
 * circular_queue.h
 *
 *  Created on: 19 jul. 2021
 *      Author: egonzalez
 */

#ifndef CIRCULAR_QUEUE_H_
#define CIRCULAR_QUEUE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  void cq_init(int len);
  bool cq_isfull(void);
  bool cq_isempty(void);
  bool cq_enqueue(double *queue, double val);
double cq_dequeue(double *queue);
   int cq_dequeue_buf(double *queue, double *buff, int n);
  void cq_display(double *queue);
  void cq_peek(double *queue);
  void cq_peep(double *queue);
   int cq_size(void);

#ifdef __cplusplus
}
#endif

#endif /* CIRCULAR_QUEUE_H_ */
