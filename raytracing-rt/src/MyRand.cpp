#include "MyRand.h"


namespace CustomRand {
    // Declare thread_local variables
    thread_local std::mt19937 CustomRand::mersenneTwister{ std::random_device{}() };
    thread_local std::uniform_real_distribution<double> CustomRand::uniform(0.0, 1.0);
}