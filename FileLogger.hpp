#ifndef FILELOGGER_HPP
#define FILELOGGER_HPP

#include "Logger.hpp"

#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <fstream>



/**
 * The FileLogger class implements the logging interface of the Logger.
 *
 * Assuming, the data are being logged from high performing code.
 * Timeconsuming and blocking operations are moved to logging thread.
 *
 * FileLogger uses a non-blocking queue to store non-serialized data.
 * The queue is then emptied, serialized and pushed to a file on a separate thread.
 *
 */

typedef enum types {
	INTEGER,
	FLOATING,
	SHORT_CHAR,
	SERIALIZABLE
} types_enum;

class FileLogEntry {
	private:
		types_enum type;
		
		union {
			char short_char[255];
			long long integer;
			double floating;
			LoggerSerializable* serializable;
		} data;
		
	public:

		FileLogEntry() : type(INTEGER) {
			this->data.integer = 0;		
		}

		FileLogEntry(int a) : type(INTEGER) {
			this->data.integer = a;		
		}

		FileLogEntry(long a) : type(INTEGER) {
			this->data.integer = a;		
		}
		
		FileLogEntry(long long a) : type(INTEGER) {
			this->data.integer = a;		
		}

		FileLogEntry(double a) : type(FLOATING) {
			this->data.floating = a;		
		}

		FileLogEntry(const char *a) : type(SHORT_CHAR) {
			int length = strnlen(a, 1024);
		
			if(length < sizeof(this->data.short_char)) {
				strncpy(this->data.short_char, a, sizeof(this->data.short_char));				
			} else {
			
			}
		}
		
		void log_to_stream(std::ostream& log) const {
			switch(this->type) {
				case INTEGER:      log << this->data.integer << "\n"; break;
				case FLOATING:     log << this->data.floating << "\n"; break;
				case SHORT_CHAR:   log << this->data.short_char << "\n"; break;
				case SERIALIZABLE: this->data.serializable->serialize_to_stream(log); log << "\n"; break;
			}		
		}
	
};

class FileLogger : Logger {

	private:
		boost::lockfree::queue<FileLogEntry> queue;
		boost::thread *thread;

		void queue_sync();
		volatile bool shut_down_signal;
		
		std::ofstream log_file;
		
	public:
		FileLogger(const char* file_name) :
			Logger(),
			queue(128),
			thread(NULL),
			shut_down_signal(false)
		{
			log_file.open(file_name);
			if(log_file.is_open()) {
				thread = new boost::thread(&FileLogger::queue_sync, this);
			}
		}
		
		void log(const FileLogEntry& entry) {
			queue.push(entry);
		}
		
		virtual ~FileLogger() {
			if(thread) {
				shut_down_signal = true;

				thread->join();
			
				delete thread;
			}
		}
};

FileLogger& operator<<(FileLogger& logger, int a);
FileLogger& operator<<(FileLogger& logger, double a);
FileLogger& operator<<(FileLogger& logger, const char* a);

#endif
