#include "FileLogger.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

using namespace std;

void FileLogger::queue_sync() {

	this->log_file << "Started\n";

	while(!this->shut_down_signal || !this->queue.empty()) {
				
		//flush the whole queue
		FileLogEntry entry;		
		while(this->queue.pop(entry)) entry.log_to_stream(this->log_file);
		
		//sleep before checking again - note: being woke up by the OS exactly when we have entries would be better.
		//However, this is a good tradeoff for using lockless/sleepless queue
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	}
	
	this->log_file << "Clean exit\n";
	
	this->log_file.close();
}

FileLogger& operator<<(FileLogger& logger, int a) {
	logger.log(FileLogEntry(a));

	return logger;
}

FileLogger& operator<<(FileLogger& logger, double a) {
	logger.log(FileLogEntry(a));

	return logger;
}

FileLogger& operator<<(FileLogger& logger, char* a) {
	logger.log(FileLogEntry(a));

	return logger;
}

FileLogger& operator<<(FileLogger& logger, const char* a) {
	logger.log(FileLogEntry(a));

	return logger;
}

