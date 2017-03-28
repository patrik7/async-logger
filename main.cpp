
#include "FileLogger.hpp"

/*
 * A test class to show cast how to log a custom object
 */
class Order : public LoggerSerializable {
	private:
		float price;
		int quantity;
		
	public:
		Order(float price, int quantity): price(price), quantity(quantity) {}
	
		virtual void serialize_to_stream(std::ostream& str) const;
		virtual Order* clone(void * placement) const {
			return placement ? new (placement) Order(*this) : new Order(*this);
		}
		
		virtual size_t get_size() const {
			return sizeof(*this);
		}


};

/*
 * Another test class, this time having large memory requirements
 */
class AHugeClass : public LoggerSerializable {
	private:
		char buff[4096]; //uninicialzed, and never used
		
	public:
		AHugeClass() {}
	
		virtual void serialize_to_stream(std::ostream& str) const;
		
		virtual AHugeClass* clone(void * placement) const {
			return placement ? new (placement) AHugeClass(*this) : new AHugeClass(*this);
		}
		
		virtual size_t get_size() const {
			return sizeof(*this);
		}


};

int main(int argc, char *argv[]) {

	auto path = "file.log";

	FileLogger logger(path);

	double pi = 3.14159265359;

	logger << pi;

	int i = 12345;

	logger << i;

	logger << "A sample message";

	{
		//log a long string
		
		const int long_text_length = 4000;
		char *buff = (char*)malloc(long_text_length);
	
		for(int i = 0; i < long_text_length - 1; i++) {
			buff[i] = "abcdef"[i % 6];
		}
		buff[long_text_length - 1] = '\0';
	
		logger << const_cast<const char *>(buff) << "abcd" << 3.23;
		free(buff);
	}


	{
		//log custom objects
	
		Order o(23, 90);

		logger << o;

		logger << AHugeClass();
	}
	
	{
		//nasty - log a non null terminated string
		
		/* note: no longer compiles

		char* buff = (char*)malloc(4);
		strncpy(buff, "abcd", 4);
		
		logger << buff;
		
		free(buff);
		
		*/
	}


	return 0;
}

void Order::serialize_to_stream(std::ostream& str) const {
	str << "Order { price: " << this->price << ", quantity: " << this->quantity << " }";
}

void AHugeClass::serialize_to_stream(std::ostream& str) const {
	str << "AHugeClass { size: " << sizeof(AHugeClass) << " }";
}
