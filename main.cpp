
#include "FileLogger.hpp"


class Order : public LoggerSerializable {
	private:
		float price;
		int quantity;
		int a;
		
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


int main(int argc, char *argv[]) {

	auto path = "file.log";

	FileLogger logger(path);

	double pi = 3.14159265359;

	logger << pi;

	int i = 12345;

	logger << i;

	logger << "A sample message";

	const int long_text_length = 4000;
	char *buff = (char*)malloc(long_text_length);
	
	for(int i = 0; i < long_text_length - 1; i++) {
		buff[i] = "abcdef"[i % 6];
	}
	buff[long_text_length - 1] = '\n';
	
	logger << buff << "abcd" << 3.23;
	free(buff);

	Order o(23, 90);

	logger << (&o);

	return 0;
}

void Order::serialize_to_stream(std::ostream& str) const {
	str << "Order { price: " << this->price << ", quantity: " << this->quantity << " }";
}
