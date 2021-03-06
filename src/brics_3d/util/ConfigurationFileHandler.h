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

#ifndef BRICS_3D_CONFIGURATIONFILEHANDLER_H_
#define BRICS_3D_CONFIGURATIONFILEHANDLER_H_


#include <string>

#include "brics_3d/core/Logger.h"

#ifdef BRICS_XERCES_ENABLE
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
XERCES_CPP_NAMESPACE_USE
#endif

namespace brics_3d {

/**
 * @brief Reads a XML configuration file to get parameters for an algorithm
 *
 * The current implementation bases on the Xerces library. If Xerces is not installed all methods will return <code>false</code>.
 * In general the provided functionality is abstract enough that would allow to replace Xerces, so a factory does not directly depend on Xerces.
 */
class ConfigurationFileHandler {
public:

	/**
	 * @brief Constructor
	 * @param filename Specifies the path to the XML configuration file
	 */
	ConfigurationFileHandler(const std::string filename);

	/**
	 * @brief Standard destructor
	 */
	virtual ~ConfigurationFileHandler();

	/**
	 * @brief Get a certain attribute of an algorithm e.g a threshold, etc.
	 * @param algorithm Specifies which algorithm will be searched.
	 *        An algorithm is specified by the tagname of an element in the XML file
	 * @param attribute Specifies which attribute of the algorithm (specified by "algorithm") will be searched.
	 *        An attribute is specified by an attribute of an element in the XML file
	 * @param[out] result Pointer to a string where to store the result.
	 * @result True if attribute is found, else false (or if an error occurred)
	 */
	bool getAttribute(std::string algorithm, std::string attribute, std::string* result);

	/**
	 * @brief Get a certain attribute of an algorithm e.g a threshold, etc.
	 * @param algorithm Specifies which algorithm will be searched.
	 *        An algorithm is specified by the tagname of an element in the XML file
	 * @param attribute Specifies which attribute of the algorithm (specified by "algorithm") will be searched.
	 *        An attribute is specified by an attribute of an element in the XML file
	 * @param[out] result Pointer to an integer where to store the result.
	 * @result True if attribute is found, else false (or if an error occurred)
	 */
	bool getAttribute(std::string algorithm, std::string attribute, int* result);

	/**
	 * @brief Get a certain attribute of an algorithm e.g a threshold, etc.
	 * @param algorithm Specifies which algorithm will be searched.
	 *        An algorithm is specified by the tagname of an element in the XML file
	 * @param attribute Specifies which attribute of the algorithm (specified by "algorithm") will be searched.
	 *        An attribute is specified by an attribute of an element in the XML file
	 * @param[out] result Pointer to a double where to store the result.
	 * @result True if attribute is found, else false (or if an error occurred)
	 */
	bool getAttribute(std::string algorithm, std::string attribute, double* result);

	/**
	 * @brief Get the a sub brics_3d/core/component
	 *
	 * TODO: return XML node ID to allow for arbitrary nested algorithms
	 *
	 * @param algorithm Specifies which algorithm will be searched.
	 *        An algorithm is specified by the tagname of an element in the XML file
	 * @param subalgorithm Specifies which subalgorithm will be searched.
	 *        A subalgorithm is specified by the tagname of a child element in the XML file.
	 * @param[out] result Pointer to a string where to store the result. The returned result ist the "implementation" attribute in the XML file.
	 * @result True if attribute is found, else false (or if an error occurred)
	 *
	 */
	bool getSubAlgorithm(std::string algorithm, std::string subalgorithm, std::string* result);

	/**
	 * @brief Check if errors e.i. return <code>errorsOccured</code>
	 * @return True if errors occurred during parsing in initalization.
	 */
	bool getErrorsOccured();

private:

	/// Stores the path to the XML configuration file
	std::string filename;

	/// Flag that indicates if errors occurred during parsing
	bool errorsOccured;

#ifdef BRICS_XERCES_ENABLE
	/// XML DOM representation
	XercesDOMParser* parser;
#endif

};

}

#endif /* BRICS_3D_CONFIGURATIONFILEHANDLER_H_ */

/* EOF */
