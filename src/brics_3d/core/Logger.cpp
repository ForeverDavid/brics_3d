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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/time.h>
#include <iomanip>
#include "Logger.h"
#ifdef WIN32
#include "windows.h"
#endif

using namespace std;

namespace brics_3d {

Logger::Loglevel Logger::minLoglevel = Logger::WARNING;
bool Logger::useFilename = false;
bool Logger::useLogfile = false;
bool Logger::useTime = true;
std::string Logger::logfilename;
Logger::Listener* Logger::listener = 0;
std::ofstream Logger::logfile;

Logger::Logger(Loglevel level) {
	needNewline = false;
	this->level = level;
	isAboveLoglevel = (level >= minLoglevel);
}

Logger::~Logger() {
	if (isAboveLoglevel)
		flush();
}

void Logger::setMinLoglevel(Loglevel minLoglevel) {
	Logger::minLoglevel = minLoglevel;
}

Logger::Loglevel Logger::getMinLoglevel() {
	return Logger::minLoglevel;
}

void Logger::setUseFilename(bool useFilenames) {
	Logger::useFilename = useFilenames;
}

void Logger::setUseTime(bool useTime) {
	Logger::useTime = useTime;
}

void Logger::setLogfile(std::string filename, bool append) {
	if (useLogfile)
		logfile.close();

	useLogfile = true;
	ios_base::openmode mode = ios::out;
	if (!append)
		mode |= ios::trunc;
	else
		mode |= ios::app;
	logfile.open(filename.c_str(), mode);
}

void Logger::setListener(Listener* listener) {
	Logger::listener = listener;
}

std::string& Logger::levelToString(Loglevel loglevel) {
	static std::string names[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
	return names[loglevel];
}
	
std::ostream& Logger::write(std::string filename, int line) {
	if (!isAboveLoglevel)
		return stream; //todo: should return stream which does not do anything

	if (useTime) {
		struct timeval thisTime;
		gettimeofday(&thisTime, 0);
		long double timeInSec = static_cast<long double>( thisTime.tv_sec + thisTime.tv_usec * 1.0e-6);

		stream.precision(3);
		stream << std::fixed << std::setw(8) << std::right << timeInSec << " ";
	}

	stream << "[" << levelToString(level);
	if (useFilename)
		stream << " " << filename << "(" << line << ")";
	stream << "] ";
	needNewline = true;

	return stream;
}

void Logger::flush() {
	if (needNewline)
		stream << endl;

	if (useLogfile) {
		logfile << stream.str();
		logfile.flush();
	}

	if (listener)
		listener->write(level, stream.str());

	switch (level) {
	case FATAL: 
#ifdef WIN32
		MessageBox(0, stream.str().c_str(), "BRICS_3D Fatal Error", MB_OK|MB_ICONERROR);
#else
		cerr << stream.str();
#endif
		exit(0);
	case LOGERROR: 
		cerr << stream.str();
		break;
	default:
	 	cout << stream.str();
	}

}

} //namespace brics_3d
