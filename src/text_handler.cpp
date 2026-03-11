#include "text_handler.h"
#include "common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>

// Clean token: keep letters and digits, lowercase.
static std::string clean_token(const std::string& word) {
    std::string cleaned;
    cleaned.reserve(word.size());
    for (char ch : word) {
        if (std::isalnum(static_cast<unsigned char>(ch))) cleaned.push_back(std::tolower(static_cast<unsigned char>(ch)));
    }
    return cleaned;
}

static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;
    while (iss >> word) {
        std::string cleaned = clean_token(word);
        if (!cleaned.empty()) tokens.push_back(cleaned);
    }
    return tokens;
}

void process_text_data(const std::string& input_file,
    const std::string& output_file,
    double epsilon,
    int repetitions,
    std::mt19937& gen) {
    if (!(epsilon > 0.0)) {
        std::cerr << "Text: epsilon must be > 0\n";
        return;
    }
    std::ifstream infile(input_file);
    if (!infile.is_open()) {
        std::cerr << "Cannot open input file: " << input_file << std::endl;
        return;
    }
    std::unordered_map<std::string, int> freq_map;
    std::string line;
    while (std::getline(infile, line)) {
        auto toks = tokenize(line);
        for (const auto& t : toks) freq_map[t]++;
    }
    infile.close();
    if (freq_map.empty()) {
        std::cerr << "No tokens found in text.\n";
        return;
    }

    // Convert to vector and sort by token (stable deterministic output)
    std::vector<std::pair<std::string, int>> items;
    items.reserve(freq_map.size());
    for (auto& p : freq_map) items.emplace_back(p.first, p.second);
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    int vocab_size = static_cast<int>(items.size());
    std::vector<std::string> words(vocab_size);
    std::vector<int> true_counts(vocab_size);
    for (int i = 0; i < vocab_size; ++i) {
        words[i] = items[i].first;
        true_counts[i] = items[i].second;
    }

    double sensitivity = 1.0;
    double total_mse = 0.0;
    std::vector<std::vector<double>> all_noisy(repetitions, std::vector<double>(vocab_size, 0.0));

    for (int rep = 0; rep < repetitions; ++rep) {
        std::vector<double> noisy(vocab_size);
        for (int i = 0; i < vocab_size; ++i) {
            noisy[i] = static_cast<double>(true_counts[i]) + laplace_noise(sensitivity / epsilon, gen);
            if (noisy[i] < 0.0) noisy[i] = 0.0;
        }
        all_noisy[rep] = noisy;
        double sumsq = 0.0;
        for (int i = 0; i < vocab_size; ++i) {
            double diff = static_cast<double>(true_counts[i]) - noisy[i];
            sumsq += diff * diff;
        }
        double mse = sumsq / static_cast<double>(vocab_size);
        total_mse += mse;
        std::cout << "Repetition " << rep + 1 << " - MSE: " << mse << std::endl;
    }

    double avg_mse = total_mse / static_cast<double>(repetitions);
    std::cout << "Average MSE over " << repetitions << " repetitions: " << avg_mse << std::endl;

    if (!output_file.empty()) {
        std::ofstream outfile(output_file);
        if (!outfile.is_open()) {
            std::cerr << "Cannot open output file: " << output_file << std::endl;
            return;
        }
        outfile << "word,noisy_count\n";
        // write last repetition
        for (int i = 0; i < vocab_size; ++i) {
            outfile << words[i] << "," << std::fixed << std::setprecision(2) << all_noisy[repetitions - 1][i] << "\n";
        }
        outfile.close();
        std::cout << "Noisy word frequencies saved to: " << output_file << std::endl;
    }
}