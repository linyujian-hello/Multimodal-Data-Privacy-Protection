#include "numeric_handler.h"
#include "common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>

static std::vector<std::vector<int>> read_transactions(const std::string& filename) {
    std::vector<std::vector<int>> transactions;
    std::ifstream file(filename);
    if (!file.is_open()) return transactions;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<int> record;
        int val;
        while (iss >> val) {
            record.push_back(val);
        }
        if (!record.empty()) transactions.push_back(std::move(record));
    }
    return transactions;
}

static std::vector<int> compute_frequencies(const std::vector<std::vector<int>>& transactions, int max_item) {
    std::vector<int> freq(std::max(1, max_item), 0);
    for (const auto& t : transactions) {
        for (int item : t) {
            if (item >= 1 && item <= max_item) freq[item - 1]++;
        }
    }
    return freq;
}

static std::vector<double> add_laplace_noise_to_counts(const std::vector<int>& true_counts,
    double epsilon,
    double sensitivity,
    std::mt19937& gen) {
    double scale = sensitivity / epsilon;
    std::vector<double> noisy(true_counts.size());
    for (size_t i = 0; i < true_counts.size(); ++i) {
        double noise = laplace_noise(scale, gen);
        noisy[i] = static_cast<double>(true_counts[i]) + noise;
        // optional: clip to non-negative (counts cannot be negative)
        if (noisy[i] < 0.0) noisy[i] = 0.0;
    }
    return noisy;
}

static double compute_mse(const std::vector<int>& true_counts, const std::vector<double>& noisy_counts) {
    double sum = 0.0;
    size_t n = std::max<size_t>(1, true_counts.size());
    for (size_t i = 0; i < true_counts.size(); ++i) {
        double diff = static_cast<double>(true_counts[i]) - noisy_counts[i];
        sum += diff * diff;
    }
    return sum / static_cast<double>(n);
}

void process_numeric_data(const std::string& input_file,
    double epsilon,
    int max_item,
    int repetitions,
    std::mt19937& gen) {
    if (epsilon <= 0.0) {
        std::cerr << "Numeric: epsilon must be > 0\n";
        return;
    }
    if (max_item <= 0) {
        std::cerr << "Numeric: max_item must be > 0\n";
        return;
    }
    auto transactions = read_transactions(input_file);
    if (transactions.empty()) {
        std::cerr << "No transactions read from: " << input_file << std::endl;
        return;
    }
    auto true_counts = compute_frequencies(transactions, max_item);

    double sensitivity = global_sensitivity(true); // for counts, sensitivity = 1
    double total_mse = 0.0;
    for (int rep = 0; rep < repetitions; ++rep) {
        auto noisy = add_laplace_noise_to_counts(true_counts, epsilon, sensitivity, gen);
        double mse = compute_mse(true_counts, noisy);
        total_mse += mse;
        std::cout << "Repetition " << rep + 1 << " - MSE: " << mse << "\nNoisy counts: ";
        for (double v : noisy) std::cout << std::fixed << std::setprecision(2) << v << " ";
        std::cout << std::endl;
    }
    double avg_mse = total_mse / static_cast<double>(repetitions);
    std::cout << "Average MSE over " << repetitions << " repetitions: " << avg_mse << std::endl;
}