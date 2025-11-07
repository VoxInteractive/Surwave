#include <algorithm>
#include <thread>

#include "constants.h"
#include "utilities.h"

using namespace project;

unsigned int Utilities::get_thread_count()
{
    unsigned int num_hardware_threads = std::thread::hardware_concurrency();
    unsigned int num_threads = std::max(
        1U, std::min(
                project::constants::MAX_THREADS,
                num_hardware_threads > 1 ? num_hardware_threads - 1 : 1U));
    return num_threads;
}
