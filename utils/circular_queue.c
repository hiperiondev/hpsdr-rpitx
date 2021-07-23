/*
 * circular_queue.c
 *
 *  Created on: 19 jul. 2021
 *      Author: egonzalez
 */

#include <stdio.h>
#include <stdbool.h>

static int front = -1, rear = -1, n;

void cq_init(int len) {
    n = len;
}

bool cq_isfull(void) {
    if (front == (rear + 1) % n || front > rear) {
        return true;
    } else {
        return false;
    }
}

bool cq_isempty(void) {
    if (rear == -1) {
        return true;
    } else {
        return false;
    }
}

bool cq_enqueue(double *queue, double val) {
    if (cq_isfull() == 1) {
        //printf("Queue is full...");
        return false;
    } else if (cq_isempty() == true) {
        front++;
        rear++;
        queue[rear] = val;
    } else {
        rear = (rear + 1) % n;
        queue[rear] = val;
    }
    return true;
}

bool cq_dequeue(double *queue, double *val) {
    int e;

    if (cq_isempty() == true) {
        //printf("queue is empty...");
        *val = -1;
        return false;
    } else {
        e = queue[front];
        if (front == rear) {
            front = -1;
            rear = -1;
        } else {
            front = (front + 1) % n;
        }
        *val = e;
    }
    return true;
}

int cq_dequeue_buf(double *queue, double *buff, int n) {
    int cnt;

    for (cnt = 0; cnt < n; cnt++) {
        if (!cq_dequeue(queue, buff)) {
            break;
        }
    }

    return cnt;
}

void cq_display(double *queue) {
    int y = front, z = rear;
    if (y == -1) {
        printf("queue is empty");
    } else {
        printf("The elements are : ");
        do {
            printf("%f\t", queue[y]);
            y = (y + 1) % n;
        } while (y != z || y < rear);
        printf("%f\t", queue[y]);
    }
}

void cq_peek(double *queue) {
    if (front == -1) {
        printf("queue is empty");
    } else {
        printf("The peek element is :");
        printf("%f", queue[front]);
    }
}

void cq_peep(double *queue) {
    int y;
    int p;
    printf("Enter the position : ");
    scanf("%d", &p);
    y = front;
    while (y == p - 1) {
        y = (y + 1) % n;
    }
    printf("The element found at position %d is %f ", p, queue[y]);
}

int cq_size(void) {
    int count = 0, y = front, z = rear;
    if (front == -1) {
        return 0;
    } else {
        while (y != z) {
            count++;
            y = (y + 1) % n;
        }
        count++;
        return count;
    }
}
