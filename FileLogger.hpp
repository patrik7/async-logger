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
	SERIALIZABLE_INPLACE, //optiomization to save memory allocation for small objects
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
				//short strings are kept in the queue
			
				strncpy(this->data.short_char, a, sizeof(this->data.short_char));				
			} else {
				//long strings are copied to the heap
			
				this->type = SERIALIZABLE;
				this->data.serializable = new StringSerializable(a);
			}
		}

		FileLogEntry(LoggerSerializable * const s) : type(SERIALIZABLE) {
			if(s->get_size() <= sizeof(this->data)) {				
				s->clone(&this->data);
				type = SERIALIZABLE_INPLACE;
			} else {
				this->data.serializable = s->clone(); //allocates copy on the heap
			}
		}
		
		void log_to_stream(std::ostream&) const;
	
};

class FileLogger : public Logger {

	private:
		boost::lockfree::queue<FileLogEntry> queue;
		boost::thread *thread;

		void queue_sync();
		volatile bool shut_down_signal;
		
		std::ofstream log_file;
		
		//counter keeps track of how many log entries failed to log due to queue being full
		volatile std::atomic_uint failed_log_attempts;

		const int idle_wait_time_ms = 500;
		const int queue_size = 256;
		
	public:
		FileLogger(const char* file_name) :
			Logger(),
			queue(queue_size),
			thread(NULL),
			shut_down_signal(false),
			failed_log_attempts(0)
		{
			log_file.open(file_name);
			if(log_file.is_open()) {
				thread = new boost::thread(&FileLogger::queue_sync, this);
			}
		}
		
		//thread safe method
		void log(const FileLogEntry& entry) {
			bool success = queue.push(entry);
			
			if(!success) failed_log_attempts++; //only touching volatile variable if log fails - should be rare
		}
		
		virtual ~FileLogger() {
			if(thread) {
				shut_down_signal = true;

				thread->join();
			
				delete thread;
			}
		}
};

//basic types
FileLogger& operator<<(FileLogger&, int);
FileLogger& operator<<(FileLogger&, long long);
FileLogger& operator<<(FileLogger&, double);
FileLogger& operator<<(FileLogger&, const char*);

//extandable generic type - extend from LoggerSerializable
FileLogger& operator<<(FileLogger&, LoggerSerializable * const);
FileLogger& operator<<(FileLogger&, const LoggerSerializable&);

#endif
