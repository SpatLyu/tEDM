#include <vector>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

/**
 * @brief Generate time-delay embeddings for a univariate time series.
 *
 * This function reconstructs the state space of a scalar time series
 * using time-delay embedding with dimension E and lag tau.
 *
 * - When tau = 0, embedding uses lags of 0, 1, ..., E-1.  (original behavior)
 * - When tau > 0 and style = 1, embedding uses lags of tau, 2*tau, ..., E*tau.
 * - When tau > 0 and style = 0, embedding uses lags of 0, tau, 2*tau, ..., (E-1)*tau.
 *
 * Example:
 * Input: vec = {1, 2, 3, 4, 5}, E = 3, tau = 0
 * Output:
 * 1    NaN    NaN
 * 2    1      NaN
 * 3    2      1
 * 4    3      2
 * 5    4      3
 *
 * All values are pre-initialized to NaN. (Elements are filled only when
 * sufficient non-NaN lagged values are available. *Previously bound,
 * now abandoned*) Columns containing only NaN values are removed before
 * returning. If no valid embedding columns remain (due to short input
 * and large E/tau), an exception is thrown.
 *
 * @param vec The input time series as a vector of doubles.
 * @param E Embedding dimension.
 * @param tau Time lag.
 * @param style Lag style when tau > 0:
 *        - style = 1: tau, 2*tau, ..., E*tau
 *        - style = 0: 0, tau, 2*tau, ..., (E-1)*tau
 * @return A 2D vector (matrix) with valid embeddings (rows × cols).
 */
std::vector<std::vector<double>> Embed(
    const std::vector<double>& vec,
    int E = 3,
    int tau = 1,
    int style = 0
) {
  const size_t N = vec.size();
  
  // Compute the maximum required lag before embedding
  int max_lag;
  if (tau == 0) {
    max_lag = E - 1;
  } else if (style == 1) {
    max_lag = E * tau;
  } else { // style == 0
    max_lag = (E - 1) * tau;
  }

  // Pre-check: if the largest required lag exceeds available data
  if (max_lag >= static_cast<int>(N)) {
    throw std::invalid_argument(
        "Embedding parameters require a lag larger than available data length."
    );
  }

  const double NaN = std::numeric_limits<double>::quiet_NaN();

  // Preallocate embedding matrix: N rows, E columns
  std::vector<std::vector<double>> mat(N, std::vector<double>(E, NaN));

  for (size_t t = 0; t < N; ++t) {
    for (int j = 0; j < E; ++j) {
      int lag;
      if (tau == 0) {
        lag = j; // Original behavior: 0, 1, ..., E-1
      } else if (style == 1) {
          lag = (j + 1) * tau;   // tau, 2*tau, ..., E*tau
      } else {
          lag = j * tau;         // 0, tau, 2*tau, ..., (E-1)*tau
      }
      int idx = static_cast<int>(t) - lag;
      if (idx >= 0 && idx < static_cast<int>(N)) {
        mat[t][j] = vec[idx];
        // else leave NaN
      }
    }
  }

  // Check which columns contain at least one non-NaN value
  std::vector<bool> keep(E, false);
  for (int j = 0; j < E; ++j) {
    for (size_t i = 0; i < N; ++i) {
      if (!std::isnan(mat[i][j])) {
        keep[j] = true;
        break;
      }
    }
  }

  // If no columns remain, throw exception
  bool any_column = false;
  for (bool b : keep) {
    if (b) {
      any_column = true;
      break;
    }
  }
  if (!any_column) {
    throw std::invalid_argument(
        "Embedding dimension E and lag tau are too large for input length, "
        "no valid embeddings can be generated."
    );
  }

  // Create cleaned matrix with only columns having valid data
  std::vector<std::vector<double>> cleaned;
  for (size_t i = 0; i < N; ++i) {
    std::vector<double> row;
    for (int j = 0; j < E; ++j) {
      if (keep[j]) {
        row.push_back(mat[i][j]);
      }
    }
    cleaned.push_back(row);
  }

  return cleaned;
}


