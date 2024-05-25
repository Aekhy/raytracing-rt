#pragma once
#include <random>

namespace CustomRand {
    // Declare thread_local variables
    extern thread_local std::mt19937 mersenneTwister;
    extern thread_local std::uniform_real_distribution<double> uniform;

    // Function to get a random value in [0, 1)
    inline float uniform_random_value() {
        return uniform(mersenneTwister);
    }
}
