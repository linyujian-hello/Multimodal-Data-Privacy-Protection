#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

#include <string>
#include <random>

void process_image_data(const std::string& input_folder,
    const std::string& output_folder,
    double epsilon,
    int repetitions,
    std::mt19937& gen);

#endif // IMAGE_HANDLER_H