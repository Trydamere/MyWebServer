// @Author Huang Yilong
// @Email yilong.huang@outlook.com
#pragma once
#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"

// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后
// 外层的start才返回
class CountDownLatch : noncopyable {
    public:
        explicit CountDownLatch(int count); //倒数几次
        void wait();                        //等待计数值为0
        void countDown();                   //计数减1
    
    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
};