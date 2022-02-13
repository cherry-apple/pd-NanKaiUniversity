#include <iostream>
#include <string>
#include <vector>
#include <options.h>
#include <h5pp/h5pp.h>
#include <fmt/core.h>
#include "matplot/matplot.h"
#include "matrixConversion.h"
#include "ErrorMeasures.h"

using namespace std;
using namespace options;
using namespace h5pp;

int main(int argc, char* argv[])
{
    OptionsInstance instance(argc, argv);
    options::Options& ops = OptionsInstance::get();

    File residualsFile    ( ops.getString("residuals"),      FilePermission::READONLY);
    File errorMeasuresFile( ops.getString("errorMeasures"),  FilePermission::REPLACE);
    cout << "calculate error measures: \n"
         << "  residuals       : " << ops.getString("residuals") << "\n"
         << "  error measures  : " << ops.getString("errorMeasures") << "\n";

    // set segmentsNumber.
    int segmentsNumber;
    residualsFile.readDataset(segmentsNumber, "/segmentsNumber");
    if ( ops.presents("numberOfSegmentsToProcess")){
        int numberOfSegmentsToProcess = ops.getInt("numberOfSegmentsToProcess", -1);
        if (numberOfSegmentsToProcess != -1 &&
            numberOfSegmentsToProcess < segmentsNumber ){
            segmentsNumber = numberOfSegmentsToProcess;
        }
    }

    for (int segmentID=0; segmentID < segmentsNumber; segmentID++){
        // read residuals
        Eigen::MatrixXf residuals;
        residualsFile.readDataset(residuals,
                                  fmt::format("segments/{}/residuals", segmentID) );        
        int T = residuals.rows();

        // calculate error measures
        ErrorMeasures errorMeasures;
        vector<float> means(T);
        vector<float> pnrs(T);
        vector<float> errors(T);
        for (int t=0; t<T; t++){
            errorMeasures.getMeasures( toVector( residuals.row(t) ),
                                       means[t], pnrs[t], errors[t]);
        }

        // output measures
        errorMeasuresFile.writeDataset(means,
                                   fmt::format("segments/{}/means",  segmentID) );
        errorMeasuresFile.writeDataset(pnrs,
                                   fmt::format("segments/{}/pnrs",   segmentID) );
        errorMeasuresFile.writeDataset(errors,
                                   fmt::format("segments/{}/errors", segmentID) );
    }

    errorMeasuresFile.writeDataset(segmentsNumber, "/segmentsNumber");
}
