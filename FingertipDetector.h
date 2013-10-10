//
//  FingertipDetector.h
//  SequentialTextReader
//
//  Created by roy_shilkrot on 10/10/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#ifndef _FINGERTIPDETECTOR_H
#define _FINGERTIPDETECTOR_H

#include "AbstractAlgorithm.h"

struct FingertipResult {
	Point2f p;
	float probability;
};

class FingertipDetector : public AbstractAlgorithm {
public:
	void train() {}
	void detect() {}
	void bootstrap(const Mat& img) {

	}

private:

};

#endif
