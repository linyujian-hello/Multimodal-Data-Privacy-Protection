#include "image_handler.h"
#include "common.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace fs = std::filesystem;

// read image and convert to CV_32FC3 in range [0,1]
static cv::Mat read_image_normalized(const std::string& path) {
    cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
    if (img.empty()) return cv::Mat();
    cv::Mat img_float;
    img.convertTo(img_float, CV_32F, 1.0 / 255.0);
    return img_float;
}

static void save_image_denormalized(const cv::Mat& img_float, const std::string& path) {
    cv::Mat clipped;
    cv::min(cv::max(img_float, 0.0f), 1.0f, clipped);
    cv::Mat img_uint8;
    clipped.convertTo(img_uint8, CV_8U, 255.0);
    cv::imwrite(path, img_uint8);
}

static void add_laplace_noise_to_image(cv::Mat& img_float, double epsilon, double sensitivity, std::mt19937& gen) {
    if (epsilon <= 0.0) throw std::invalid_argument("epsilon must be > 0");
    double scale = sensitivity / epsilon;
    // iterate per pixel and channel
    for (int r = 0; r < img_float.rows; ++r) {
        for (int c = 0; c < img_float.cols; ++c) {
            cv::Vec3f& pixel = img_float.at<cv::Vec3f>(r, c);
            for (int ch = 0; ch < 3; ++ch) {
                float noise = static_cast<float>(laplace_noise(scale, gen));
                pixel[ch] += noise;
            }
        }
    }
    // clip to [0,1]
    cv::min(cv::max(img_float, 0.0f), 1.0f, img_float);
}

static double compute_psnr(const cv::Mat& original, const cv::Mat& distorted) {
    // assume CV_32F and range [0,1]
    cv::Mat diff;
    cv::absdiff(original, distorted, diff);
    diff = diff.mul(diff);
    cv::Scalar sse_scalar = cv::sum(diff);
    double sse = sse_scalar[0] + sse_scalar[1] + sse_scalar[2];
    if (sse <= 1e-12) return std::numeric_limits<double>::infinity(); // no noise
    double mse = sse / (static_cast<double>(original.total()) * original.channels());
    double max_i = 1.0; // because normalized
    return 10.0 * std::log10((max_i * max_i) / mse);
}

static double compute_ssim(const cv::Mat& img1, const cv::Mat& img2) {
    // Implement SSIM for normalized CV_32F images
    const double C1 = 0.01 * 0.01;
    const double C2 = 0.03 * 0.03;
    cv::Mat mu1, mu2;
    cv::GaussianBlur(img1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(img2, mu2, cv::Size(11, 11), 1.5);

    cv::Mat mu1_sq = mu1.mul(mu1);
    cv::Mat mu2_sq = mu2.mul(mu2);
    cv::Mat mu1_mu2 = mu1.mul(mu2);

    cv::Mat sigma1_sq, sigma2_sq, sigma12;
    cv::Mat tmp1 = img1.mul(img1);
    cv::GaussianBlur(tmp1, sigma1_sq, cv::Size(11, 11), 1.5);
    sigma1_sq -= mu1_sq;

    cv::Mat tmp2 = img2.mul(img2);
    cv::GaussianBlur(tmp2, sigma2_sq, cv::Size(11, 11), 1.5);
    sigma2_sq -= mu2_sq;

    cv::Mat tmp12 = img1.mul(img2);
    cv::GaussianBlur(tmp12, sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    cv::Mat t1 = 2.0 * mu1_mu2 + C1;
    cv::Mat t2 = 2.0 * sigma12 + C2;
    cv::Mat t3 = mu1_sq + mu2_sq + C1;
    cv::Mat t4 = sigma1_sq + sigma2_sq + C2;

    cv::Mat ssim_map;
    cv::divide(t1.mul(t2), t3.mul(t4), ssim_map);
    cv::Scalar mean_ssim = cv::mean(ssim_map);
    return (mean_ssim[0] + mean_ssim[1] + mean_ssim[2]) / 3.0;
}

void process_image_data(const std::string& input_folder,
    const std::string& output_folder,
    double epsilon,
    int repetitions,
    std::mt19937& gen) {
    if (!(epsilon > 0.0)) {
        std::cerr << "Image: epsilon must be > 0\n";
        return;
    }
    if (!fs::exists(input_folder)) {
        std::cerr << "Input folder does not exist: " << input_folder << std::endl;
        return;
    }
    fs::create_directories(output_folder);

    std::vector<std::string> image_paths;
    for (const auto& entry : fs::directory_iterator(input_folder)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tiff")
            image_paths.push_back(entry.path().string());
    }
    if (image_paths.empty()) {
        std::cerr << "No images found in: " << input_folder << std::endl;
        return;
    }

    double total_psnr = 0.0, total_ssim = 0.0;
    int total_images = 0;
    double sensitivity = global_sensitivity(true); // normalized

    for (int rep = 0; rep < repetitions; ++rep) {
        for (const auto& img_path : image_paths) {
            cv::Mat original = read_image_normalized(img_path);
            if (original.empty()) continue;
            cv::Mat noisy = original.clone();
            add_laplace_noise_to_image(noisy, epsilon, sensitivity, gen);

            fs::path in_path(img_path);
            std::string out_name = "noisy_" + std::to_string(rep + 1) + "_" + in_path.filename().string();
            fs::path out_path = fs::path(output_folder) / out_name;
            save_image_denormalized(noisy, out_path.string());

            double psnr = compute_psnr(original, noisy);
            double ssim = compute_ssim(original, noisy);
            total_psnr += psnr;
            total_ssim += ssim;
            total_images++;

            std::cout << "Rep " << rep + 1 << ", " << in_path.filename()
                << " -> PSNR: " << std::fixed << std::setprecision(3) << psnr
                << " dB, SSIM: " << std::fixed << std::setprecision(4) << ssim << std::endl;
        }
    }

    if (total_images > 0) {
        std::cout << "\nAverage PSNR: " << (total_psnr / total_images)
            << " dB, Average SSIM: " << (total_ssim / total_images) << std::endl;
    }
}