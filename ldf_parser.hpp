#ifndef ldf_parser_hpp
#define ldf_parser_hpp

#include <map>
#include <string>
#include <optional>
#include "ldf_parser_dependencies/frame.hpp"
#include "ldf_parser_dependencies/signal.hpp"
#include "ldf_parser_dependencies/signal_encoding_type.hpp"

struct ScheduleEntry {
    std::string name;    // 帧名或命令名
    double delay;        // 延时时间
    std::string type;    // "Frame", "Command", "Unknown"
};

struct ScheduleTable {
    std::string name;
    std::vector<ScheduleEntry> entries;
};

class LdfParser {

public:

	// Construct using either a File or a Stream of a LDF-File
	// A bool is used to indicate whether parsing succeeds or not
	bool parse(const std::string& filePath);
	// Encode
	int encode(
		int const frameId,
		std::vector<std::pair<std::string, double> >& signalsToEncode,
		unsigned char encodedPayload[MAX_FRAME_LEN]
	);
	// Decode
	std::map<std::string, std::tuple<double, std::string, LinSigEncodingValueType> >
		decode(
			int const frameId,
			int const frmSize,
			unsigned char payLoad[MAX_FRAME_LEN]
		);
	// Print LDF info
	friend std::ostream& operator<<(std::ostream& os, const LdfParser& ldfFile);

	// 获取调度表
	const std::vector<ScheduleTable>& getScheduleTables() const { return scheduleTables; }
	// 获取主节点名称
    const std::string& getMasterNode() const { return masterNode; }
    // 获取从节点名称列表
    const std::vector<std::string>& getSlaveNodes() const { return slaveNodes; }
	// 获取所有信号
    const std::map<std::string, Signal>& getSignals() const { return signalsLibrary; }
    // 获取所有帧
    const std::map<int, Frame>& getFrames() const { return framesLibrary; }
    // 获取所有信号编码类型
    const std::map<std::string, SignalEncodingType>& getSignalEncodingTypes() const { return sigEncodingTypeLibrary; }

private:

	std::optional<std::string> LinProtocolVersion;
	typedef std::map<int, Frame>::iterator framesLib_itr;
	typedef std::map<std::string, Signal>::iterator signalsLib_itr;
	typedef std::map<std::string, SignalEncodingType>::iterator sigEncodingTypeLib_itr;
	bool isEmptyLibrary = true;
	bool isEmptyFramesLibrary = true;
	bool isEmptySignalsLibrary = true;
	bool isEmptySigEncodingTypeLibrary = true;
	// A hash table that stores all info of frames. <Frame id, Frame object>
	std::map<int, Frame> framesLibrary{};
	// A hash table that stores all info of signals. <Signal name, Signal object>
	std::map<std::string, Signal> signalsLibrary{};
	// A hash table that stores all info of signal encoding types. <encoding type name, Signal Encoding type object>
	std::map<std::string, SignalEncodingType> sigEncodingTypeLibrary{};
	// Function used to parse LDF file
	void resetParsedContent();
	void consistencyCheck();
	void loadAndParseFromFile(std::istream& in);

	std::string masterNode;
    std::vector<std::string> slaveNodes;
	std::vector<ScheduleTable> scheduleTables;
};

#endif /* ldf_parser_hpp */
