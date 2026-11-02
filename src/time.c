#include <sys/time.h>
#include "time.h"

long long timeInMilliseconds(){
    struct timeval tv;
    
    gettimeofday(&tv, 0);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}