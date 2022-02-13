#include <string>
#include <iostream>
#include <limits>
#include <random>
#include <filesystem>
#include <options.h>
#include <nanotimer.h>
#include <h5pp/h5pp.h>
#include <Eigen/Eigen>
#include <fmt/core.h>
#include <matplot/matplot.h>
#include <matrixConversion.h>
#include "cluster/Clusterer.h"

#include "SegmentsDataset.h"

void testCluster();

using namespace std;
using namespace options;
using namespace Eigen;
using namespace h5pp;

void constructSpectra(SegmentsDataset& dataset, int startX, int endX,
                      vector< vector<double> >& spectra)
{
    int vectorDimension = dataset(startX).cols();
    int frameNumbers = endX - startX;

    // copy input data.
    spectra.resize(vectorDimension);  // number of rows in the spectra image.
    for (int row=0; row<spectra.size(); row++){
        spectra[row].resize( frameNumbers );   // number of colums in the spectra image.
        for (int col=0; col<frameNumbers; col++){
            spectra[row][col] = dataset(col)[vectorDimension - 1 - row];  //flipud.
        }
    }
}

void drawSpectraAndClustering(const vector< vector<double> >& spectra,
                              const vector<double>& assignments)
{
    using namespace matplot;
    gcf()->size(1600, 800);
    static auto h1 = subplot(2, 1, 0);
    image(h1, spectra);
    static auto h2 = subplot(2, 1, 1);
    scatter(h2, iota(0, assignments.size()-1), assignments);
    xlim(h2, {0.0, (double)assignments.size()-1});
    show();
}

int main(int argc, char* argv[])
{
    OptionsInstance instance(argc, argv);
    options::Options& ops = OptionsInstance::get();

    // input
    File inputFeatureFile ( ops.getString("inputFeature"),  FilePermission::READONLY);    
    SegmentsDataset dataset(inputFeatureFile);
    cout << "processing " << ops.getString("inputFeature") << "\n";

    // used in progress reporting.
    string basename = filesystem::path( ops.getString("inputFeature") ).stem().string();

    // output
    string resultFilename = ops.getString("markingResult");
    File resultFile(resultFilename,  FilePermission::REPLACE);

    Clusterer clusterer(dataset);        

    int totalSegments = dataset.getSegmentsNumber();
    vector<int> informativeSegments;    
    // Statistics of the algorithm; used for paper writing.
    long int numberOfDistanceCalculation = 0;
    nanotimer timer;
    timer.start();
    int segmentID;
    for (segmentID=0; segmentID < totalSegments; segmentID++){
        // get range of the current segment.
        // A feature vector of a frame is called a data point.
        // Although the data points are stored in segments, we use their orders in all the
        // points of a h5 file as their absolute indexes, so that logically the h5 file is
        // just a sequence of data points.
        int startX, endX; // indexes of the first and last data points of the current segment.
        dataset.getSegmentRange(segmentID, startX, endX);

        // set epsilon for every segment.
        clusterer.clearNumerOfDistanceCalculation();
        clusterer.setEpsilon(startX, endX);
        numberOfDistanceCalculation += clusterer.getNumberOfDistanceCalculation();
        cout << "numberOfDistanceCalculation for setEpsilon: "
             << clusterer.getNumberOfDistanceCalculation() << "\n";

        //cout << "segment " << segmentID << " epsilon: "
        //     << fmt::format("{:.2f}", clusterer.getEpsilon()) << "\n";

        // process the current segment.
        clusterer.prepare();
        int numberOfDistanceCalculationForClustering = 0;
        for (int x=startX; x<endX; x++){
            //cout << "\nprocessing point #" << x << "\n";
            clusterer.clearNumerOfDistanceCalculation();
            clusterer.cluster(x);            
            numberOfDistanceCalculationForClustering += clusterer.getNumberOfDistanceCalculation();
            // debug
            //clusterer.printStatus();
        }
        numberOfDistanceCalculation += numberOfDistanceCalculationForClustering;
        cout << "numberOfDistanceCalculation for clustering: "
             << numberOfDistanceCalculationForClustering << "\n";

        // get assignments
        // Although each assignment has a type of 'int', we use float here so that this data
        // item can be more easily readed and converted to SonicViewer-readable data.
        vector<float> assignments;
        const vector<CenterID>& assignments_ = clusterer.getAssignments();
        assignments.insert( assignments.end(),
                            assignments_.begin(), assignments_.end() );
        resultFile.writeDataset(assignments,
                            fmt::format("/segments/{}/assignments", segmentID) );

        // get distances to assignments.
        vector<float> distancesToAssignments = clusterer.getDistancesToAssignments();
        resultFile.writeDataset(distancesToAssignments,
                            fmt::format("/segments/{}/distancesToAssignments", segmentID) );

        // get previousCentersNumber
        // since the number is to be viewed as "segment_scalar" which must be a float, we
        // use a type of 'float' here.
        float previousMaxCenterID = max( clusterer.getPreviousCentersNumber() - 1,  0);
        resultFile.writeDataset(previousMaxCenterID,
                            fmt::format("/segments/{}/previousMaxCenterID", segmentID) );

        // get judegement.
        bool isInformative = clusterer.currentSegmentIsInformative();
        if ( isInformative ){
            informativeSegments.push_back(segmentID);
            cout << "[" << basename << "]: "
                 << "segment #" << segmentID << " is informative\n";
        }

        clusterer.updateCenters();        

        // mark centers which should be removed in the end of processing the current segment.
        set<CenterID> centersToBeRemoved;
        if ( !isInformative ){
            // If the current segment is marked as redundant, new clusters should be removed,
            // otherwise, when processing the next segment, points assigned to those clusters
            // will be treated as "redundant" points, but they should be treated as informative
            // points.
            clusterer.markNewCenters(centersToBeRemoved);
        }

        clusterer.markUntouchedCenters(centersToBeRemoved);

        clusterer.removeCenters(centersToBeRemoved);

        // print out number of centers for paper writing.
        cout << "number of centers after removing untouched centers: "
             << clusterer.getCurrentCentersNumber() << "\n";

        // report progress.
        if ( (segmentID+1) % 100 == 0 ){
            cout << "[" << basename << "]: "
                 << "processed "  << segmentID + 1
                 << " of " << totalSegments
                 << " segments(" << int( (segmentID + 1) * 100.0 / totalSegments)  << "%)\n";
        }

        if (ops.presents("numberOfSegmentsToProcess")){
            int numberOfSegmentsToProcess = ops.getInt("numberOfSegmentsToProcess", 0);
            if ( numberOfSegmentsToProcess!=-1 &&
                 (segmentID+1) >= numberOfSegmentsToProcess) {
                cout << "[" << basename << "]: "
                     << "number of processed segments (" << segmentID+1 << ") reaches numberOfSegmentsToProcess("
                     << numberOfSegmentsToProcess <<"), exiting...\n";
                break;
            }
        }
    }
    // print calculate statistics, for paper writing.
    cout << "average number of distance calculation: "
         << numberOfDistanceCalculation / segmentID << "\n";
    cout << "average processing time for each segment: "
         << timer.get_elapsed_ms() / segmentID << " ms\n";
    // The above processing time account for operations including the kernel clustering,
    // and some other extra operations such as reading input data, redandency detection and
    // removal.

    // Save results.
    resultFile.writeDataset(totalSegments, "/segmentsNumber");
    resultFile.writeDataset(informativeSegments, "/informativeSegments");
}
