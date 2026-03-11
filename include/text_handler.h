#ifndef TEXT_HANDLER_H
#define TEXT_HANDLER_H

#include <string>
#include <random>

void process_text_data(const std::string& input_file,
    const std::string& output_file,
    double epsilon,
    int repetitions,
    std::mt19937& gen);

#endif // TEXT_HANDLER_H