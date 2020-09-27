
#ifndef WW_SYNC_QUEUE_HELPER_H
#define WW_SYNC_QUEUE_HELPER_H

#include <vector>
#include <set>
#include <iostream>
#include "sync_queue.h"

namespace details {

    class Timer {
    public:
        Timer() {
            start = std::chrono::system_clock::now();
        }

        size_t totally() {
            return std::chrono::duration_cast<std::chrono::milliseconds>
                    (std::chrono::system_clock::now() - start).count();
        }

        void print() {
            std::cout << "Done in " << totally() << " ms" << std::endl;
        }

    private:
        std::chrono::time_point<std::chrono::system_clock> start;
    };

    class Task {
    public:
        friend std::istream &operator>>(std::istream &input, Task &task);

        Timer timer;
        unsigned long long start = 0, end = 0;
        std::string id;
    };

    std::istream &operator>>(std::istream &input, Task &task) {
        input >> task.id >> task.start >> task.end;
        return input;
    }

    class Done {
    public:
        Done(Task task, size_t counter) : task{task}, counter{counter} {
            result = task.timer.totally();
        }

        void print() const {
            std::cout << "Task " << task.id << " done in " << result << " ms, found "
                      << counter << " primes" << std::endl;
        }

    private:
        Task task;
        size_t counter = 0, result;
    };
}


#endif //WW_SYNC_QUEUE_HELPER_H
