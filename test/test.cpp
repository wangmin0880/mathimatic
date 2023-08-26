/*
* This times how long it takes to run the mathimatic function.
* remember: Do not waste time.
*
* initial this file to save time, as future need remove to a special folder.
*                                                    ---wangmin0880@163.com
*/

#include "stdio.h"
#include "time.h"
#include "string.h"

#include "area.h"
#include "test.h"


double calculate_time_spend_nsec(area_type type)
{
    struct timespec tp_begin;
    struct timespec tp_end;
    double syscall_using_time;
    double cost;

    memset(&tp_begin, 0, sizeof(struct timespec));
    memset(&tp_end, 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC,&tp_begin);
    clock_gettime(CLOCK_MONOTONIC,&tp_end);

    syscall_using_time = tp_end.tv_nsec - tp_begin.tv_nsec;
    /* syscall should not touch second level cost */
    if(tp_end.tv_sec != tp_begin.tv_sec){
        syscall_using_time += (tp_end.tv_sec - tp_begin.tv_sec) * 1000000000;
        printf("Warning: syscall [clock_gettime] cost too much time.\n");
    }

    if(type == CIRCLE)
    {
        clock_gettime(CLOCK_MONOTONIC,&tp_begin);
        double circle_area = math_circle_area(5);
        clock_gettime(CLOCK_MONOTONIC,&tp_end);
    }
    else if(type == ANNULUS)
    {
        clock_gettime(CLOCK_MONOTONIC,&tp_begin);
        double annulus_area = math_annulus_area(8,5);
        clock_gettime(CLOCK_MONOTONIC,&tp_end);
    }
    else if(type == ELLIPSE)
    {
         clock_gettime(CLOCK_MONOTONIC,&tp_begin);
         double ellipse_area = math_ellipse_area(8,5);
         clock_gettime(CLOCK_MONOTONIC,&tp_end);
    }

    cost = (tp_end.tv_sec - tp_begin.tv_sec) * 1000000000 + tp_end.tv_nsec - tp_begin.tv_nsec;
    /*
    due to syscall not using same time. ;-)
    Plan to use system CLOCK it self by calling this at kernel mode in the future.
    cost -= syscall_using_time;
    */
    return cost;
}
