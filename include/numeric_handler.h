#ifndef NUMERIC_HANDLER_H
#define NUMERIC_HANDLER_H

#include <vector>
#include <string>
#include <random>


void process_numeric_data(const std::string& input_file,
    double epsilon,
    int max_item,
    int repetitions,
    std::mt19937& gen);

#endif 