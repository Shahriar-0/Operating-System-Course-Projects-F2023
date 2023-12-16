#ifndef PFILTER_HPP_INCLUDE
#define PFILTER_HPP_INCLUDE

#include "bmp.hpp"
#include "filter.hpp"
#include "threading/thread.hpp"

namespace pfilter {

using FilterFunc = void (*)(bmp::BmpView);

class FilterTask : public thread::Task {
public:
    FilterTask(bmp::BmpView view, FilterFunc filter)
        : view_(view), filter_(filter) {}

    void run() override {
        filter_(view_);
    }

private:
    bmp::BmpView view_;
    FilterFunc filter_;
};

class ReplaceTask : public thread::Task {
public:
    ReplaceTask(bmp::BmpView subBmp, bmp::BmpView src)
        : subBmp_(subBmp), src_(src) {}

    void run() override {
        bmp::BmpView::replace(subBmp_, src_);
    }

private:
    bmp::BmpView subBmp_;
    bmp::BmpView src_;
};

} // namespace pfilter

#endif // PFILTER_HPP_INCLUDE
