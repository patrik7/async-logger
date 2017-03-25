#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>

#include <string>

class Logger {

};

/**
 * Extend from this class to have ability of passing it to the logger.
 *
 * Depending on Logger implementation, the class may be copied to the heap. 
 */
class LoggerSerializable {
	public:
					
		//deep copy
		virtual LoggerSerializable* clone(void* placement) const = 0;
		
		virtual ~LoggerSerializable() {}		
	
		//custom function to serialize the class
		virtual void serialize_to_stream(std::ostream& str) const = 0;
		
		virtual size_t get_size() const = 0;
		
		virtual LoggerSerializable* clone() const {
			return this->clone(NULL);
		}
	
};

/**
 * An example of extending the LoggerSerializable.
 *
 * Used to allocate long strings on the heap.
 */
class StringSerializable : public LoggerSerializable {
	std::string string;

	public:
		StringSerializable(const char* string, int length) : string(string, length) {}
		virtual ~StringSerializable() {}
		
		virtual void serialize_to_stream(std::ostream& str) const {
			str << string;
		}
	
		virtual LoggerSerializable* clone(void * placement) const {			
			return placement ? new (placement) StringSerializable(*this) : new StringSerializable(*this);
		}
		
		virtual size_t get_size() const {
			return sizeof(*this);
		}
		
};

#endif
