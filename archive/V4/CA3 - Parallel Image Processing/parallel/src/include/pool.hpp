#ifndef POOL_HPP
#define POOL_HPP

#include "thread.hpp"

constexpr int THREAD_COUNT = 8;
extern thread::Pool pool;

#endif // POOL_HPP