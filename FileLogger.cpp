#include "FileLogger.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

using namespace std;

void FileLogger::queue_sync() {

	this->log_file << "FileLogger started, queue size: " << this->queue_size << " (" << (sizeof(LogEntry)*this->queue_size) << "b)\n";

	while(!this->shut_down_signal.load() || !this->queue.empty()) {
		
		bool logged_something = false;
		
		//flush the whole queue
		LogEntry entry;		
		while(this->queue.pop(entry)) {
			logged_something = true;
			
			entry.log_to_stream(this->log_file);
			
			//update stats
			if(entry.requires_allocation()) {
				this->log_entries_with_allocation++;
			} else {
				this->log_entries_flat++;
			}
		}
		
		int failed_attempts = this->failed_log_attempts.exchange(0);		
		if(failed_attempts) {
			this->log_file << failed_attempts << " log entries failed to log\n";
		}
		
		if(logged_something) {
			this->log_file.flush(); //make sure we sync data to disk often enogh - we don't want to loose last logs on segfault		
		} else {
		
			//sleep before checking again - note: being woke up by the OS exactly when we have entries would be better.
			//However, this is a good tradeoff for using lockless/sleepless queue
			boost::this_thread::sleep(boost::posix_time::milliseconds(this->idle_wait_time_ms));
		}
	}
	
	this->log_file << "FileLogger shut down\n";
	this->log_file << "-log entries that required memory allocation/freeing: " << this->log_entries_with_allocation << "\n";
	this->log_file << "-log entries without memory allocation/freeing: " << this->log_entries_flat << "\n";
	
	this->log_file.close();
	
}

void FileLogger::LogEntry::log_to_stream(std::ostream& log) const {

	switch(this->type) {
		case INTEGER:      log << this->data.integer << "\n"; break;
		case FLOATING:     log << this->data.floating << "\n"; break;
		case SHORT_CHAR:   log << this->data.short_char << "\n"; break;
		case SERIALIZABLE_INPLACE: {
			reinterpret_cast<const LoggerSerializable*>(&this->data)->serialize_to_stream(log);
			log << "\n";		
			break;
		}
		case SERIALIZABLE: {
			this->data.serializable->serialize_to_stream(log);
			log << "\n";
			delete this->data.serializable;
			break;
		}
	}
		
}

FileLogger& operator<<(FileLogger& logger, long long a) {
	logger.log(FileLogger::LogEntry(a));

	return logger;
}

FileLogger& operator<<(FileLogger& logger, int a) {
	return logger << (long long)a;
}


FileLogger& operator<<(FileLogger& logger, double a) {
	logger.log(FileLogger::LogEntry(a));

	return logger;
}

FileLogger& operator<<(FileLogger& logger, const std::string& a) {
	logger.log(FileLogger::LogEntry(a));

	return logger;
}

FileLogger& operator<<(FileLogger& logger, const char * a) {
	logger << std::string(a);

	return logger;
}

FileLogger& operator<<(FileLogger& logger, LoggerSerializable * const s) {
	logger.log(s);

	return logger;
}

FileLogger& operator<<(FileLogger& logger, const LoggerSerializable& s) {
	logger.log( (LoggerSerializable*)&s);

	return logger;
}


