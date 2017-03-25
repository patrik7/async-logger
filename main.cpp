
#include "FileLogger.hpp"

int main(int argc, char *argv[]) {

	auto path = "file.log";

	FileLogger logger(path);

	double pi = 3.14159265359;

	logger << pi;

	int i = 12345;

	logger << i;

	logger << "A sample message";

	return 0;
}
