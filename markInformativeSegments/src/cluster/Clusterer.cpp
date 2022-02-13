#include <iostream>
#include <fmt/color.h>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <random>
#include <set>
#include <options.h>
#include <nanotimer.h>
#include "Clusterer.h"
#include "matplot/matplot.h"
#include "matrixConversion.h"
#include "options.h"

using namespace std;
using namespace Eigen;

Clusterer::Clusterer(Dataset& dataset_):
    dataset(dataset_)
{
    vectorDimension = dataset(0).cols();    
    N = dataset.size();    
}

void Clusterer::setEpsilon(int segmentStartX, int segmentEndX)
{
    options::Options& ops = options::OptionsInstance::get();  

    int pointsNumber = std::min<int>( 10,  segmentEndX - segmentStartX);
    vector<float> nearestDistances(pointsNumber, 0.0);
    for (int i=0; i < pointsNumber; i++){
        int x1 = segmentStartX + rand() % (segmentEndX - segmentStartX);
        float minDistance = numeric_limits<float>::max();
        for (int x2=segmentStartX; x2< segmentEndX; x2++){
            if ( x1==x2) continue;
            float distance = pointToPointDistance(x1, x2);
            if (distance < minDistance)
                minDistance = distance;
        }
        nearestDistances[i] = minDistance;
    }

    // find the median
    vector<float>& v= nearestDistances;
    auto middle = v.begin() + v.size()/2;
    nth_element(v.begin(), middle, v.end() );

    float epsilonRatio = ops.getDouble("epsilonRatio", 2.0);
    epsilon = *middle * epsilonRatio;

    // debug
    //cout << "epsilon(" << epsilon << ") = "
    //     << "middle("  << *middle << ") * "
    //     << "epsilonRatio(" << epsilonRatio << ")"
    //     << "\n";
    //cout << "distances are: ";
    //for (float value: v){
    //    cout << value << " ";
    //};
    //cout << "\n";
}

float Clusterer::getEpsilon()
{
    return epsilon;
}


void Clusterer::prepare()
{
    assignments.clear();
    distancesToAssignments.clear();
    pointIndexes.clear();

    previousCentersNumber = centers.size();
}

void Clusterer::updateRecentCenters(CenterID center)
{
    recentCenters.push_back(center);
    while (recentCenters.size() > recentCentersMaxLength){
        recentCenters.pop_front();
    }
}

float Clusterer::pointToPointDistance(int x1, int x2)
{
    numerOfDistanceCalculation++;

    RowVectorXf diff = dataset(x1) - dataset(x2);
    return diff.norm();
}

float Clusterer::pointToCenterDistance(int pointIndex, uint16_t center)
{
    numerOfDistanceCalculation++;

    RowVectorXf diff = dataset(pointIndex) - centers[center];
    return diff.norm();
}

float Clusterer::centerToCenterDistance(uint16_t center1, uint16_t center2)
{
    numerOfDistanceCalculation++;

    RowVectorXf diff = centers[center1] - centers[center2];
    return diff.norm();
}

void Clusterer::printStatus()
{
    cout << "centers:\n";
    for (int c=0; c<centers.size(); c++){
        cout << c << ": ";
        const int L = 4;
        for (int k=0; k<std::min((int)centers[c].cols(), L); k++)
             cout << centers[c][k] << " ";
        if ( centers[c].size() > L )
            cout << "...";
        cout << "\n";
    }    

    cout << "recent centers:\n";
    for (CenterID center: recentCenters){
        cout << center << " ";
    }
    cout << "\n";

    cout << "distances between centers:\n";
    for (int c=0; c<centers.size(); c++){
        for (int j=0; j<=c; j++){
            string number = fmt::format("{:5.3f}",
                                        centerToCenterDistance(c,j) );
            cout << number << " ";
        }
        cout << "\n";
    }
}

void Clusterer::findClosest(int x, const set<CenterID>& centers,
                            float& minDistance, CenterID& closestCenter)
{
    // search the closest.
    for (CenterID center: centers){
        float distance = pointToCenterDistance(x, center);
        if (distance < minDistance){
            minDistance   = distance;
            closestCenter = center;
        }
    }
}

void Clusterer::cluster(int x)
{
    // the centers could be empty, so we should initialize the following variables with
    // some certain values.
    CenterID assignment    = numeric_limits<CenterID>::max();
    float minDistance      = numeric_limits<float>::max();
    CenterID closestCenter = numeric_limits<CenterID>::max();

    // ==== search in recent active centers.
    // construct a set containing recent centers.
    set<CenterID> recentActiveCenters;
    for (CenterID center: recentCenters)
        recentActiveCenters.insert(center);
    // debug
    //cout << "recentActiveCenters: ";
    //for (CenterID center: recentActiveCenters){
    //        cout << center << " ";
    //}
    //cout << "\n";

    //search
    findClosest(x, recentActiveCenters,
                minDistance, closestCenter);
    if ( minDistance < epsilon){
        //cout << "found a match in recent active centers, "
        //     << "distance = " << distance << ", "
        //     << "matched center: " << closestCenter << "\n";
        assignment = closestCenter;
        goto foundMatched;
    }

    {{
    // ==== search in remaining centers if the previous search failed.
    //cout << "could not find any match in recent active centers, "
    //     << "search in remaining centers.\n";
    // construct a set containing remaining centers.
    set<CenterID> remainingCenters;
    for (CenterID c=0; c<centers.size(); c++){
        if ( recentActiveCenters.count(c) == 0){
            remainingCenters.insert(c);
        }
    }
    //cout << "remainingCenters: ";
    //for (CenterID center: remainingCenters){
    //        cout << center << " ";
    //}
    //cout << "\n";

    // search.
    findClosest(x, remainingCenters,
                minDistance, closestCenter);
    if ( minDistance < epsilon){
        //cout << "found a match in remaining centers, "
        //     << "distance = " << distance << ", "
        //     << "matched center: " << closestCenter << "\n";
        assignment = closestCenter;
        goto foundMatched;
    }
    }}

    // ==== construct a new cluster if the previous search still failed.    
    //cout << "could not find any match in all centers, the closest: \n"
    //     << "  min distance   = " << minDistance << "\n"
    //     << "  closest center = " << closestCenter << "\n";
    centers.push_back( dataset(x) );
    assignment  = centers.size() - 1;
    minDistance = 0.0;
    //cout << "created a new center, id = " << assignment << "\n";

foundMatched:
    if (assignment >= centers.size()){
        cout << "illegal condition: assignment = " << assignment << ", but "
             << "centers number = " << centers.size() << "\n";
        exit(-1);
    }
    // update recent centers queue
    updateRecentCenters( assignment );

    assignments.push_back(assignment);
    distancesToAssignments.push_back(minDistance);
    pointIndexes.push_back(x);
}

bool Clusterer::currentSegmentIsInformative()
{
    options::Options& ops = options::OptionsInstance::get();

    // count number of points which are assigned to new centers.
    int newPointsNumber = 0;
    for (CenterID assignment: assignments){
        if (assignment >= previousCentersNumber)
            newPointsNumber++;
    }
    float newPointsRatio = float(newPointsNumber)/ assignments.size();
    bool isInformative = newPointsRatio > ops.getDouble("newPointsRatioThreshold", 0.05);

    return isInformative;
}

int Clusterer::getPreviousCentersNumber()
{
    return previousCentersNumber;
}

const vector<CenterID>& Clusterer::getAssignments()
{
    return assignments;
}

const vector<float>& Clusterer::getDistancesToAssignments()
{
    return distancesToAssignments;
}

void Clusterer::updateCenters()
{
    const int K = centers.size();
    const int vectorDimension = centers.front().cols();

    // initialization.
    MatrixXf clusterVectorSums(K, vectorDimension);
    clusterVectorSums.fill(0.0);
    vector<int> clusterSizes(K, 0);

    // accumulate
    assert( assignments.size() == pointIndexes.size() );
    for (int i=0; i<assignments.size(); i++){
        int assignment = assignments[i];
        clusterVectorSums.row(assignment) += dataset( pointIndexes[i] );
        clusterSizes[assignment]++;
    }

    // update the centers.
    for (int c=0; c<centers.size(); c++){
        centers[c] = clusterVectorSums.row(c) / clusterSizes[c];
    }
}

void Clusterer::markNewCenters(set<CenterID>& centersToBeRemoved)
{
    for (CenterID c=previousCentersNumber; c<centers.size(); c++){
        centersToBeRemoved.insert(c);
    };
}

void Clusterer::markUntouchedCenters(set<CenterID>& centersToBeRemoved)
{
    set<CenterID> touchedCenters;
    for (int i=0; i<assignments.size(); i++){
        int assignment = assignments[i];
        touchedCenters.insert(assignment);
    }
    for (CenterID c=0; c<centers.size(); c++){
        if ( touchedCenters.count(c) == 0){
            centersToBeRemoved.insert(c);
        }
    }
}

void Clusterer::removeCenters(const set<CenterID>& centersToBeRemoved)
{
    for (int c=centers.size()-1; c>=0; c--){
        if ( centersToBeRemoved.count(c) > 0) {
            // remove the center.
            //cout << "to remove center " << c << "\n";
            centers.erase( centers.begin() + c );
        }
    }

    // clear the list of recent active centers, since
    // the centerIDs in it may refer to removed clusters.
    recentCenters.clear();
}

int Clusterer::getCurrentCentersNumber()
{
    return centers.size();
}

void Clusterer::clearNumerOfDistanceCalculation()
{
    numerOfDistanceCalculation = 0;
}

int Clusterer::getNumberOfDistanceCalculation()
{
    return numerOfDistanceCalculation;
}

