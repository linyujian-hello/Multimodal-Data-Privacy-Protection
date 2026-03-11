#ifndef COMMON_H
#define COMMON_H

#include <random>
#include <stdexcept>

double laplace_noise(double scale, std::mt19937& gen);


inline double global_sensitivity(bool normalized = true) {
    return normalized ? 1.0 : 255.0;
}

#endif 