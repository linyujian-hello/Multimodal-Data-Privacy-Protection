## Multimodal Differential Privacy Protection System

### Project Overview
This project implements a differential privacy protection framework supporting three data modalities: **numeric frequency counts**, **images**, and **text**. It applies the **Laplace mechanism** to perturb data, providing  privacy protection while quantifying the utility loss through metrics such as **MSE** (for numeric/text) and **PSNR** and **SSIM** (for images). Users can select the modality via command-line arguments and obtain noisy data along with quality assessments.

### Directory Structure
| Part                  | Description                                     |
| :-------------------- | :---------------------------------------------- |
| `common.h`            | Laplace noise generation and global sensitivity |
| `numeric_handler.h`   | Numeric data processing                         |
| `image_handler.h`     | Image data processing                           |
| `text_handler.h`      | Text data processin                             |
| `common.cpp`          | Implementation of utilities                     |
| `numeric_handler.cpp` | Numeric data handling functions                 |
| `image_handler.cpp`   | Image data handling functions                   |
| `text_handler.cpp`    | Text data handling functions                    |

---

### Application Scenarios
This toolkit is designed for privacy-preserving data publishing and analysis in various domains:

* **survey data** — Publish aggregated frequency counts while protecting individual responses using differential privacy.
* **Natural language processing** — Share word frequency statistics from sensitive text corpora with formal privacy guarantees .
* **Machine learning model training** — Preprocess training data with differential privacy to train models with DP guarantees.
* **Federated learning**  — Perturb local data summaries before aggregation to protect user privacy.
* **Benchmarking privacy-utility trade-offs** — Evaluate how different privacy budgets (epsilon) affect data quality across modalities.

---

### Performance & Scaling

- The current implementation is single-threaded and suitable for small to moderate datasets. For large-scale data, consider:
  - Parallelizing processing with multi-threading.
  - Reading and processing data in batches.
  - Using faster storage.

## Contributing

Contributions are welcome! Please feel free to contact me. Before contributing, ensure your code adheres to the existing style and includes appropriate tests.
