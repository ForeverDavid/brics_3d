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

#ifdef BRICS_OSG_ENABLE

#include "OSGPointCloudVisualizer.h"
#include "brics_3d/core/ColoredPoint3D.h"
#include <osg/Point>

namespace brics_3d {

/* OSG Helper classes  */

class OSGOperationAdd : public osg::Operation {
public:

	OSGOperationAdd(OSGPointCloudVisualizer* obj, osg::ref_ptr<osg::Node> node, osg::Group* parent = 0) : osg::Operation() {
		this->obj = obj;
		this->node = node;
		this->parent = parent;
	}

	void operator()(osg::Object*) {
		if (parent)
			parent->addChild(node);
		else
			obj->rootGeode->addChild(node);
	}

	OSGPointCloudVisualizer* obj; //volatile?
	osg::ref_ptr<osg::Node> node;
	osg::Group* parent;
};

class OSGOperationClear : public osg::Operation {
public:

	OSGOperationClear(OSGPointCloudVisualizer* obj, osg::Node* node) : osg::Operation() {
		this->obj = obj;
		this->node = node;
	}

	void operator()(osg::Object*) {
		obj->rootGeode->removeChildren(0, obj->rootGeode->getNumChildren());
	}

	OSGPointCloudVisualizer* obj;
	osg::Node* node;
};

class OSGOperationClearButLast : public osg::Operation {
public:

	OSGOperationClearButLast(OSGPointCloudVisualizer* obj) : osg::Operation() {
		this->obj = obj;
	}

	void operator()(osg::Object*) {
//		std::cout << "Number of children before clearance = " << obj->rootGeode->getNumChildren() << std::endl;
		if(obj->rootGeode->getNumChildren() <= 1) { //complete clear
			//leave node as it is
		} else { //leave the first node
			obj->rootGeode->removeChildren(0, obj->rootGeode->getNumChildren()-1);
		}
	}

	OSGPointCloudVisualizer* obj;
};

OSGPointCloudVisualizer::OSGPointCloudVisualizer() {
	init();
}

OSGPointCloudVisualizer::~OSGPointCloudVisualizer() {

}

void OSGPointCloudVisualizer::init() {
	rootGeode = new osg::Group();
    thread = new boost::thread(boost::bind(&OSGPointCloudVisualizer::threadFunction, this, this));
}

struct DrawCallback: public osg::Drawable::DrawCallback {

	DrawCallback() :
		_firstTime(true) {
	}

	virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const {
		osg::State& state = *renderInfo.getState();

		if (!_firstTime) {
			_previousModelViewMatrix = _currentModelViewMatrix;
			_currentModelViewMatrix = state.getModelViewMatrix();
			_inverseModelViewMatrix.invert(_currentModelViewMatrix);

			osg::Matrix T(_previousModelViewMatrix * _inverseModelViewMatrix);

			osg::Geometry * geometry = dynamic_cast<osg::Geometry*> (const_cast<osg::Drawable*> (drawable));
			osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*> (geometry->getVertexArray());
			for (unsigned int i = 0; i + 1 < vertices->size(); i += 2) {
				(*vertices)[i + 1] = (*vertices)[i] * T;
			}
		} else {
			_currentModelViewMatrix = state.getModelViewMatrix();
		}

		_firstTime = false;

		drawable->drawImplementation(renderInfo);
	}

	mutable bool _firstTime;
	mutable osg::Matrix _currentModelViewMatrix;
	mutable osg::Matrix _inverseModelViewMatrix;
	mutable osg::Matrix _previousModelViewMatrix;
};

void OSGPointCloudVisualizer::addPointCloud(PointCloud3D* pointCloud, float red, float green, float blue, float alpha) {
	viewer.addUpdateOperation(new OSGOperationAdd(this, createPointCloudNode(pointCloud, red, green, blue, alpha)));
}

void OSGPointCloudVisualizer::addColoredPointCloud(PointCloud3D* coloredPointCloud, float alpha) {
	viewer.addUpdateOperation(new OSGOperationAdd(this, createColoredPointCloudNode(coloredPointCloud, alpha)));
}

void OSGPointCloudVisualizer::visualizePointCloud(PointCloud3D *pointCloud, float red, float green, float blue, float alpha)
{
	osg::Point* point=new osg::Point;
	point->setSize(2.0f);
	rootGeode->getOrCreateStateSet()->setAttribute(point);

	rootGeode->addChild(createPointCloudNode(pointCloud, red, green, blue, alpha));
	while(!viewer.done()) {
		//nothing here
	}
}

void OSGPointCloudVisualizer::visualizeColoredPointCloud(PointCloud3D *pointCloud, float alpha)
{
	osg::Point* point=new osg::Point;
	point->setSize(2.0f);
	rootGeode->getOrCreateStateSet()->setAttribute(point);

	rootGeode->addChild(createColoredPointCloudNode(pointCloud, alpha));
	while(!viewer.done()) {
		//nothing here
	}
}

void OSGPointCloudVisualizer::clear() {
	viewer.addUpdateOperation(new OSGOperationClear(this, 0));
}

void OSGPointCloudVisualizer::clearButLast() {
	viewer.addUpdateOperation(new OSGOperationClearButLast(this));
}

bool OSGPointCloudVisualizer::done() {
	return viewer.done();
}
osg::ref_ptr<osg::Node> OSGPointCloudVisualizer::createPointCloudNode(PointCloud3D* pointCloud, float red, float green, float blue, float alpha) {

	unsigned int targetNumVertices = 10000; //maximal points per geode

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array(1); //all point have same color
	(*colours)[0].set(red, green, blue, alpha); //set colours (r,g,b,a)

	vertices->reserve(targetNumVertices);
	//normals->reserve(targetNumVertices);
	colours->reserve(targetNumVertices);

	//feed point cloud into osg "geode(s)"
	unsigned int j = 0;
	unsigned int i = 0;
	for (i = 0; i < pointCloud->getSize(); ++i, j += 2) {
//	for (i = 0; i < 60032; ++i, j += 2) { //TODO: strange limit on a windows machine; higher results in crash

		osg::Vec3 tmpPoint;
		tmpPoint.set((float) ((*pointCloud->getPointCloud())[i].getX()), (float) ((*pointCloud->getPointCloud())[i].getY()),
				(float) ((*pointCloud->getPointCloud())[i].getZ()));
		vertices->push_back(tmpPoint);

		/*
		 * If geode gets bigger than 10000 (targetNumVertices) points than create a new child node.
		 * This is necessary to improve the performance due to graphics adapter internals.
		 */
		if (vertices->size() >= targetNumVertices) {
			// finishing setting up the current geometry and add it to the geode.
			geometry->setUseDisplayList(true);
			geometry->setUseVertexBufferObjects(true);
			geometry->setVertexArray(vertices);
			//geometry->setNormalArray(normals);
			//geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
			geometry->setColorArray(colours);
			geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

			geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size())); //GL_POINTS

			geode->addDrawable(geometry);

			// allocate a new geometry
			geometry = new osg::Geometry;

			vertices = new osg::Vec3Array;
			//normals = new osg::Vec3Array;
			//colours = new osg::Vec4ubArray;

			vertices->reserve(targetNumVertices);
			//normals->reserve(targetNumVertices);
			//colours->reserve(targetNumVertices);

		}

	}

	geometry->setUseDisplayList(true);
	geometry->setVertexArray(vertices);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));
	geometry->setColorArray(colours);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
	geometry->setDrawCallback(new DrawCallback);

	geode->addDrawable(geometry);
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Group> group = new osg::Group;
	group->addChild(geode);

	return group;
}

osg::ref_ptr<osg::Node> OSGPointCloudVisualizer::createPointCloudNode(IPoint3DIterator* pointCloudIt, float red, float green, float blue, float alpha) {
	unsigned int targetNumVertices = 10000; //maximal points per geode

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
//	osg::ref_ptr<osg::Vec4ubArray> colours = new osg::Vec4ubArray; //every point has color
	osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array(1); //all point have same color
	(*colours)[0].set(red, green, blue, alpha); //set colours (r,g,b,a)

	vertices->reserve(targetNumVertices);
	colours->reserve(targetNumVertices);

	//feed point cloud into osg "geode(s)"
	for (pointCloudIt->begin(); !pointCloudIt->end(); pointCloudIt->next()) {

		osg::Vec3 tmpPoint;
		tmpPoint.set((float) pointCloudIt->getX(),
				(float) pointCloudIt->getY(),
				(float) pointCloudIt->getZ());
		vertices->push_back(tmpPoint);

		/*
		 * If geode gets bigger than 10000 (targetNumVertices) points than create a new child node.
		 * This is necessary to improve the performance due to graphics adapter internals.
		 */
		if (vertices->size() >= targetNumVertices) {
			// finishing setting up the current geometry and add it to the geode.
			geometry->setUseDisplayList(true);
			geometry->setUseVertexBufferObjects(true);
			geometry->setVertexArray(vertices);
			//geometry->setNormalArray(normals);
			//geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
			geometry->setColorArray(colours);
//			geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
			geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

			geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size())); //GL_POINTS

			geode->addDrawable(geometry);

			// allocate a new geometry
			geometry = new osg::Geometry;

			vertices = new osg::Vec3Array;
			//normals = new osg::Vec3Array;
//			colours = new osg::Vec4ubArray;

			vertices->reserve(targetNumVertices);
			//normals->reserve(targetNumVertices);
			colours->reserve(targetNumVertices);

		}

	}

	geometry->setUseDisplayList(true);
	geometry->setVertexArray(vertices);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));
	geometry->setColorArray(colours);
//	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
	geometry->setDrawCallback(new DrawCallback);

	geode->addDrawable(geometry);
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Group> group = new osg::Group;
	group->addChild(geode);

	return group;
}


osg::ref_ptr<osg::Node> OSGPointCloudVisualizer::createColoredPointCloudNode(PointCloud3D* coloredPointCloud, float alpha) {
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;

	unsigned int targetNumVertices = 10000; //maximal points per geode

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	//osg::Vec3Array* normals = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4ubArray> colours = new osg::Vec4ubArray; //every point has color
//	osg::Vec4Array* colours = new osg::Vec4Array(1); //all point have same color
//	(*colours)[0].set(red, green, blue, alpha); //set colours (r,g,b,a)

	vertices->reserve(targetNumVertices);
	//normals->reserve(targetNumVertices);
	colours->reserve(targetNumVertices);

	//feed point cloud into osg "geode(s)"
	unsigned int j = 0;
	unsigned int i = 0;
	for (i = 0; i < coloredPointCloud->getSize(); ++i, j += 2) {

		osg::Vec3 tmpPoint;
		tmpPoint.set((float) ((*coloredPointCloud->getPointCloud())[i].getX()),
				(float) ((*coloredPointCloud->getPointCloud())[i].getY()),
				(float) ((*coloredPointCloud->getPointCloud())[i].getZ()));
		vertices->push_back(tmpPoint);

		osg::Vec4ub tmpColor;

		/* default color (= white) if no colored decoration layer is found */
		red = 1.0;
		green = 1.0;
		blue = 1.0;

		if ((*coloredPointCloud->getPointCloud())[i].asColoredPoint3D() != 0) { //go for default colors in case no color informatin is attached
			red = (*coloredPointCloud->getPointCloud())[i].asColoredPoint3D()->getR();
			green = (*coloredPointCloud->getPointCloud())[i].asColoredPoint3D()->getG();
			blue = (*coloredPointCloud->getPointCloud())[i].asColoredPoint3D()->getB();
		}

		tmpColor.set(red, green, blue, alpha);
		colours->push_back(tmpColor);

		/*
		 * If geode gets bigger than 10000 (targetNumVertices) points than create a new child node.
		 * This is necessary to improve the performance due to graphics adapter internals.
		 */
		if (vertices->size() >= targetNumVertices) {
			// finishing setting up the current geometry and add it to the geode.
			geometry->setUseDisplayList(true);
			geometry->setUseVertexBufferObjects(true);
			geometry->setVertexArray(vertices);
			//geometry->setNormalArray(normals);
			//geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
			geometry->setColorArray(colours);
			geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

			geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size())); //GL_POINTS

			geode->addDrawable(geometry);

			// allocate a new geometry
			geometry = new osg::Geometry;

			vertices = new osg::Vec3Array;
			//normals = new osg::Vec3Array;
			colours = new osg::Vec4ubArray;

			vertices->reserve(targetNumVertices);
			//normals->reserve(targetNumVertices);
			colours->reserve(targetNumVertices);

		}

	}

	geometry->setUseDisplayList(true);
	geometry->setVertexArray(vertices);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));
	geometry->setColorArray(colours);
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setDrawCallback(new DrawCallback);

	geode->addDrawable(geometry);
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Group> group = new osg::Group;
	group->addChild(geode);

	return group;
}

osg::ref_ptr<osg::Node> OSGPointCloudVisualizer::createColoredPointCloudNode(IPoint3DIterator* pointCloudIt, float alpha) {
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	unsigned int targetNumVertices = 10000; //maximal points per geode

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4ubArray> colours = new osg::Vec4ubArray; //every point has color

	vertices->reserve(targetNumVertices);
	colours->reserve(targetNumVertices);

	Point3D* rawPoint;

	//feed point cloud into osg "geode(s)"
	for (pointCloudIt->begin(); !pointCloudIt->end(); pointCloudIt->next()) {

		osg::Vec3 tmpPoint;
		tmpPoint.set((float) pointCloudIt->getX(),
				(float) pointCloudIt->getY(),
				(float) pointCloudIt->getZ());
		vertices->push_back(tmpPoint);

		osg::Vec4ub tmpColor;

		/* default color (= white) if no colored decoration layer is found */
		red = 1.0;
		green = 1.0;
		blue = 1.0;

		rawPoint =  pointCloudIt->getRawData();

		if (rawPoint->asColoredPoint3D() != 0) { //go for default colors in case no color information is attached
			red = rawPoint->asColoredPoint3D()->getR();
			green = rawPoint->asColoredPoint3D()->getG();
			blue = rawPoint->asColoredPoint3D()->getB();
		}

		tmpColor.set(red, green, blue, alpha);
		colours->push_back(tmpColor);

		/*
		 * If geode gets bigger than 10000 (targetNumVertices) points than create a new child node.
		 * This is necessary to improve the performance due to graphics adapter internals.
		 */
		if (vertices->size() >= targetNumVertices) {
			// finishing setting up the current geometry and add it to the geode.
			geometry->setUseDisplayList(true);
			geometry->setUseVertexBufferObjects(true);
			geometry->setVertexArray(vertices);
			//geometry->setNormalArray(normals);
			//geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
			geometry->setColorArray(colours);
			geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

			geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size())); //GL_POINTS

			geode->addDrawable(geometry);

			// allocate a new geometry
			geometry = new osg::Geometry;

			vertices = new osg::Vec3Array;
			//normals = new osg::Vec3Array;
			colours = new osg::Vec4ubArray;

			vertices->reserve(targetNumVertices);
			//normals->reserve(targetNumVertices);
			colours->reserve(targetNumVertices);

		}

	}

	geometry->setUseDisplayList(true);
	geometry->setVertexArray(vertices);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));
	geometry->setColorArray(colours);
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setDrawCallback(new DrawCallback);

	geode->addDrawable(geometry);
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::Group> group = new osg::Group;
	group->addChild(geode);

	return group;
}


void OSGPointCloudVisualizer::threadFunction(OSGPointCloudVisualizer* obj) {
	boost::this_thread::sleep(boost::posix_time::milliseconds(100)); // Avoid race condition
	viewer.setSceneData(rootGeode);
	viewer.setUpViewInWindow(10, 10, 500, 500);
	//	//viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

	//The viewer.run() method starts the threads and the traversals.
	viewer.run();
}

}

#endif //BRICS_OSG_ENABLE

/* EOF */
