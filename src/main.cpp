#include <iostream>
#include <string>
#include <random>
#include <cstring>
#include <filesystem>
#include "numeric_handler.h"
#include "image_handler.h"
#include "text_handler.h"

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " --type <numeric|image|text> --input <path> [--output <path>] --epsilon <value> [--repetitions <N>] [--max-item <M>]\n";
    std::cout << "  --type: data type: numeric, image, or text\n";
    std::cout << "  --input: input file (for numeric/text) or folder (for image)\n";
    std::cout << "  --output: output file (for numeric/text) or folder (for image)\n";
    std::cout << "  --epsilon: privacy budget (positive double)\n";
    std::cout << "  --repetitions: number of repetitions (default: 5)\n";
    std::cout << "  --max-item: maximum item ID for numeric data (default: 10)\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string type, input_path, output_path;
    double epsilon = 0.0;
    int repetitions = 5;
    int max_item = 10;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            type = argv[++i];
        }
        else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            input_path = argv[++i];
        }
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        }
        else if (strcmp(argv[i], "--epsilon") == 0 && i + 1 < argc) {
            epsilon = std::stod(argv[++i]);
        }
        else if (strcmp(argv[i], "--repetitions") == 0 && i + 1 < argc) {
            repetitions = std::stoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--max-item") == 0 && i + 1 < argc) {
            max_item = std::stoi(argv[++i]);
        }
        else {
            std::cerr << "Unknown or malformed argument: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (type.empty() || input_path.empty() || epsilon <= 0.0) {
        std::cerr << "Error: missing required arguments or invalid epsilon.\n";
        print_usage(argv[0]);
        return 1;
    }

    // Seed RNG with random device for non-deterministic runs; change to fixed seed for reproducibility
    std::random_device rd;
    std::mt19937 gen(rd());

    try {
        if (type == "numeric") {
            process_numeric_data(input_path, epsilon, max_item, repetitions, gen);
        }
        else if (type == "image") {
            if (output_path.empty()) {
                std::cerr << "Error: --output required for image type.\n";
                return 1;
            }
            process_image_data(input_path, output_path, epsilon, repetitions, gen);
        }
        else if (type == "text") {
            // for text: if output not provided, write to noisy_counts.csv in current dir
            if (output_path.empty()) {
                output_path = "noisy_counts.csv";
            }
            process_text_data(input_path, output_path, epsilon, repetitions, gen);
        }
        else {
            std::cerr << "Unknown type: " << type << "\n";
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}