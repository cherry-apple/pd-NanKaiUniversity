#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include "Dataset.h"

using std::vector;
using Eigen::RowVectorXf;
using Eigen::MatrixXf;

class ElkanKmeansClusterer {
public:
    ElkanKmeansClusterer(Dataset& dataset, int K_);

    void cluster();

    const vector<RowVectorXf>& getCenters();
    const vector<uint16_t>& getAssignments();    
    vector<int> getClusterSizes();

    // debug functions.
    void printStatus();

    // Only applicable to two-dimensional data points.
    void draw() const;

private:
    // higher level functions.
    // randomly select K data points from the database and set them
    // as the initial centers.
    void setInitialCenters();

    // At the beginning, assign the data points to initial centers.
    void calculateInitialAssignment();

    void runOneIteration();

private:
    // middle level functions.
    // calculate distances between each pair of centers.
    void calculateCentersDistances();

    // For each center, calculate the closest distance to other centers.
    void calculateClosestCenterToCenterDistances();

    // calculate new centers of the current iteration.
    void calculateNewCenters(vector<RowVectorXf>& centersMeans);

private:
    // lower level functions.
    // calculate distance between all pairs of centers.
    float pointToCenterDistance(int pointIndex, uint16_t center) const;
    float centerToCenterDistance(uint16_t center1, uint16_t center2) const;
    float centerToNewCenterDistance(uint16_t center, const RowVectorXf& newCenter) const;

private:
    Dataset& dataset;    
    int vectorDimension;
    int N;   // number of data points.
    int K;   // cluster number.

    // Shape is (K, K). Stores all "d(c,c')".
    MatrixXf centersDistances;

    // Shape is (K). Stores "s(c)".
    RowVectorXf closestCenterToCenterDistance;

    // Shape is (N). Stores "u(x)".
    RowVectorXf upperBounds;

    // Shape is (N, K). Stores "l(x,c)".
    MatrixXf lowerBounds;

    // Shape is (N). For each point in x, keep which cluster it is assigned to. By using a
    // short, we assume a limited number of clusters (fewer than 2^16).
    vector<uint16_t> assignments;

    // Shape is (K). k-th element is the center of the k-th cluster.
    vector< RowVectorXf> centers;

    // whether there is any change of assignment. If so, the cluster has not converged.
    bool assignmentChanged;

    // debug. how many point-center calculations are performed.
    int numberOfDistanceCalculation;
};

