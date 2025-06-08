#ifndef frame_hpp
#define frame_hpp

#include <map>
#include <string>
#include <vector>
#include "signal.hpp"

class Frame {

public:

	// Getters
	int getId() const { return id; }
	int getDlc() const { return messageSize; }
	const std::string getPublisher() const { return publisher; }
	std::string getName() const { return name; }
    int getLength() const { return messageSize; }
	std::vector<Signal*> getConnectedSignals() const { return connectedSignals; }
	// Setters
	void setId(const int& id) { this->id = id; }
	void setName(const std::string& name) { this->name = name; }
	void setMessageSize(const int& messageSize) { this->messageSize = messageSize; }
	void setPublisher(const std::string& publisher) { this->publisher = publisher; }
	void addSignalInfo(Signal* sig) { this->connectedSignals.push_back(sig); }
	// Overloads
	friend std::ostream& operator<<(std::ostream& os, const Frame& frm);
	
	// 获取所有信号的所有订阅者（去重）
    std::vector<std::string> getSubscribers() const;
private:

	int id{};
	int messageSize{};
	std::string name{};
	std::string publisher;
	std::vector<Signal*> connectedSignals;

};

#endif /* frame_hpp */
