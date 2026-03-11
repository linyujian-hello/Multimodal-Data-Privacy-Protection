#include "common.h"
#include <cmath>
#include <limits>

double laplace_noise(double scale, std::mt19937& gen) {
    if (!(scale > 0.0)) {
        throw std::invalid_argument("laplace_noise: scale must be > 0");
    }
    std::uniform_real_distribution<double> dist(-0.5, 0.5);
    double u = dist(gen);

    double absu = std::abs(u);
    if (absu >= 0.5) { 
        absu = std::nextafter(0.5, 0.0);
    }
    double sign = (u >= 0.0) ? 1.0 : -1.0;
    double val = -scale * sign * std::log(1.0 - 2.0 * absu);

    if (!std::isfinite(val)) return 0.0;
    return val;
}