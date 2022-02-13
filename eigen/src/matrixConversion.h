#pragma once
#include <vector>
#include <Eigen/Eigen>

using std::vector;

// STL containers to Eigen matrixes/vectors.
void vecVecToMatrix(const vector<vector<float> >& vecVec,  Eigen::MatrixXf& matrix);

// Eigen matrixes/vectors to STL containers
void toVector(const Eigen::RowVectorXf& source, vector<float>& target);
vector<float> toVector(const Eigen::RowVectorXf& source);

