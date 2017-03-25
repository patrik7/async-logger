#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>

#include <string>

class Logger {
	public:

};

class LoggerSerializable {
	public:
		virtual void serialize_to_stream(std::ostream& str) const = 0;
		
		virtual LoggerSerializable* clone() const = 0;
		
		virtual ~LoggerSerializable() {}
		
};

class StringSerializable : public LoggerSerializable {
	std::string string;

	public:
		StringSerializable(const char* string) : string(string) {}
		virtual ~StringSerializable() {}
		
		virtual void serialize_to_stream(std::ostream& str) const {
			str << string;
		}
	
		virtual LoggerSerializable* clone() const {
			return new StringSerializable(*this);
		}
		
};

#endif
