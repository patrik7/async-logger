#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>

class Logger {
	public:

};

class LoggerSerializable {
	public:
		virtual void serialize_to_stream(std::ostream& str) const = 0;
		
		virtual void release() {
		
		}
};

#endif
