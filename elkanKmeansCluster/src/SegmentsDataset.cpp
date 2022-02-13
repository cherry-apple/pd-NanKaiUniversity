#include <limits>
#include <options.h>
#include "SegmentsDataset.h"

using namespace std;

SegmentsDataset::SegmentsDataset(h5pp::File& h5File_):
    h5File(h5File_)
{
    cachedSegment = -1;

    h5File.readDataset(segmentsNumber,   "/segmentsNumber");
    h5File.readDataset(framesPerSegment, "/framesPerSegment");

    assert( double(segmentsNumber) * double(framesPerSegment) <
            double( numeric_limits<int>::max() ) );

    int framesNumber = segmentsNumber * framesPerSegment;

    options::Options& ops = options::OptionsInstance::get();
    if (ops.presents("ratioOfDatabaseToUse") ){
        framesNumber *= ops.getDouble("ratioOfDatabaseToUse", 0.0);
    }

    setSize(framesNumber);
}

int SegmentsDataset::getSegmentsNumber()
{
    return segmentsNumber;
}

void SegmentsDataset::getSegmentRange(int segmentID, int& startX, int& endX)
{
    startX = segmentID * framesPerSegment;
    endX   = startX + framesPerSegment;
}

Eigen::RowVectorXf SegmentsDataset::operator()(int pointIndex)
{
    assert( pointIndex < size() );

    int segment = pointIndex / framesPerSegment;
    if (segment != cachedSegment ){
        h5File.readDataset(cachedMatrix,
                           fmt::format("segments/{}/features", segment) );
        cachedSegment = segment;
        //cout << "read in segment " << cachedSegment << "\n";
    }else{
        //cout << "hit cache\n";
    }

    int frame = pointIndex % framesPerSegment;
    return cachedMatrix.row(frame);
}
