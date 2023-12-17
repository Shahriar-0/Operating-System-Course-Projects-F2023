#ifndef GENERIC_TASK_HPP_INCLUDE
#define GENERIC_TASK_HPP_INCLUDE

#include <functional>
#include <tuple>
#include <utility>

namespace thread {

namespace detail {
#if __cplusplus >= 201402L
using std::index_sequence;
using std::index_sequence_for;
using std::make_index_sequence;
#else
template <std::size_t...>
struct index_sequence {};

template <std::size_t N, std::size_t... Is>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct make_index_sequence<0, Is...> : index_sequence<Is...> {};

template <class... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
#endif
}  // namespace detail

template <class... Ts>
class GenericTask final : public Task {
public:
    template <class F, class... Args>
    GenericTask(F&& func, Args&&... args)
        : func_(std::forward<F>(func)), args_(std::forward<Args>(args)...) {}

    void run() override { runImpl(args_); }

private:
    std::function<void(Ts...)> func_;
    std::tuple<Ts...> args_;

    template <class... Args, int... Is>
    void runImpl(std::tuple<Args...>& tup, detail::index_sequence<Is...>) {
        func_(std::get<Is>(tup)...);
    }

    template <class... Args>
    void runImpl(std::tuple<Args...>& tup) {
        runImpl(tup, detail::index_sequence_for<Args...>{});
    }
};

template <class F, class... Args>
GenericTask<Args...> make_action(F&& f, Args&&... args) {
    return GenericTask<Args...>(std::forward<F>(f), std::forward<Args>(args)...);
}

}  // namespace thread

template <typename F, typename... Ts>
class Action : public thread::Task {
    static_assert(!(std::is_rvalue_reference_v<Ts> && ...));

private:
    F f;
    std::tuple<Ts...> args;

public:
    template <typename FwdF, typename... FwdTs,
              typename = std::enable_if_t<(std::is_convertible_v<FwdTs&&, Ts> && ...)>>
    Action(FwdF&& func, FwdTs&&... args)
        : f(std::forward<FwdF>(func)), args{std::forward<FwdTs>(args)...} {}

    void run() { std::apply(f, args); }
};

template <typename F, typename... Args>
auto make_action(F&& f, Args&&... args) {
    return Action<std::decay_t<F>, std::remove_cv_t<std::remove_reference_t<Args>>...>(
        std::forward<F>(f), std::forward<Args>(args)...);
}

#endif  // GENERIC_TASK_HPP_INCLUDE
