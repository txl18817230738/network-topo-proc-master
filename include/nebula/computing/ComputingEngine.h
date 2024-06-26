// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <algorithm>
#include <memory>

#include "nebula/common/base/Status.h"
#include "nebula/common/thread/GenericThreadPool.h"

namespace nebula {
class PlanNode;
class ResultTable;

namespace computing {
class ComputingContext;

template <typename T, bool isFuture>
struct FutureValueType;

template <typename T>
struct FutureValueType<T, true> {
    using type = typename T::value_type;
};

template <typename T>
struct FutureValueType<T, false> {
    using type = T;
};

template <typename T>
using future_value_t = typename FutureValueType<T, folly::isFutureOrSemiFuture<T>::value>::type;

template <typename F, typename... Args>
using ReturnType =
        std::conditional_t<std::is_void_v<thread::GenericWorker::ReturnType<F, Args...>>,
                           folly::Unit,
                           future_value_t<thread::GenericWorker::ReturnType<F, Args...>>>;

template <typename Container>
using value_t = typename Container::value_type;

template <typename F, typename Container, typename... Args>
using return_t = ReturnType<F, value_t<Container>, Args...>;

/**
 * @brief The ComputingEngine is used to run algorithms in parallel.
 *  It will create a thread pool to run the algorithms. The thread pool will be shared by all
 *  the algorithms and destroyed when the ComputingEngine is destroyed.
 */
class ComputingEngine final {
public:
    ComputingEngine();
    ~ComputingEngine();

    /**
     * @brief Initialize the ComputingEngine and create the thread pool.
     * @return Status
     */
    Status init();

    /**
     * @brief Run the future on the current thread.
     * @param f The future to be get.
     * @return The result of the future.
     */
    template <typename T>
    static T runOnCurrentThread(folly::SemiFuture<T>&& future);

    /**
     * @brief Use the thread pool to run the function applied on a container in parallel.
     * @param container The container to be applied on.
     * @param f The function to be run, must be thread-safe.
     * @return The result of the future.
     */
    template <typename Container, typename F, typename R = return_t<F, Container>>
    auto parallelFor(const Container& container, F&& f) -> folly::SemiFuture<std::vector<R>> {
        return parallelFor(std::begin(container), std::end(container), std::forward<F>(f));
    }

    /**
     * @brief Use the thread pool to run the function applied on a container in parallel.
     * @param begin The begin iterator of the container.
     * @param end The end iterator of the container.
     * @param f The function to be run, must be thread-safe.
     * @return The result of the future.
     */
    template <typename Iterator, typename F, typename R = return_t<F, Iterator>>
    auto parallelFor(Iterator begin, Iterator end, F&& f) -> folly::SemiFuture<std::vector<R>>;


    /**
     * @brief Use the thread pool to run the filter on a container in parallel.
     * @param container The container to be filtered.
     * @param f The filter function, must be thread-safe.
     * @return The result of the future.
     */
    template <typename Container, typename F, typename R = value_t<Container>>
    auto parallelFilter(const Container& container, F&& f)
            -> folly::SemiFuture<std::vector<R>> {
        return parallelFilter(std::begin(container), std::end(container), std::forward<F>(f));
    }

    /**
     * @brief Use the thread pool to run the filter on a container in parallel.
     * @param begin The begin iterator of the container.
     * @param end The end iterator of the container.
     * @param f The filter function, must be thread-safe.
     * @return The result of the future.
     */
    template <typename Iterator, typename F, typename R = value_t<Iterator>>
    auto parallelFilter(Iterator begin, Iterator end, F&& f)
            -> folly::SemiFuture<std::vector<R>>;

    /**
     * @brief Use the thread pool to run the reduce on a container in parallel.
     * @param container The container to be filtered.
     * @param f The reduce function, must be thread-safe.
     * @param rf The reduce function, must be thread-safe.
     * @return The result of the future.
     */
    template <typename Container,
              typename F,
              typename T,
              typename R = value_t<Container>,
              typename RF = std::function<T(const T&, R)>>
    auto parallelReduce(const Container& container, F f, RF rf)
            -> folly::SemiFuture<std::vector<R>> {
        using Iterator = decltype(std::begin(container));
        return parallelReduce<Iterator, F, T, R, RF>(
                std::begin(container), std::end(container), std::move(f), std::move(rf));
    }

    /**
     * @brief Use the thread pool to run the reduce on a container in parallel.
     * @param begin The begin iterator of the container.
     * @param end The end iterator of the container.
     * @param f The filter function, must be thread-safe.
     * @param rf The reduce function, must be thread-safe.
     * @return The result of the future.
     */
    template <typename Iterator,
              typename F,
              typename T,
              typename R = value_t<Iterator>,
              typename RF = std::function<T(const T&, R)>>
    auto parallelReduce(Iterator begin, Iterator end, F f, RF rf)
            -> folly::SemiFuture<std::vector<R>>;

    thread::GenericThreadPool* threadPool() const {
        return threadPool_.get();
    }

    static constexpr size_t kParallelThreshold = 5000u;

private:
    /**
     * @brief Split the range into tasks
     * @return the pair of the number of tasks and the size of each task
     */
    std::pair<size_t, size_t> splitTasks(size_t range) const {
        auto numThreads = threadPool_->numThreads();
        if (range < kParallelThreshold || range < numThreads) {
            return std::make_pair(1u, range);
        }
        auto dataSizePerThread = range / numThreads;
        if (range % numThreads != 0) {
            ++dataSizePerThread;
        }
        return std::make_pair(numThreads, dataSizePerThread);
    }

    // Threads used for computing algorithms
    std::unique_ptr<thread::GenericThreadPool> threadPool_;
};

template <typename T>
T ComputingEngine::runOnCurrentThread(folly::SemiFuture<T>&& future) {
    folly::InlineExecutor executor;
    return std::move(future)
            .via(&executor)
            .thenError([](folly::exception_wrapper&& ex) {
                LOG(ERROR) << "Failed to run: " << ex.get_exception()->what();
                return T{};
            })
            .get();
}

template <typename Iterator, typename F, typename R>
auto ComputingEngine::parallelFor(Iterator begin, Iterator end, F&& f)
        -> folly::SemiFuture<std::vector<R>> {
    auto fn = [f](auto id) {
        if constexpr (std::is_same_v<R, folly::Unit>) {
            f(id);
            return folly::Unit{};
        } else {
            return f(id);
        }
    };

    auto [numTasks, step] = this->splitTasks(end - begin);
    std::vector<folly::SemiFuture<std::vector<R>>> futures;
    for (size_t i = 0; i < numTasks; ++i) {
        auto from = begin + i * step;
        auto to = (from + step >= end ? end : from + step);
        std::vector<typename Iterator::value_type> c(from, to);
        auto cb = [c, fn]() -> folly::SemiFuture<std::vector<R>> {
            std::vector<folly::SemiFuture<R>> res;
            res.reserve(c.size());
            for (auto v : c) {
                res.emplace_back(fn(v));
            }
            return folly::collectAll(res).deferValue([](std::vector<folly::Try<R>>&& tries) {
                std::vector<R> ret;
                ret.reserve(tries.size());
                for (auto& t : tries) {
                    ret.emplace_back(std::move(t).value());
                }
                return ret;
            });
        };
        auto nestedFuture = threadPool_->addTask(std::move(cb));
        // Flatten the nested future to future
        auto future =
                std::move(nestedFuture).deferValue([](auto&& ff) { return std::move(ff); });
        futures.emplace_back(std::move(future));
    }
    return folly::collectAll(futures).deferValue([](auto&& res) {
        std::vector<R> ret;
        for (auto& t : res) {
            auto& v = t.value();
            ret.insert(ret.end(), v.begin(), v.end());
        }
        return ret;
    });
}

template <typename Iterator, typename F, typename R>
auto ComputingEngine::parallelFilter(Iterator begin, Iterator end, F&& f)
        -> folly::SemiFuture<std::vector<R>> {
    auto [numTasks, step] = this->splitTasks(end - begin);
    std::vector<folly::SemiFuture<std::vector<R>>> futures;
    for (size_t i = 0; i < numTasks; ++i) {
        auto from = begin + i * step;
        auto to = (from + step >= end ? end : from + step);
        std::vector<typename Iterator::value_type> c(from, to);
        auto future = threadPool_->addTask([c, f]() {
            std::vector<R> res;
            res.reserve(c.size());
            for (auto v : c) {
                f(v, res);
            }
            return res;
        });
        futures.emplace_back(std::move(future));
    }
    return folly::collectAll(futures).deferValue([](auto&& res) {
        std::vector<R> ret;
        for (auto& t : res) {
            auto& v = t.value();
            ret.insert(ret.end(), v.begin(), v.end());
        }
        return ret;
    });
}


template <typename Iterator, typename F, typename T, typename R, typename RF>
auto ComputingEngine::parallelReduce(Iterator begin, Iterator end, F f, RF rf)
        -> folly::SemiFuture<std::vector<R>> {
    auto [numTasks, step] = this->splitTasks(end - begin);
    std::vector<folly::SemiFuture<std::vector<R>>> futures;
    for (size_t i = 0; i < numTasks; ++i) {
        auto from = begin + i * step;
        auto to = (from + step >= end ? end : from + step);
        std::vector<typename Iterator::value_type> c(from, to);
        auto future = threadPool_->addTask([c, f, rf]() {
            std::vector<R> res;
            std::unordered_map<R, std::vector<T>> tmp;
            res.reserve(c.size());
            tmp.reserve(c.size());
            for (const auto& v : c) {
                f(v, tmp, res);
            }
            for (const auto& v : res) {
                // FIXME(yee): how to init the first value
                T tnew;
                for (const auto& t : tmp[v]) {
                    tnew = rf(tnew, t);
                }
            }
            return res;
        });
        futures.emplace_back(std::move(future));
    }
    return folly::collectAll(futures).deferValue([](auto&& res) {
        std::vector<R> ret;
        for (auto& t : res) {
            if (!t.hasException()) {
                auto& v = t.value();
                ret.insert(ret.end(), v.begin(), v.end());
            }
        }
        return ret;
    });
}

}  // namespace computing
}  // namespace nebula
