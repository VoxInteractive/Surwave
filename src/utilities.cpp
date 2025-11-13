#include <algorithm>
#include <thread>

#include "utilities.h"

using namespace project;

unsigned int Utilities::get_thread_count()
{
    unsigned int num_hardware_threads = std::thread::hardware_concurrency();
    unsigned int num_threads = std::max(
        1U, std::min(
            64U, // Maximum thread count
            num_hardware_threads > 1 ? num_hardware_threads - 1 : 1U));
    return num_threads;
}
