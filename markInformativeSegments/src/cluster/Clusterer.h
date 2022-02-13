#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <set>
#include <deque>
#include "Dataset.h"

using std::vector;
using std::set;
using Eigen::RowVectorXf;
using Eigen::MatrixXf;

typedef uint16_t CenterID;

class Clusterer{
public:
    Clusterer(Dataset& dataset);

    void setEpsilon(int startX, int endX);
    float getEpsilon();

    // Before processing of each segment, this function should be called first.
    void prepare();

    // cluster one data point.
    void cluster(int x);

    bool currentSegmentIsInformative();

    void updateCenters();    

    // functions related to removing centers.
    // append new centers to the list of to-be-removed.
    void markNewCenters(set<CenterID>& centersToBeRemoved);
    // append untouched centers to the list of to-be-removed.
    void markUntouchedCenters(set<CenterID>& centersToBeRemoved);
    void removeCenters(const set<CenterID>& centersToBeRemoved);

    int getPreviousCentersNumber();
    int getCurrentCentersNumber();
    // return the assignments. Some centerIDs may refer to deleted centers.
    const vector<CenterID>& getAssignments();
    const vector<float>& getDistancesToAssignments();

    // only used for paper writing.
    void clearNumerOfDistanceCalculation();
    int getNumberOfDistanceCalculation();

private:
    // higher level functions.

    // find the closest center among the set of centers, return the minimum distance and
    // the center.
    void findClosest(int x, const std::set<CenterID>& centers,
                     float& minDistance, CenterID& closestCenter);

    void updateRecentCenters(CenterID center);

private:    
    // lower level functions.
    float pointToPointDistance(int x1, int x2);
    float pointToCenterDistance(int pointIndex, uint16_t center);
    float centerToCenterDistance(uint16_t center1, uint16_t center2);

public:
    // debug functions.
    void printStatus();

private:
    // information of input points.
    Dataset& dataset;    
    int vectorDimension;
    int N;

    // cluster radius. Any distance betwen a point and a center should be less than this.
    float epsilon;

    // k-th element is the center of the k-th cluster.
    vector< Eigen::RowVectorXf> centers;
    // Shape is (K, vectorDimension).

    const int recentCentersMaxLength = 10;
    std::deque<CenterID> recentCenters;

    // The following structures only store information for the current segment.
    // k-th element is the assignment of the k-th point of the current segment.
    vector<CenterID> assignments;
    // k-th element is the distance of the k-th point to its assignment.
    vector<float> distancesToAssignments;
    // k-th element is the (absolute) index(in the entire dataset) of the k-th points
    // of the current segment.
    vector<int> pointIndexes;

    // set before clustering of the current segment.
    int previousCentersNumber;

    // The following members are for the paper writing;they are not the kernel part of a cluster.
    // The number of distance calculation for each data point.
    int numerOfDistanceCalculation;
};

