// @Author Huang Yilong
// @Email yilong.huang@outlook.com
#pragma once

class noncopyable {
    protected:
        noncopyable() {}
        ~noncopyable() {}

    private:
        noncopyable(const noncopyable&);
        const noncopyable& operator=(const noncopyable&);
};