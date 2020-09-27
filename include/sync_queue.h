

#ifndef WW_SYNC_QUEUE_SYNC_QUEUE_H
#define WW_SYNC_QUEUE_SYNC_QUEUE_H


#include <optional>
#include <cstddef>
#include <stdexcept>
#include <experimental/optional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace lab_17 {
    class queue_is_shutdown : std::exception {
    };

    template<typename T>
    class sync_queue {
    public:

        explicit sync_queue(std::size_t max_size = std::numeric_limits<std::size_t>::max());

        ~sync_queue() noexcept = default;

        sync_queue(sync_queue &&) noexcept = delete;

        sync_queue &operator=(sync_queue &&) noexcept = delete;

        sync_queue(sync_queue const &) = delete;

        sync_queue &operator=(sync_queue const &) = delete;

        void push(T &&value);

        void push(const T &value);

        T pop();

        bool try_push(T &&x);

        bool try_push(const T &x);

        std::optional<T> try_pop();

        void shutdown();

        bool is_shutdown() const noexcept;

        std::size_t size() const noexcept;

    private:

        bool do_try_push(T &&value);

        bool do_try_push(const T &value);

        void check_down();

        std::optional<T> do_try_pop();

        mutable std::mutex queueMutex;
        std::queue<T> mQueue;
        size_t maxSize;
        bool queueShutdown = false;

        std::condition_variable queueNotFull;
        std::condition_variable queueNotEmpty;

    };

    template<typename T>
    sync_queue<T>::sync_queue(std::size_t max_size) : maxSize{max_size} {}

    template<typename T>
    bool sync_queue<T>::is_shutdown() const noexcept {
        return queueShutdown;
    }

    template<typename T>
    void sync_queue<T>::check_down() {
        if (is_shutdown())
            throw queue_is_shutdown();
    }

    template<typename T>
    void sync_queue<T>::shutdown() {
        std::lock_guard<std::mutex> guard(queueMutex);

        check_down();

        queueShutdown = true;
        queueNotEmpty.notify_all();
        queueNotFull.notify_all();
    }

    template<typename T>
    std::size_t sync_queue<T>::size() const noexcept {
        std::lock_guard<std::mutex> guard(queueMutex);
        return mQueue.size();
    }

    template<typename T>
    std::optional<T> sync_queue<T>::try_pop() {
        std::lock_guard<std::mutex> guard(queueMutex);

        if (is_shutdown())
            return std::nullopt;

        return do_try_pop();
    }

    template<typename T>
    T sync_queue<T>::pop() {
        std::unique_lock<std::mutex> queueLock(queueMutex);

        queueNotEmpty.wait(queueLock, [this]() {
            return is_shutdown() || mQueue.size() > 0;
        });

        check_down();

        return do_try_pop().value();
    }

    template<typename T>
    bool sync_queue<T>::try_push(T &&x) {
        std::lock_guard<std::mutex> guard(queueMutex);

        if (is_shutdown())
            return false;

        return do_try_push(std::forward<T>(x));
    }

    template<typename T>
    bool sync_queue<T>::try_push(const T &x) {
        std::lock_guard<std::mutex> guard(queueMutex);

        if (is_shutdown())
            return false;

        return do_try_push(std::forward<T>(x));
    }

    template<typename T>
    bool sync_queue<T>::do_try_push(T &&value) {
        if (mQueue.size() == maxSize) {
            return false;
        }

        mQueue.push(std::move(value));
        queueNotEmpty.notify_one();
        return true;
    }

    template<typename T>
    bool sync_queue<T>::do_try_push(const T &value) {
        if (mQueue.size() == maxSize) {
            return false;
        }

        mQueue.push(std::move(value));
        queueNotEmpty.notify_one();
        return true;
    }

    template<typename T>
    std::optional<T> sync_queue<T>::do_try_pop() {
        if (mQueue.empty()) {
            return std::nullopt;
        }

        auto element = std::move(mQueue.front());
        mQueue.pop();
        queueNotFull.notify_one();
        return element;
    }

    template<typename T>
    void sync_queue<T>::push(T &&value) {

        std::unique_lock<std::mutex> queueLock(queueMutex);

        queueNotFull.wait(queueLock, [this]() {
            return is_shutdown() || mQueue.size() < maxSize;
        });

        check_down();

        do_try_push(std::forward<T>(value));
    }

    template<typename T>
    void sync_queue<T>::push(const T &value) {

        std::unique_lock<std::mutex> queueLock(queueMutex);

        queueNotFull.wait(queueLock, [this]() {
            return is_shutdown() || mQueue.size() < maxSize;
        });

        check_down();

        do_try_push(std::forward<T>(value));
    }

}

#endif //WW_SYNC_QUEUE_SYNC_QUEUE_H
