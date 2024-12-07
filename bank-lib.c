#include "bank-lib.h"

void GetInput(int *days, double *total, int *closetime, int *interval_lo, int *interval_up,
              int *durtime_lo, int *durtime_up, double *amount_lo, double *amount_up) {
    do {
        printf("Please input the days of simulation (>=1): ");
        scanf_s("%d", days);
    } while (*days < 1);

    do {
        printf("Please input the daily total amount of money in the bank (>0): ");
        scanf_s("%lf", total);
    } while (*total <= 0);

    do {
        printf("Please input the closing time of the bank (>0): ");
        scanf_s("%d", closetime);
    } while (*closetime <= 0);

    do {
        printf("Please input the interval time range (lo up, both >=0, up>=lo): ");
        scanf_s("%d %d", interval_lo, interval_up);
    } while (*interval_lo < 0 || *interval_up < 0 || *interval_up < *interval_lo);

    do {
        printf("Please input the duration time range (lo up, both >0, up>=lo): ");
        scanf_s("%d %d", durtime_lo, durtime_up);
    } while (*durtime_lo <= 0 || *durtime_up <= 0 || *durtime_up < *durtime_lo);

    do {
        printf("Please input the banking amount range (lo up, both >0, up>=lo): ");
        scanf_s("%lf %lf", amount_lo, amount_up);
    } while (*amount_up < *amount_lo || *amount_lo <= 0 || *amount_up <= 0);
}

double random_sign(void) {
    // 使用 rand() 生成 0 或 1
    return (rand() % 2 == 0) ? 1.0 : -1.0;
}

void InitQueue(Queue *Q) {
    Q->base = (CustNode *)malloc(CHUNKSIZE * sizeof(CustNode));
    if (!Q->base) {
        printf("\033[47;31m Out of Memory! \033[0m\n");
        exit(1);
    }
    Q->chunksize = CHUNKSIZE;
    Q->front = Q->rear = Q->length = 0;
}

void DestroyQueue(Queue *Q) {
    free(Q->base);
    Q->base = NULL;
    Q->front = Q->rear = Q->length = 0;
}

CustNode EnQueue(Queue *Q, CustNode e) {
    if (Q->length == Q->chunksize) {
        // printf("\033[47;31m Queue is Full! \033[0m\n");
        // 队列满，扩容
        CustNode *newbase = (CustNode *)realloc(
            Q->base, (CHUNKSIZE + Q->chunksize) * sizeof(CustNode));
        if (!newbase) {
            free(Q->base);
            printf("\033[47;31m Out of Memory! \033[0m\n");
            exit(1);
        }
        Q->base = newbase;
        Q->chunksize += CHUNKSIZE;
        if (Q->rear < Q->front) {
            for (int i = 0; i < Q->rear; i++) {
                Q->base[Q->chunksize - CHUNKSIZE + i] = Q->base[i];
            }
            Q->rear += Q->chunksize;
        }
    }
    Q->base[Q->rear] = e;
    Q->rear = (Q->rear + 1) % Q->chunksize;
    Q->length++;
    return e;
}

CustNode DeQueue(Queue *Q) {
    if (Q->length == 0) {
        printf("\033[47;31m Queue is Empty! \033[0m\n");
        CustNode empty = {0};
        return empty;
    }
    CustNode e = Q->base[Q->front];
    Q->front = (Q->front + 1) % Q->chunksize;
    Q->length--;
    return e;
}

CustNode *InitList() {
    CustNode *eventlist = (CustNode *)malloc(MAX * sizeof(CustNode));
    if (!eventlist) {
        printf("\033[47;31m Out of Memory! \033[0m\n");
        exit(1);
    }
    eventlist[0].next = 0;
    for (int i = 1; i < MAX; i++) {
        eventlist[i].next = -1;
        eventlist[i].arrtime = 0;
        eventlist[i].amount = 0;
        eventlist[i].durtime = 0;
        eventlist[i].leavetime = -1;
        eventlist[i].completed = false;
    }
    return eventlist;
}

void DestroyList(CustNode *eventlist) {
    free(eventlist);
}

// 将新业务插入到eventlist中并按arrtime升序
void InsertToList(CustNode *eventlist, CustNode new_cust) {
    int ptr;
    for (ptr = 0; ptr < MAX; ptr++) {
        if (eventlist[ptr].next == -1) break;
    }
    if (ptr == MAX) {
        printf("\033[47;31m eventlist is Full! \033[0m\n");
        return;
    }
    eventlist[ptr] = new_cust;

    // 排序,找到第一个比新业务arrtime大的业务，插入到其前面；若表为空，直接插入；若表中所有业务的arrtime都比新业务小，直接插入到表尾
    int cur = 0;
    while (eventlist[cur].next != 0 && eventlist[eventlist[cur].next].arrtime <= new_cust.arrtime) {
        cur = eventlist[cur].next;
    }
    eventlist[ptr].next = eventlist[cur].next;
    eventlist[cur].next = ptr;
}

CustNode *GenerateList(CustNode *eventlist,
                       int closetime,
                       int interval_lo,
                       int interval_up,
                       int durtime_lo,
                       int durtime_up,
                       double amount_lo,
                       double amount_up) {
    CustNode new_cust = {0, false, 0, 0.0, 0, -1};
    int time_now = 0;
    int interval;
    while (1) {
        new_cust.arrtime = time_now;
        new_cust.durtime =
            rand() % (durtime_up - durtime_lo + 1) + durtime_lo;
        new_cust.amount =
            random_sign() * rand() / RAND_MAX * (amount_up - amount_lo) + amount_lo;

        InsertToList(eventlist, new_cust);

        interval = rand() % (interval_up - interval_lo + 1) + interval_lo;
        time_now += interval;
        if (time_now >= closetime) break;
    }
    return eventlist;
}

void DeleteFromList(CustNode *eventlist, int fore) {
    int cur = eventlist[fore].next;
    eventlist[fore].next = eventlist[cur].next;
    eventlist[cur].next = -1;
}

void CleanList(CustNode *eventlist) {
    int fore = 0;
    while (eventlist[fore].next) {
        if (eventlist[eventlist[fore].next].completed == true)
            DeleteFromList(eventlist, fore);
        else
            fore = eventlist[fore].next;
    }
}

void CompleteList(CustNode *eventlist, CustNode cust) {
    int cur = 0;
    while (1) {
        if (eventlist[cur].arrtime == cust.arrtime &&
            eventlist[cur].amount == cust.amount &&
            eventlist[cur].durtime == cust.durtime) {
            eventlist[cur].completed = true;
            return;
        }
        cur = eventlist[cur].next;
        if (cur == 0) break;
    }
}

// eventlist下标从1开始，从上次的指针继续向下查找一个，最后返回查找结果
CustNode ExtractList(CustNode *eventlist, bool mode, int *cur) {
    CustNode cust = {0};
    if (!eventlist || !cur) {
        return cust;
    }
    if (mode == WRITEABLE) {
        *cur = eventlist[*cur].next;
        cust = eventlist[*cur];
    } else {
        cust = eventlist[eventlist[*cur].next];
    }
    return cust;
}

void PrintList(CustNode *eventlist) {
    int cur = 0;
    while (eventlist[cur].next) {
        printf("    arrtime:%d, amount:%lf, durtime:%d, leavetime:%d, completed:%d\n",
               eventlist[eventlist[cur].next].arrtime,
               eventlist[eventlist[cur].next].amount,
               eventlist[eventlist[cur].next].durtime,
               eventlist[eventlist[cur].next].leavetime,
               eventlist[eventlist[cur].next].completed);
        cur = eventlist[cur].next;
    }
}

void PrintQueue(Queue Q) {
    int cur = Q.front;
    for (int i = 0; i < Q.length; i++) {
        printf("    arrtime:%d, amount:%lf, durtime:%d, leavetime:%d\n",
               Q.base[cur].arrtime, Q.base[cur].amount, Q.base[cur].durtime, Q.base[cur].leavetime);
        cur = (cur + 1) % Q.chunksize;
    }
}
