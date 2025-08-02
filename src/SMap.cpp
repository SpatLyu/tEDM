#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <limits>
#include "CppStats.h"

/**
 * @brief Perform S-Map prediction using locally weighted linear regression.
 *
 * This function performs prediction based on a reconstructed state-space (time-delay embedding).
 * For each prediction index, it:
 *   - Finds the nearest neighbors from the library indices, excluding the current prediction index.
 *   - Computes distance-based weights using the S-map weighting parameter (theta).
 *   - Constructs a locally weighted linear regression model using the valid neighbors.
 *   - Predicts the target value using the derived local model.
 *
 * @param vectors        A 2D matrix where each row is a reconstructed state vector (embedding).
 * @param target         A vector of scalar values to predict (e.g., time series observations).
 * @param lib_indices    Indices of the vectors used as the library (neighbor candidates).
 * @param pred_indices   Indices of the vectors used for prediction.
 * @param num_neighbors  Number of nearest neighbors to use in local regression.
 * @param theta          Weighting parameter controlling exponential decay of distances.
 * @return std::vector<double> Predicted values aligned with the input target vector.
 *         Entries at non-prediction indices or with insufficient valid neighbors are NaN.
 */
std::vector<double> SMapPrediction(
    const std::vector<std::vector<double>>& vectors,
    const std::vector<double>& target,
    const std::vector<int>& lib_indices,
    const std::vector<int>& pred_indices,
    int num_neighbors,
    double theta
) {
  size_t N = target.size();
  std::vector<double> pred(N, std::numeric_limits<double>::quiet_NaN());

  if (num_neighbors <= 0 || lib_indices.empty() || pred_indices.empty()) {
    return pred;
  }

  for (int pred_i : pred_indices) {
    // // Skip if target at prediction index is NaN
    // if (std::isnan(target[pred_i])) {
    //   continue;
    // }

    // Compute distances only for valid vector pairs with valid target values
    std::vector<double> distances;
    std::vector<int> valid_libs;

    for (int i : lib_indices) {
      // // Only use neighbors with valid target
      // if (std::isnan(target[i])) continue;

      if (i == pred_i) continue; // Skip self-matching

      double sum_sq = 0.0;
      double count = 0.0;
      for (size_t j = 0; j < vectors[pred_i].size(); ++j) {
        double vi = vectors[i][j];
        double vj = vectors[pred_i][j];
        if (!std::isnan(vi) && !std::isnan(vj)) {
          sum_sq += (vi - vj) * (vi - vj);
          count += 1.0;
        }
      }

      if (count > 0) {
        distances.push_back(std::sqrt(sum_sq / count));
        valid_libs.push_back(i);
      }
    }

    if (distances.empty()) {
      continue; // no usable neighbors
    }

    size_t actual_neighbors = std::min(static_cast<size_t>(num_neighbors), distances.size());

    // Compute mean distance
    double mean_distance = std::accumulate(distances.begin(), distances.end(), 0.0) / distances.size();

    // Compute weights using exponential kernel
    std::vector<double> weights(distances.size(), 0.0);
    for (size_t i = 0; i < distances.size(); ++i) {
      weights[i] = std::exp(-theta * distances[i] / mean_distance);
    }

    // Select top-k neighbors using partial sort
    std::vector<size_t> neighbor_indices(distances.size());
    std::iota(neighbor_indices.begin(), neighbor_indices.end(), 0);
    std::partial_sort(
      neighbor_indices.begin(),
      neighbor_indices.begin() + actual_neighbors,
      neighbor_indices.end(),
      [&](size_t a, size_t b) {
        return (distances[a] < distances[b]) ||
          (distances[a] == distances[b] && a < b);
      }
    );

    // Construct weighted linear system A * coeff = b
    size_t dim = vectors[pred_i].size();
    std::vector<std::vector<double>> A(actual_neighbors, std::vector<double>(dim + 1, 0.0));
    std::vector<double> b(actual_neighbors, 0.0);

    for (size_t i = 0; i < actual_neighbors; ++i) {
      int idx = valid_libs[neighbor_indices[i]];
      double w = weights[neighbor_indices[i]];
      for (size_t j = 0; j < dim; ++j) {
        A[i][j] = vectors[idx][j] * w;
      }
      A[i][dim] = w;  // bias term
      b[i] = target[idx] * w;
    }

    // Solve the system using SVD
    std::vector<std::vector<std::vector<double>>> svd_result = CppSVD(A);
    std::vector<std::vector<double>> U = svd_result[0];
    std::vector<double> S = svd_result[1][0];
    std::vector<std::vector<double>> V = svd_result[2];

    // Invert singular values with tolerance
    double max_s = *std::max_element(S.begin(), S.end());
    std::vector<double> S_inv(S.size(), 0.0);
    for (size_t i = 0; i < S.size(); ++i) {
      if (S[i] >= max_s * 1e-5) {
        S_inv[i] = 1.0 / S[i];
      }
    }

    // Compute regression coefficients: V * S_inv * U^T * b
    std::vector<double> coeff(dim + 1, 0.0);
    for (size_t k = 0; k < V.size(); ++k) {
      double temp = 0.0;
      for (size_t j = 0; j < S_inv.size(); ++j) {
        for (size_t i = 0; i < U.size(); ++i) {
          temp += V[k][j] * S_inv[j] * U[i][j] * b[i];
        }
      }
      coeff[k] = temp;
    }

    // Compute prediction: dot(coeff, input) + bias
    double prediction = 0.0;
    for (size_t i = 0; i < dim; ++i) {
      prediction += coeff[i] * vectors[pred_i][i];
    }
    prediction += coeff[dim];

    pred[pred_i] = prediction;
  }

  return pred;
}

/*
 * Computes the Rho value using the 'S-Maps' prediction method.
 *
 * Parameters:
 *   - vectors: Reconstructed state-space (each row is a separate vector/state).
 *   - target: Time series data vector to be predicted.
 *   - lib_indices: Vector of integer indices specifying which states to use for finding neighbors.
 *   - pred_indices: Vector of integer indices specifying which states to predict.
 *   - num_neighbors: Number of neighbors to use for S-Map.
 *   - theta: Weighting parameter for distances.
 *
 * Returns: The Pearson correlation coefficient (Rho) between predicted and actual values.
 */
double SMap(
    const std::vector<std::vector<double>>& vectors,
    const std::vector<double>& target,
    const std::vector<int>& lib_indices,
    const std::vector<int>& pred_indices,
    int num_neighbors,
    double theta
) {
  double rho = std::numeric_limits<double>::quiet_NaN();

  // Call SMapPrediction to get the prediction results
  std::vector<double> target_pred = SMapPrediction(vectors, target, lib_indices, pred_indices, num_neighbors, theta);

  if (checkOneDimVectorNotNanNum(target_pred) >= 3) {
    rho = PearsonCor(target_pred, target, true);
  }
  return rho;
}


/*
 * Computes the S-Map prediction and evaluates prediction performance.
 *
 * Parameters:
 *   - vectors: Reconstructed state-space (each row is a separate vector/state).
 *   - target: Time series data vector to be predicted.
 *   - lib_indices: Vector of integer indices specifying which states to use for finding neighbors.
 *   - pred_indices: Vector of integer indices specifying which states to predict.
 *   - num_neighbors: Number of neighbors to use for S-Map.
 *   - theta: Weighting parameter for distances.
 *
 * Returns: A vector<double> containing {Pearson correlation, MAE, RMSE}.
 */
std::vector<double> SMapBehavior(
    const std::vector<std::vector<double>>& vectors,
    const std::vector<double>& target,
    const std::vector<int>& lib_indices,
    const std::vector<int>& pred_indices,
    int num_neighbors,
    double theta
) {
  // Initialize PearsonCor, MAE, and RMSE
  double pearson = std::numeric_limits<double>::quiet_NaN();
  double mae = std::numeric_limits<double>::quiet_NaN();
  double rmse = std::numeric_limits<double>::quiet_NaN();

  // Call SMapPrediction to get the prediction results
  std::vector<double> target_pred = SMapPrediction(vectors, target, lib_indices, pred_indices, num_neighbors, theta);

  if (checkOneDimVectorNotNanNum(target_pred) >= 3) {
    // Compute PearsonCor, MAE, and RMSE
    pearson = PearsonCor(target_pred, target, true);
    mae = CppMAE(target_pred, target, true);
    rmse = CppRMSE(target_pred, target, true);
  }

  // Return the three metrics as a vector
  return {pearson, mae, rmse};
}
