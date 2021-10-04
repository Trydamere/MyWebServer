# MyWebServer

  



## 项目介绍

  



本项目是C++11编写的轻量级Web服务器，该项目使用状态机解析GET和POST请求，可处理静态资源，支持HTTP长连接，并实现了异步日志，记录服务器运行状态。

系统测试中，对每个模块进行单元测试，使用webbench对系统进行压力测试，测试结果显示服务器可以实现上万并发连接数据交换。

  



## 技术点

  



- 使用Epoll边沿触发的IO多路复用技术，非阻塞IO，使用Reactor模式
- 使用多线程充分利用多核CPU，并使用线程池避免线程频繁创建销毁的开销
- 使用基于小根堆的定时器关闭超时请求
- 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
- 使用eventfd实现了线程的异步唤醒
- 使用双缓冲区技术实现了简单的异步日志系统
- 为减少内存泄漏的可能，使用智能指针等RAII机制
- 使用状态机解析了HTTP请求,支持管线化
- 支持优雅关闭连接  

  



## 项目开发过程

  



### 完成base部分

  



#### 完成noncopyable

  



实现：将拷贝构造函数和拷贝运算符私有

  



#### 完成MutexLock MutexLockGuard

  



实现：对mutex操作进行封装

  



#### 完成condition(条件变量)

  



实现：对pthread_cond进行封装

  



#### 完成CountDownLatch

  



使用互斥量，条件变量，计数实现

> 倒计时（CountDownLatch）是一种常用且易用的同步手段。它主要有两种用途：
> 1.主线程发起多个子线程，等这些子线程各自都完成一定的任务之后，主线程才继续执行。通常用于主线程等待多个子线程完成初始化。（主线程等多线程完成）
> 2.主线程发起多个子线程，子线程都等待主线程，主线程完成其他一些任务之后通知所有子线程开始执行。通常用于多个子线程等待主线程发出“起跑”命令。（多线程等主线程完成）

  



#### 完成CurrentThread

  



__thread变量是每个线程有一份独立实体，各个线程的变量值互不干扰。

  



#### 完成Thread

  



```
进程pid: getpid()                 
线程tid: pthread_self()     //进程内唯一，但是在不同进程则不唯一。
线程pid: syscall(SYS_gettid)     //系统内是唯一的
```

thread::start()方法中assert(tid_>0); 判断线程tid合法

  



#### 完成FileUtil

  



O_CLOEXEC作用：文件描述符在fork或者fork+exec之后默认打开，使用这个标志位可以关闭文件描述符

  



#### 完成LogFile

  



输出到文件

  



#### 完成Logstream

  



参考muduo

  



#### 完成AsyncLogging

  



参考muduo

  



#### 完成logging

  



参考muduo

  



#### 完成单元测试

  



#### 完成util

  



进程接收到信号SIGPIPE，默认行为是终止当前进程。

SIGPIPE是一个进程尝试写一个关闭写或不连接的socket

处理：

```
sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
```

  



TCP_NODELAY选项是用来控制是否开启Nagle算法

  



SO_LINGER选项用来设置延迟关闭的时间，等待套接字发送缓冲区中的数据发送完成。没有设置该选项时，在调用close()后，在发送完FIN后会立即进行一些清理工作并返回。如果设置SO_LINGER选项，并且等待时间为正值，则在清理之前会等待一段时间。

  



#### 完成Timer

  



#### 完成HttpData

  



解析HTTP请求过程：

解析请求行

第一步：解析方法 通过查找方法名定位

第二步：解析文件名 找“/"位置，没有”/“,则返回index；看”/"位置和空格位置的距离，如果小于0，解析错误，如果大于1，找到?的位置，之间就是文件名，如果小于1，则返回index

第三步：解析版本号 找”/“位置，如果找不到，返回解析错误，如果”/“位置和起始位置小于3，(不是HTTP版本号)返回解析错误，查看"/"后面的数字，对应1.0和1.1

解析请求头

H_START : 起始状态

H_KEY: 键解析状态,遇到: 转移H_COLON

H_COLON: 检查: 后的空格，状态转移到H_SPACE_AFTER_COLON

H_SPACE_AFTER_COLON: 状态转移到H_VALUE

H_VALUE: 遇到\r状态转移到H_CR

H_CR: 遇到\n, 解析出key和value，插入map中

H_LF: 遇到\r，状态转移H_END_CR，否则转移H_KEY

H_END_CR: 遇到\n，状态转移到H_END_LF

H_END_LF: 更新状态，退出

  



如果状态不是H_END_LF，则需要再次读入

  



响应analysisRequest 

Connection: Keep-Alive

Keep-Alive: timeout= 30

Content-type :text/html

Content-Length : sizeof sbuf

传文件，打开文件描述符，通过mmap将文件映射到内存

  



#### 完成Channel

  



参考muduo

  



#### 完成Epoll

  



参考muduo

  



#### 完成EventLoop

  



参考muduo

  



#### 完成EventLoopThread

  



参考muduo

  



#### 完成EventLoopThreadPool

  



EventLoopthread 的线程池

  



#### 完成Server.h

  



## 项目困难：

  



大文件传输：当请求小文件，也就是调用一次writev函数就可以将数据全部发送出去的时候，不会报错。

一旦请求服务器文件较大文件时，需要多次调用writev函数，便会出现文件无法显示。

对数据传输过程分析后，每次传输后不会自动偏移文件指针和传输长度，还会按照原有指针和原有长度发送数据，修改如下：

- 报文消息报头较小，第一次传输后，需要更新传输长度，传输长度置成0
- 每次传输后都要更新下次传输的文件起始位置和长度
