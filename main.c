#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "bank-lib.h"

int main(void) {
    // 先根据前一天的eventlist的剩余初始化当日的eventlist，再根据输入的随机参数生成当日业务的arrtime、durtime、interval、amount，
    // 再根据规则模拟当日的业务流程，计算每个事件的leavetime，最后输出当日客户平均逗留时间

    double total = 10000;   // 初始时银行现存总资金
    int closetime = 600;  // 营业停止时间
    int interval;        // 两业务arrtime间隔时间
    int days = 5;        // 模拟天数
    int durtime_up = 30, durtime_lo = 5;  // 上下界
    double amount_up = 10000, amount_lo = 10;
    int interval_up = 30, interval_lo = 0;

    printf("Please input the days of simulation: ");
    scanf("%d", &days);
    printf("Please input the daily total amount of money in the bank: ");
    scanf("%lf", &total);
    printf("Please input the closing time of the bank: ");
    scanf("%d", &closetime);
    printf("Please input the upper and lower bounds of the duration time: ");
    scanf("%d%d", &durtime_up, &durtime_lo);
    printf("Please input the upper and lower bounds of the amount of money: ");
    scanf("%lf%lf", &amount_up, &amount_lo);
    printf("Please input the upper and lower bounds of the interval time: ");
    scanf("%d%d", &interval_up, &interval_lo);

    srand((unsigned) time(NULL));

    CustNode *eventlist = InitList();
    Queue Q1, Q2;  // 队列
    double avgtime_total = 0;

    for (int day = 1; day <= days; day++) {
        // 生成eventlist
        double total_daily = total;  // 重置当日总资金
        int time_now = 0;
        double avgtime_daily = 0;  // 平均逗留时间
        int cust_num = 0;          // 客户数量
        CustNode new_cust = {0, false, 0, 0.0, 0, -1};
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
        printf("\nDay %d starts\n", day);
        //PrintList(eventlist);

        // 进行排队
        InitQueue(&Q1);
        InitQueue(&Q2);
        CounterNode counter = {0};  // 柜台
        int cur = 0;
        ExtractList(NULL, WRITEABLE, &cur);
        for (time_now = 0; time_now <= closetime; time_now++) {
//            printf("time:%d  ", time_now);
//            printf("counter status:%d arrtime:%d, amount:%lf, durtime:%d, leavetime:%d\n",
//                   counter.isbusy,
//                   counter.Cust.arrtime,
//                   counter.Cust.amount,
//                   counter.Cust.durtime,
//                   counter.Cust.leavetime);
            // 业务到达，进入队列1
            while (ExtractList(eventlist, READ_ONLY, &cur).arrtime == time_now) {
                EnQueue(&Q1, ExtractList(eventlist, WRITEABLE, &cur));
                cust_num++;
            }

            // 柜台业务结束
            if (counter.isbusy == true && counter.Cust.leavetime == time_now) {
                total_daily += counter.Cust.amount;
                counter.isbusy = false;
                CompleteList(eventlist, counter.Cust);
                avgtime_daily += time_now - counter.Cust.arrtime;
                printf("Complete. time:%d, amount:%lf, total_daily:%lf\n", time_now,
                       counter.Cust.amount, total_daily);
                // 逐个检查队列2客户是否能够进入柜台
                if (counter.Cust.amount > 0 && Q2.length > 0) {
                    CustNode check;
                    // 从队列2中找到第一个取款金额小于等于总资金的业务
                    for (int i = 0; i < Q2.length; i++) {
                        check = DeQueue(&Q2);
                        if (-check.amount <= total_daily) {  // 取款金额小于等于总资金
                            counter.Cust = check;
                            counter.isbusy = true;
                            counter.Cust.leavetime =
                                time_now + counter.Cust.durtime;
                            printf("Start New from Q2. time:%d, amount:%lf, total_daily:%lf\n",
                                   time_now, counter.Cust.amount, total_daily);
                            break;
                        } else {  // 不符条件的重新进入队列2
                            EnQueue(&Q2, check);
                        }
                    }
                }
            }

            // 队列1 业务进入柜台
            if (counter.isbusy == false && Q1.length > 0) {
                counter.Cust = DeQueue(&Q1);
                counter.isbusy = true;
                counter.Cust.leavetime = time_now + counter.Cust.durtime;
                printf("Start New form Q1. time:%d, amount:%lf, total_daily:%lf\n", time_now,
                       counter.Cust.amount, total_daily);

                // 取款金额大于总资金，进入队列2等候
                while (-counter.Cust.amount > total_daily) {
                    EnQueue(&Q2, counter.Cust);
                    counter.isbusy = false;
                    printf("Fallback to Q2. time:%d, amount:%lf, total_daily:%lf\n",
                           time_now, counter.Cust.amount, total_daily);
                    if (Q1.length > 0) {
                        counter.Cust = DeQueue(&Q1);
                        counter.isbusy = true;
                        counter.Cust.leavetime =
                            time_now + counter.Cust.durtime;
                        printf("Start New from Q1. time:%d, amount:%lf, total_daily: % lf\n ",
                               time_now, counter.Cust.amount, total_daily);
                    } else {
                        break;
                    }
                }
            }
//            printf("\nQ1:\n");
//            PrintQueue(Q1);
//            printf("\nQ2:\n");
//            PrintQueue(Q2);
        }
        // 停止营业，所有客户离开银行
        if (counter.isbusy == true) {
            avgtime_daily += closetime - counter.Cust.arrtime;
        }
        while (Q1.length > 0) {
            CustNode leave = DeQueue(&Q1);
            avgtime_daily += closetime - leave.arrtime;
        }
        while (Q2.length > 0) {
            CustNode leave = DeQueue(&Q2);
            avgtime_daily += closetime - leave.arrtime;
        }
        avgtime_daily /= (double) cust_num;
        avgtime_total += avgtime_daily;
        printf("\nDay %d ends, Average Time Daily: %lf, Last Daily Amount: %lf\n", day, avgtime_daily, total_daily);
        // 清除eventlist中已完成的业务
        CleanList(eventlist);
//        PrintList(eventlist);
    }
    avgtime_total /= (double) days;
    printf("\nAverage Time Total: %lf\n", avgtime_total);

    DestroyQueue(&Q1);
    DestroyQueue(&Q2);
    DestroyList(eventlist);
    return 0;
}