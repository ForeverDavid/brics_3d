/******************************************************************************
* BRICS_3D - 3D Perception and Modeling Library
* Copyright (c) 2011, GPS GmbH
*
* Author: Sebastian Blumenthal
*
*
* This software is published under a dual-license: GNU Lesser General Public
* License LGPL 2.1 and Modified BSD license. The dual-license implies that
* users of this code may choose which terms they prefer.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License LGPL and the BSD license for
* more details.
*
******************************************************************************/

#ifndef BRICS_3D_MASKROIEXTRACTOR_H_
#define BRICS_3D_MASKROIEXTRACTOR_H_

#include "IFiltering.h"

namespace brics_3d {

class MaskROIExtractor : public IFiltering {
public:
	MaskROIExtractor();
	virtual ~MaskROIExtractor();

	void filter(PointCloud3D* originalPointCloud, PointCloud3D* resultPointCloud);

	void extractIndexedPointCloud(brics_3d::PointCloud3D* inputPoinCloud, std::vector<int> inliers, brics_3d::PointCloud3D* outputPointCloud);

	void extractNonIndexedPointCloud(brics_3d::PointCloud3D* inputPoinCloud, std::vector<int> inliers, brics_3d::PointCloud3D* outputPointCloud);

	void setMask(std::vector<int>* mask);

	void setUseInvertedMask(bool useInvertedMask);

	bool getUseInvertedMask();

private:

	std::vector<int>* mask;

	bool useInvertedMask;

};

}

#endif /* BRICS_3D_MASKROIEXTRACTOR_H_ */

/* EOF */
