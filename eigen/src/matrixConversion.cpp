#include "matrixConversion.h"

using namespace Eigen;

void vecVecToMatrix(const vector<vector<float> >& vecVec,  MatrixXf& matrix)
{
    int rows = vecVec.size();
    int cols = vecVec.front().size();
    matrix.resize(rows, cols);
    for (int i=0; i<rows; i++){
        for (int j=0; j<cols; j++){
            matrix(i,j) = vecVec[i][j];
        }
    }
}

void toVector(const Eigen::RowVectorXf& source, vector<float>& target)
{
    target.resize( source.cols() );
    for (int i=0; i<target.size(); i++){
        target[i] = source[i];
    }
}

vector<float> toVector(const Eigen::RowVectorXf& source)
{
    vector<float> v;
    toVector(source, v);
    return v;
}
