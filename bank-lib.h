#ifndef BANKINGSIMULATOR__BANK_LIB_H_
#define BANKINGSIMULATOR__BANK_LIB_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHUNKSIZE 10
#define MAX 100
#define WRITEABLE 1
#define READ_ONLY 0

typedef struct {
    int next;        // 下一个节点的下标
    bool completed;  // 是否已经完成业务

    int arrtime;    // 到达时间
    double amount;  // 业务金额
    int durtime;    // 业务耗时
    int leavetime;  // 离开时间
} CustNode;         // 客户

typedef struct {
    CustNode Cust;  // 当前柜台的客户
    bool isbusy;    // 柜台是否忙碌
} CounterNode;      // 柜台

typedef struct {
    int front;       // 队头指针
    int rear;        // 队尾指针
    int length;      // 队列长度
    int chunksize;   // 队列容量
    CustNode *base;  // 队列基址
} Queue;             // 循环队列

void GetInput(int *days, double *total, int *closetime, int *interval_lo, int *interval_up,
              int *durtime_lo, int *durtime_up, double *amount_lo, double *amount_up);

void InitQueue(Queue *Q);

CustNode EnQueue(Queue *Q, CustNode e);

CustNode DeQueue(Queue *Q);

CustNode *InitList();

void DestroyQueue(Queue *Q);

void CleanList(CustNode *eventlist);

void CompleteList(CustNode *eventlist, CustNode cust);

CustNode ExtractList(CustNode *eventlist, bool mode, int *cur);

void DestroyList(CustNode *eventlist);

void InsertToList(CustNode *eventlist, CustNode new_cust);

CustNode *GenerateList(CustNode *eventlist,
                       int closetime,
                       int interval_lo,
                       int interval_up,
                       int durtime_lo,
                       int durtime_up,
                       double amount_lo,
                       double amount_up);

void PrintList(CustNode *eventlist);

void PrintQueue(Queue Q);

#endif  // BANKINGSIMULATOR__BANK_LIB_H_
