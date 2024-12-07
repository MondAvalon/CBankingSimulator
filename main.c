#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bank-lib.h"

int main(void) {
    // 先根据前一天的eventlist的剩余初始化当日的eventlist，再根据输入的随机参数生成当日业务的arrtime、durtime、interval、amount，
    // 再根据规则模拟当日的业务流程，计算每个事件的leavetime，最后输出当日客户平均逗留时间

    double total = -1;                       // 初始时银行现存总资金
    int closetime = -1;                      // 营业停止时间
    int days = -1;                            // 模拟天数
    int durtime_up = -1, durtime_lo = -1;    // 业务耗时上下界
    double amount_up = -1, amount_lo = -1;     // 业务金额上下界
    int interval_up = -1, interval_lo = -1;  // 相邻客户到达时间间隔上下界

    GetInput(&days, &total, &closetime, &interval_lo, &interval_up, &durtime_lo, &durtime_up, &amount_lo, &amount_up);

    srand((unsigned) time(NULL));

    CustNode *eventlist = InitList();
    Queue Q1, Q2;  // 队列
    double avgtime_total = 0;

    for (int day = 1; day <= days; day++) {
        double total_daily = total;  // 重置当日总资金
        double avgtime_daily = 0;    // 平均逗留时间
        int cust_num = 0;            // 到达银行的客户数量
        // 生成当日的eventlist
        eventlist = GenerateList(eventlist,
                                 closetime,
                                 interval_lo,
                                 interval_up,
                                 durtime_lo,
                                 durtime_up,
                                 amount_lo,
                                 amount_up);

        printf("\nDay %d starts\n", day);
        // PrintList(eventlist);

        // 进行排队
        InitQueue(&Q1);
        InitQueue(&Q2);
        CounterNode counter = {0};  // 柜台
        int cur = 0;
        double last_total;
        int Switch = 1;
        ExtractList(NULL, WRITEABLE, &cur);
        for (int time_now = 0; time_now <= closetime; time_now++) {
            //            printf("time:%d  ", time_now);
            //            printf("counter status:%d arrtime:%d, amount:%lf, durtime:%d, leavetime:%d\n",
            //                   counter.isbusy,
            //                   counter.Cust.arrtime,
            //                   counter.Cust.amount,
            //                   counter.Cust.durtime,
            //                   counter.Cust.leavetime);

            // 业务到达，进入队列1
            while (ExtractList(eventlist, READ_ONLY, &cur).arrtime == time_now) {
                printf("Arrive. time: %d\n", time_now);
                EnQueue(&Q1, ExtractList(eventlist, WRITEABLE, &cur));
                cust_num++;
            }

            // 柜台业务结束
            if (counter.isbusy == true && counter.Cust.leavetime == time_now) {
                if (counter.Cust.amount > 0) {
                    last_total = total_daily;
                }
                total_daily += counter.Cust.amount;
                counter.isbusy = false;
                CompleteList(eventlist, counter.Cust);
                avgtime_daily += time_now - counter.Cust.arrtime;
                printf("Complete. time:%d, amount:%lf, total_daily:%lf\n", time_now,
                       counter.Cust.amount, total_daily);
                // 逐个检查队列2客户是否能够进入柜台
                if (counter.Cust.amount > 0 && Q2.length > 0) {
                    CustNode check;  // 用于检查Q2
                    // 从队列2中找到第一个取款金额小于等于总资金的业务
                    int i;
                    for (i = 0; i < Q2.length && total_daily > last_total; i++) {  // && total_daily > last_total
                        check = DeQueue(&Q2);
                        if (-check.amount <= total_daily) {  // 取款金额小于等于总资金，开始业务
                            Switch = 2;
                            counter.Cust = check;
                            counter.isbusy = true;
                            counter.Cust.leavetime =
                                time_now + counter.Cust.durtime;
                            printf("Start New from Q2. time:%d, amount:%lf, total_daily:%lf\n",
                                   time_now, counter.Cust.amount, total_daily);
                        } else
                            EnQueue(&Q2, check);  // 不符条件的重新进入队列2
                    }
                    if (i == Q2.length) Switch = 1;
                }
                if (total_daily <= last_total) Switch = 1;
            }

            // 队列1 业务进入柜台
            if (Switch == 1 && counter.isbusy == false && Q1.length > 0) {
                counter.Cust = DeQueue(&Q1);
                counter.isbusy = true;
                counter.Cust.leavetime = time_now + counter.Cust.durtime;
                printf("Start New from Q1. time:%d, amount:%lf, leave time: %d, total_daily: % lf\n ",
                       time_now, counter.Cust.amount, counter.Cust.leavetime, total_daily);

                // 取款金额大于总资金，进入队列Q2等候
                while (-counter.Cust.amount > total_daily) {
                    EnQueue(&Q2, counter.Cust);
                    counter.isbusy = false;
                    printf("Fallback to Q2. time:%d, amount:%lf, total_daily:%lf\n",
                           time_now, counter.Cust.amount, total_daily);
                    // 立即开始处理Q1的业务（如有)
                    if (Q1.length <= 0) break;
                    counter.Cust = DeQueue(&Q1);
                    counter.isbusy = true;
                    counter.Cust.leavetime =
                        time_now + counter.Cust.durtime;
                    printf("Start New from Q1. time:%d, amount:%lf, leave time: %d, total_daily: % lf\n ",
                           time_now, counter.Cust.amount, counter.Cust.leavetime, total_daily);
                }
            }

            if (Switch == 2 && counter.isbusy == false && Q2.length > 0) {
                CustNode check = DeQueue(&Q2);
                counter.Cust = check;
                counter.isbusy = true;
                counter.Cust.leavetime =
                    time_now + counter.Cust.durtime;
                printf("Start New from Q2. time:%d, amount:%lf, total_daily:%lf\n",
                       time_now, counter.Cust.amount, total_daily);
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
    free(eventlist);
    return 0;
}