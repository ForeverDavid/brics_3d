/**
 * @file 
 * Octree.h
 *
 * @date: Feb 18, 2010
 * @author: sblume
 */

#ifndef OCTREE_H_
#define OCTREE_H_

#include "algorithm/filter/IOctree.h"
#include "algorithm/filter/IOctreePartition.h"
#include "algorithm/filter/IOctreeSetup.h"

namespace BRICS_3D {

/**
 * @brief Implementation of the Octree component.
 * @ingroup filtering
 */
class Octree : public IOctree, public IOctreePartition, public IOctreeSetup {
public:

	/**
	 * @brief Standard constructor.
	 */
	Octree();

	/**
	 * @brief Standard destructor.
	 */
	virtual ~Octree();

	void createOctree(PointCloud3D* originalPointCloud, PointCloud3D* resultPointCloud);

	void partitionPointCloud(PointCloud3D* pointCloud, std::vector<PointCloud3D*>* pointCloudCells);

	void setVoxelSize(double voxelSize);

	double getVoxelSize();

private:

	/// The maximum voxel size of the smallest cube in the Octree
	double voxelSize;

};

}

#endif /* OCTREE_H_ */

/* EOF */
