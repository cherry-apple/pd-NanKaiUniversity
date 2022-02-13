#include <string>
#include <iostream>
#include <limits>
#include <random>
#include <options.h>
#include <h5pp/h5pp.h>
#include <Eigen/Eigen>
#include <fmt/core.h>
#include <matplot/matplot.h>
#include <matrixConversion.h>
#include "cluster/ElkanKmeansClusterer.h"
#include "SegmentsDataset.h"

void clusterSythesizedData();

using namespace std;
using namespace options;
using namespace Eigen;
using namespace h5pp;

void clusterAudioData()
{
    options::Options& ops = OptionsInstance::get();
    File inputFeatureFile ( ops.getString("inputFeature"),  FilePermission::READONLY);
    SegmentsDataset dataset(inputFeatureFile);
    cout << "data points : " << dataset.size() << "\n";

    // cluster the points.
    ElkanKmeansClusterer clusterer(dataset, 16);
    clusterer.cluster();
}

int main(int argc, char* argv[])
{
    OptionsInstance instance(argc, argv);    

    // If you want get an intuitive impression on the clustering algorithm, you can
    // uncomment the following line. The function synthesizes some two-dimensional points,
    // performs clustering on them, and draws a figure to show the clustering result.
    //clusterSythesizedData();

    clusterAudioData();
}
