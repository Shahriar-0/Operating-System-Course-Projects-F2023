#ifndef PFILTER_HPP_INCLUDE
#define PFILTER_HPP_INCLUDE

#include "bmp.hpp"
#include "filter.hpp"
#include "thread.hpp"

namespace pfilter {

using FilterFunc = void (*)(BMP24::BMP_View);

class FilterTask : public thread::Task {
public:
    FilterTask(BMP24::BMP_View view, FilterFunc filter)
        : view_(view), filter_(filter) {}

    void run() override {
        filter_(view_);
    }

private:
    BMP24::BMP_View view_;
    FilterFunc filter_;
};

class ReplaceTask : public thread::Task {
public:
    ReplaceTask(BMP24::BMP_View subBmp, BMP24::BMP_View src)
        : subBmp_(subBmp), src_(src) {}

    void run() override {
        BMP24::BMP_View::replace(subBmp_, src_);
    }

private:
    BMP24::BMP_View subBmp_;
    BMP24::BMP_View src_;
};

} // namespace pfilter

#endif // PFILTER_HPP_INCLUDE
