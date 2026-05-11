#include <iostream>
#include <vector>

#include "models/event.hpp"

#include "logs.hpp"
#include "output.hpp"

int num_calls = 0;
int num_successful_calls = 0;
int total_time = 0;
std::vector<Event> events;

void log_event(Event e) {
    events.push_back(e);
}

void log_call() {
    num_calls++;
}

void log_successful_call() {
    num_successful_calls++;
}

void add_time(int time) {
    total_time += time;
}

void output_metrics() {
    for (Event e : events) {
        std::cout << e << '\n';
    }
    std::cout << "Total calls: " << num_calls << '\n';
    std::cout << "Successful calls: " << num_successful_calls << '\n';
    std::cout << "Call success rate: " << 1.0 * num_successful_calls / num_calls * 100 << "%\n";
    std::cout << "Average call time: " << 1.0 * total_time / num_calls << " minutes\n";
}
