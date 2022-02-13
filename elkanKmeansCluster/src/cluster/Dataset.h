#pragma once

#include <iostream>
#include <Eigen/Eigen>

// This abstract class represents a set of all data points to be processed by
// a k-means algorithm. Clients should derive a sub-class to describe data points.
// We use an Eigen::RowVectorXf to represent a data point.
class Dataset {
public:
    Dataset() : pointsNumber(0){};

    int size() const{
        return pointsNumber;
    };

    virtual Eigen::RowVectorXf operator()(int pointIndex) = 0;

protected:
    void setSize(int pointsNumber_) {
        pointsNumber = pointsNumber_;
    };

private:
    int pointsNumber;  // number of data points.
};
