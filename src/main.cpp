#include <iostream>
#include "helper.h"


static details::Done calculate(details::Task &task, bool &end) {

    size_t counter = 0;

    for (auto begin = task.start; begin < task.end; begin++) {
        if (begin <= 1) continue;
        bool prime = true;
        for (size_t j = 2; j * j <= begin; j++) {
            if (end)
                throw lab_17::queue_is_shutdown();
            if (begin % j == 0) {
                prime = false;
                break;
            }
        }
        if (prime) counter++;
    }

    details::Done done(task, counter);
    return done;
}

void Cycle(lab_17::sync_queue<details::Task> &tasks,
           lab_17::sync_queue<details::Done> &results, bool &end) {
    while (true) {
        try {
            auto task = tasks.pop();
            results.push(calculate(task, end));
        }
        catch (lab_17::queue_is_shutdown &e) {
            return;
        }
    }
}

int main(int argc, char **argv) {

    lab_17::sync_queue<details::Task> tasks(std::stoull(argv[2]));
    lab_17::sync_queue<details::Done> results;
    std::vector<std::thread> threads(std::stoull(argv[1]));

    bool end = false;

    for (auto &thread : threads) {
        thread = std::thread(Cycle,
                             std::ref(tasks), std::ref(results), std::ref(end));
    }

    std::string command;
    bool work = true;

    while (work) {

        std::cin >> command;

        if (command == "submit") {
            details::Timer timer;
            details::Task task;
            std::cin >> task;
            if (tasks.size() == std::stoull(argv[2])) {
                std::cout << "Queue is full!" << std::endl;
                continue;
            }
            tasks.push(std::forward<details::Task>(task));
            timer.print();
        } else if (command == "show-done") {
            details::Timer timer;
            while (results.size() > 0) {
                results.pop().print();
            }
            timer.print();
        } else if (command == "quit") {
            details::Timer timer;
            end = true;
            work = false;
            tasks.shutdown();
            results.shutdown();
            for (auto &thread : threads) {
                thread.join();
            }
            timer.print();
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    return 0;
}
