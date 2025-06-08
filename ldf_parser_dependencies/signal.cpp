#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include "signal.hpp"
#include "ldf_parser_helper.hpp"

std::ostream& operator<<(std::ostream& os, const Signal& sig) {
	std::cout << "<Signal> " << sig.name << ": " << std::endl;
	std::cout << "\t" << std::left << std::setw(20) << "size: " << sig.signalSize << std::endl;
	std::cout << "\t" << std::left << std::setw(20) << "start bit: " << sig.startBit << std::endl;
	std::cout << "\t" << std::left << std::setw(20) << "initial value: " << sig.initValue << std::endl;
	if (sig.encodingType != NULL) {
		std::cout << "\t"
			<< std::left
			<< std::setw(20)
			<< "Encode type: "
			<< sig.encodingType->getName()
			<< std::endl;
	}
	else {
		std::cout << "\t"
			<< std::left
			<< std::setw(20)
			<< "No Encode type"
			<< std::endl;
	}
	std::cout << "\t"
		<< std::left
		<< std::setw(20)
		<< "publisher: "
		<< sig.publisher
		<< std::endl;
	if (sig.subscribers.size() != 0) {
		std::cout << "\t"
			<< std::left
			<< std::setw(20)
			<< std::to_string(sig.subscribers.size()) + " subscriber(s): ";
		for (size_t i = 0; i < sig.subscribers.size(); i++) {
			std::cout << sig.subscribers[i] << ' ';
		}
		std::cout << std::endl;
	}
	else { std::cout << "\tNo subscribers" << std::endl; }
	return os;
}

std::istream& operator>>(std::istream& in, Signal& sig) {
    // Parse signal info
    std::string sigName = utils::getline(in, ':');
    int sigSize = utils::stoi(utils::getline(in, ','));
    // Read initial value and check for byte array signals
    std::string rawString = "";
    int64_t initValue = 0;
    std::vector<int> initBytes; // 用于存储全部初始字节
	// 跳过空白
    while (in && std::isspace(in.peek())) in.get();
    if (in.peek() == '{') {
		in.get(); // 跳过 '{'
		std::string initByteBlock = utils::readBlockWithBraces(in); // 读取大括号内的内容
		if (in.peek() == ',')
		{
			in.get(); // 跳过 ','
		}
		if (initByteBlock.empty()) {
			throw std::invalid_argument("Parse Failed. Initial byte block is empty.");
		}
		// 去除前后空格
		utils::trim(initByteBlock); // 去除前后空格
		if(initByteBlock.back() == '}')
		{
			initByteBlock.pop_back(); // 去除末尾的 '}'
		}
        std::stringstream ss(initByteBlock);
        std::string byteStr;
        while (std::getline(ss, byteStr, ',')) {
            utils::trim(byteStr);
            if (!byteStr.empty())
                initBytes.push_back(std::stoi(byteStr));
        }
        // 按照小端方式将initBytes合成为initValue
        for (size_t i = 0; i < initBytes.size(); ++i) {
            initValue |= (initBytes[i] & 0xFF) << (8 * i);
        }
    } else {
		rawString = utils::getline(in, ',');
        initValue = utils::stoi(rawString);
    }
    std::string publisher = utils::getline(in, ',');
    std::string subscriber = utils::getline(in, ',');
    std::vector<std::string> subscribers;
    while (subscriber != "") {
        // Remove semi colon if there exists one
        char lastCharOfSubscriber = subscriber.back();
        if ((lastCharOfSubscriber == ';') && (!subscriber.empty())) {
            // Remove trailling colon and white spaces
            subscriber.pop_back();
            utils::trim(subscriber);
        }
        subscribers.push_back(subscriber);
        subscriber = utils::getline(in, ',');
    }
    // Store signal info
    sig.setName(sigName);
    sig.setSignalSize(sigSize);
    sig.setInitValue(initValue);
    sig.setPublisher(publisher);
    sig.setSubscribers(subscribers);
    return in;
}

std::tuple<double, std::string, LinSigEncodingValueType>
Signal::decodeSignal(unsigned char rawPayload[MAX_FRAME_LEN]) {
	// Change endianness
	int64_t payload = 0;
	for (int i = MAX_FRAME_LEN; i > 0; i--) {
		payload <<= 8;
		payload |= (uint64_t)rawPayload[i - 1];
	}
	// Decode raw value
	int64_t decodedRawValue = 0;
	uint8_t* data = (uint8_t*)&payload;
	unsigned int currentBit = startBit;
	// Access the corresponding byte and make sure we are reading a bit that is 1
	for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
		if (data[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT))) {
			decodedRawValue |= (1ULL << bitpos);
		}
		currentBit++;
	}
	// Apply linear transformation
	LinSigEncodingValueType sigValueType = encodingType->getValueTypeFromRawValue(decodedRawValue);
	std::string unit = encodingType->getUnitFromRawValue(decodedRawValue);
	double factor = encodingType->getFactorFromRawValue(decodedRawValue);
	double offset = encodingType->getOffsetFromRawValue(decodedRawValue);
	double decodedPhysicalValue = (double)decodedRawValue * factor + offset;
	return std::make_tuple(decodedPhysicalValue, unit, sigValueType);
}

uint64_t Signal::encodeSignal(double valueToEncode, bool isInitialValue) {
	// Reverse linear conversion rule
	// to convert the signals physical value into the signal's raw value
	uint64_t encodedValue = 0; encodedValue = ~encodedValue;
	uint64_t rawValue;
	if (isInitialValue) {
		rawValue = (uint64_t)valueToEncode;
	}
	else {
		double offset = encodingType->getOffsetFromPhysicalValue(valueToEncode);
		double factor = encodingType->getFactorFromPhysicalValue(valueToEncode);
		rawValue = (uint64_t)((valueToEncode - offset) / factor);
	}
	// Encode
	unsigned int currentBit = 0;
	uint8_t* rawPayload = (uint8_t*)&rawValue;
	for (unsigned short bitpos = 0; bitpos < signalSize; bitpos++) {
		// Access the corresponding byte and make sure we are reading a dominant bit 0
		if (!(rawPayload[currentBit / CHAR_BIT] & (1 << (currentBit % CHAR_BIT)))) {
			// Add dominant bit
			encodedValue &= ~(1ULL << (bitpos + startBit));
		}
		currentBit++;
	}
	// Change endianness
	unsigned char encodedPayload[MAX_FRAME_LEN];
	for (short i = 8 - 1; i >= 0; i--) {
		encodedPayload[i] = encodedValue % 256; // get the last byte
		encodedValue /= 256; // get the remainder
	}
	encodedValue = 0;
	for (int i = MAX_FRAME_LEN; i > 0; i--) {
		encodedValue <<= 8;
		encodedValue |= (uint64_t)encodedPayload[i - 1];
	}
	return encodedValue;
}
