// @Author Huang Yilong
// @Email yilong.huang@outlook.com
#pragma once
#include <string>
#include "noncopyable.h"

class AppendFile : noncopyable {
    public:
        explicit AppendFile(std::string filename);
        ~AppendFile();
        //append会向文件写
        void append(const char *logline, const size_t len);
        void flush();
    
    private:
        size_t write(const char *logline, size_t len);
        FILE *fp_;
        char buffer_[64 * 1024];
};