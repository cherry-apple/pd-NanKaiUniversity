#include <iostream>
#include <cmath>
#include <cassert>
#include <set>
#include <algorithm>
#include <random>
#include <options.h>
#include <nanotimer.h>
#include "ElkanKmeansClusterer.h"
#include "matplot/matplot.h"
#include "matrixConversion.h"

using namespace std;
using namespace Eigen;

ElkanKmeansClusterer::ElkanKmeansClusterer(Dataset& dataset_, int K_):
    dataset(dataset_)
{
    vectorDimension = dataset(0).cols();
    N = dataset.size();
    K = K_;

    centersDistances.resize(K, K);
    closestCenterToCenterDistance.resize(K);

    lowerBounds.resize(N, K);
    upperBounds.resize(N);

    assignments.resize(N);
    centers.resize(K);
}

float ElkanKmeansClusterer::pointToCenterDistance(int pointIndex, uint16_t center) const
{
    RowVectorXf diff = dataset(pointIndex) - centers[center];
    return diff.norm();
}

float ElkanKmeansClusterer::centerToCenterDistance(uint16_t center1, uint16_t center2) const
{
    RowVectorXf diff = centers[center1] - centers[center2];
    return diff.norm();
}

float ElkanKmeansClusterer::centerToNewCenterDistance(uint16_t center,
                                    const RowVectorXf& newCenter) const
{
    RowVectorXf diff = centers[center] - newCenter;
    return diff.norm();
}

const vector< Eigen::RowVectorXf>& ElkanKmeansClusterer::getCenters()
{
    return centers;
}

const vector<uint16_t>& ElkanKmeansClusterer::getAssignments()
{
    return assignments;
}

vector<int> ElkanKmeansClusterer::getClusterSizes()
{
    vector<int> clusterSizes(K, 0);
    for (int x=0; x<N; x++){
        uint16_t cx = assignments[x];
        clusterSizes[cx]++;
    }

    return clusterSizes;
}

void ElkanKmeansClusterer::printStatus()
{
    cout << "centers and their info:\n";
    vector<int> clusterSizes = getClusterSizes();
    for (int c=0; c<K; c++){
        cout << c << ": " << centers[c] << "\n"
             << "  cluster size: " << clusterSizes[c] << "\n";
    }
    cout << "\n";

    cout << "data points and their info:\n";
    for (int x=0; x<N; x++){
        uint16_t cx = assignments[x];
        float assignmentDistance = pointToCenterDistance(x, cx);
        cout << x << ": " << dataset(x) << "\n"
             << "  assignment: " << cx << "\n"
             << "  assignmentDistance: " << assignmentDistance  << "\n";
    }
    cout << "\n";
}

void ElkanKmeansClusterer::draw() const
{
    using namespace matplot;

    hold(on);
    xlim({-1, 1});
    ylim({-1, 1});

    // show data points.
    vector<double> x;
    vector<double> y;
    vector<double> size;
    vector<double> color;
    for (int pointIndex = 0; pointIndex < dataset.size(); pointIndex++){
        Eigen::RowVectorXf point = dataset(pointIndex);
        x.push_back(point[0]);
        y.push_back(point[1]);
        size.push_back(4);
        color.push_back(assignments[pointIndex]);
    }
    scatter( x, y, size, color);

    // show centers.
    x.clear(); y.clear(); size.clear(); color.clear();    
    for (int c=0; c<K; c++){
        x.push_back(centers[c][0]);
        y.push_back(centers[c][1]);
        size.push_back(20);
        color.push_back(c);
    }
    auto l = scatter( x, y, size, color);
    l->marker_style(line_spec::marker_style::cross);

    show();
}

void ElkanKmeansClusterer::setInitialCenters()
{
    default_random_engine engine;
    uniform_int_distribution<int> uniformDist(0, N - 1);
    set<int> chosenIndexes;

    centers.resize(K);
    for (int c=0; c<K; c++){
        // get a random but unseen index.
        int randomPointIndex;
        do{
            randomPointIndex = uniformDist(engine);
        }while ( chosenIndexes.count(randomPointIndex)>0 );

        chosenIndexes.insert(randomPointIndex);
        centers[c] = dataset(randomPointIndex);
    }
}

void ElkanKmeansClusterer::calculateCentersDistances()
{
    for (int c=0; c<K; c++){
        for (int k=0; k<K; k++){
            centersDistances(c, k) = centerToCenterDistance(c, k);
        }
    }
}

void ElkanKmeansClusterer::calculateClosestCenterToCenterDistances()
{
    for (int c=0; c<K; c++){
        float minDistance = numeric_limits<float>::max();
        for (int k=0; k<K; k++){
            if (k==c) continue;
            if (centersDistances(c,k) < minDistance ){
                minDistance = centersDistances(c,k);
            }
        }
        closestCenterToCenterDistance[c] = 0.5 * minDistance;
    }
}

void ElkanKmeansClusterer::calculateInitialAssignment()
{
    calculateCentersDistances();
    for (int x=0; x<N; x++){
        // Initially assign the first center as the closest.
        uint16_t cx = 0;
        float minDistance  = pointToCenterDistance(x, cx);
        lowerBounds(x, cx) = minDistance;

        // find the closest center.
        for (int c=1; c<K; c++){
            // computation avoided if the center is too far from the current assignment.
            if ( centersDistances(c, cx) > 2 * minDistance){
                continue;
            }

            float distance = pointToCenterDistance(x, c);
            lowerBounds(x, c) = distance;
            if (distance < minDistance){
                cx = c;
                minDistance = distance;
            }
        }

        // store its assignment.
        assignments[x] = cx;
        upperBounds[x] = minDistance;
    }
}

void ElkanKmeansClusterer::calculateNewCenters(vector<RowVectorXf>& newCenters)
{
    // The new centers first act as an accumulating vector.
    newCenters.resize(K);
    for (int c=0; c<K; c++){
        newCenters[c].resize(vectorDimension);
        newCenters[c].fill(0.0);
    }

    // accumulate the data points to the centers.
    vector<int> clustersSizes(K, 0);
    for (int x=0; x<N; x++){
        int cx = assignments[x];
        clustersSizes[cx]++;
        newCenters[cx] += dataset(x);
    }

    // dividing by cluster sizes to produce new centers.
    for (int c=0; c<K; c++){
        newCenters[c] /= clustersSizes[c];
    }
}

void ElkanKmeansClusterer::runOneIteration()
{
    assignmentChanged = false;
    numberOfDistanceCalculation = 0;

    // step 1.
    calculateCentersDistances();
    calculateClosestCenterToCenterDistances();

    for (int x=0; x<N; x++){
        // step 2
        int cx = assignments[x];
        if ( upperBounds[x] <= closestCenterToCenterDistance[cx] ){
            // all other centers are too far away from the current assignment, so
            // should keep the current assignment.
            continue;
        }

        // step 3
        //nanotimer timer;
        //timer.start();
        float distanceToCurrentAssignment;
        bool hasCalculatedDistanceToCurrentAssignment = false;
        for (int c=0; c<K; c++){
            if (c==cx) continue;
            if (upperBounds[x] < lowerBounds(x,c) ||
                upperBounds[x] < 0.5 * centersDistances(cx, c) ){
                // the calculation d(x,c) can be avoided.
                continue;
            }

            // calculate distance to the current assignment.
            if ( ! hasCalculatedDistanceToCurrentAssignment ){
                distanceToCurrentAssignment = pointToCenterDistance(x, cx);
                upperBounds[x]    = distanceToCurrentAssignment;
                lowerBounds(x,cx) = distanceToCurrentAssignment;

                hasCalculatedDistanceToCurrentAssignment = true;
            }

            // calculate d(x,c).
            float distance = pointToCenterDistance(x,c);
            lowerBounds(x,c) = distance;
            numberOfDistanceCalculation++;

            if (distance < distanceToCurrentAssignment){
                // change assignment.
                cx = c;
                assignments[x] = cx;
                distanceToCurrentAssignment = distance;

                assignmentChanged = true;
            }
        }
        //cout << "step 3 spent: " << timer.get_elapsed_us() << "\n";

        // step 4
        //timer.start();
        vector<RowVectorXf> newCenters;
        calculateNewCenters(newCenters);
        vector<float> centerMovements(K);
        for (int c=0; c<K; c++){
            centerMovements[c] = centerToNewCenterDistance(c, newCenters[c] );
        }
        //cout << "step 4 spent: " << timer.get_elapsed_us() << "\n";

        // step 5
        //timer.start();
        for (int x=0; x<N; x++){
            for (int c=0; c<K; c++){
                lowerBounds(x,c) = std::max(0.0f, lowerBounds(x,c) - centerMovements[c] );
            }
        }
        //cout << "step 5 spent: " << timer.get_elapsed_us() << "\n";

        // step 6.
        for (int x=0; x<N; x++){
            uint16_t cx = assignments[x];
            upperBounds[x] += centerMovements[cx];
        }

        // step 7.
        centers = newCenters;

        // report progress.
        if (N>1000){
            if ( (x+1) % (300) == 0){
                cout << "processing data point " << x << " of " << N
                     << " (" << int( float(x*100)/N) << "%)\n";
            }
        }
    }
}

void ElkanKmeansClusterer::cluster()
{
    nanotimer timer;
    timer.start();

    // preparation.
    // Randomly select some data points as initial centers.
    setInitialCenters();
    // initialize lower bounds.
    for (int x=0; x<N; x++){
        for (int c=0; c<K; c++){
            lowerBounds(x, c) = 0.0;
        }
    }
    calculateInitialAssignment();

    // iterations.
    int totalNumberOfDistanceCalculation = 0;
    for (int iteration=0; ; iteration++){
        cout << "==== iteration #" << iteration << " ====\n";
        runOneIteration();        
        cout << "numberOfDistanceCalculation = " << numberOfDistanceCalculation << "\n";
        totalNumberOfDistanceCalculation += numberOfDistanceCalculation;
        if (! assignmentChanged ) break;
    }
    //printStatus();
    //draw();
    cout << "clustering converged.\n";
    cout << "total number of distance calculation: " << totalNumberOfDistanceCalculation << "\n";

    cout << "total clustering spent: " << timer.get_elapsed_ms() << " ms\n";
}
