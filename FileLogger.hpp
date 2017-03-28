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
 * It's possible to log to the logger from multiple threads.
 */

class FileLogger : public Logger {

	typedef enum types {
		INTEGER,
		FLOATING,
		SHORT_CHAR,
		SERIALIZABLE_INPLACE, //optiomization to save memory allocation for small objects
		SERIALIZABLE
	} types_enum;

	public:
		class LogEntry {
			private:
				types_enum type;
		
				union {
					char short_char[255];
					long long integer;
					double floating;
					LoggerSerializable* serializable;
				} data;
		
			public:

				LogEntry() : type(INTEGER) {
					this->data.integer = 0;		
				}

				LogEntry(int a) : type(INTEGER) {
					this->data.integer = a;		
				}

				LogEntry(long a) : type(INTEGER) {
					this->data.integer = a;		
				}
		
				LogEntry(long long a) : type(INTEGER) {
					this->data.integer = a;		
				}

				LogEntry(double a) : type(FLOATING) {
					this->data.floating = a;		
				}

				LogEntry(const std::string &a) : type(SHORT_CHAR) {
					int length = a.length();
		
					if(length < sizeof(this->data.short_char)) {
						//short strings are kept in the queue
			
						strncpy(this->data.short_char, a.c_str(), sizeof(this->data.short_char));				
					} else {
						//long strings are copied to the heap
			
						this->type = SERIALIZABLE;
						this->data.serializable = new StringSerializable(a.c_str(), length);
					}					
				}

				LogEntry(LoggerSerializable * const s) : type(SERIALIZABLE) {
					if(s->get_size() <= sizeof(this->data)) {				
						s->clone(&this->data);
						type = SERIALIZABLE_INPLACE;
					} else {
						this->data.serializable = s->clone(); //allocates copy on the heap
					}
				}
		
				bool requires_allocation() const {
					return type == SERIALIZABLE;
				}
		
				void log_to_stream(std::ostream&) const;
	
		};

	private:

		const int idle_wait_time_ms = 1000;
		const int queue_size = 256;
	
		boost::thread *thread;

		std::atomic_uint shut_down_signal;
		std::ofstream log_file;
		
		//counter keeps track of how many log entries failed to log due to queue being full
		std::atomic_uint failed_log_attempts;
		
		//stats
		int log_entries_flat;
		int log_entries_with_allocation;
		
		boost::lockfree::queue<LogEntry> queue;

		void queue_sync();
		
	public:
		FileLogger(const char* file_name) :
			Logger(),
			thread(NULL),
			shut_down_signal(0),
			log_file(),
			failed_log_attempts(0),
			log_entries_flat(0),
			log_entries_with_allocation(0),
			queue(queue_size)
		{
			log_file.open(file_name);
			if(log_file.is_open()) {
				thread = new boost::thread(&FileLogger::queue_sync, this);
			}
		}

		//thread safe method
		void log(const LogEntry& entry) {
			bool success = queue.push(entry);
			
			if(!success) failed_log_attempts++; //only touching std::atomic variable if log fails - should be rare
		}

		virtual ~FileLogger() {

			if(thread) {
				shut_down_signal.store(true); //signal the tread to stop processing
				
				thread->join(); //wait for the thread to properly empty the queue and close the file
			
				delete thread;
			}
		}
};

//basic types
FileLogger& operator<<(FileLogger&, int);
FileLogger& operator<<(FileLogger&, long long);
FileLogger& operator<<(FileLogger&, double);
FileLogger& operator<<(FileLogger&, const std::string&);

FileLogger& operator<<(FileLogger&, char *); //purposely not implemneted to fail compilation and avoid implicit cast to const char *

FileLogger& operator<<(FileLogger&, const char *);

//extendable generic type - extend from LoggerSerializable
FileLogger& operator<<(FileLogger&, LoggerSerializable * const);
FileLogger& operator<<(FileLogger&, const LoggerSerializable&);

#endif
