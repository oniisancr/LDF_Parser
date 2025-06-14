#ifndef ldf_parser_helper_hpp
#define ldf_parser_helper_hpp

#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace utils {
	// 辅助函数：读取一对大括号内的内容（支持嵌套）
	inline std::string readBlockWithBraces(std::istream &in)
	{
		std::string result;
		int braceCount = 1;
		char ch;
		while (in.get(ch))
		{
			result += ch;
			if (ch == '{')
				++braceCount;
			else if (ch == '}')
			{
				--braceCount;
				if (braceCount == 0)
					break;
			}
		}
		return result;
	}
	// A special version of trim function that removes any leading and trailling white spaces
	// and also removes all occurrences of new line and tab characters
	inline std::string& trim(std::string& str)
	{
		 // 去除前导空白
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        // 去除尾部空白
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), str.end());
		return str;
	}
	// A custom getline function that trims the word before returning
	inline std::string getline(std::istream& lineStream, char delimiter) {
		std::string word;
		getline(lineStream, word, delimiter);
		trim(word);
		return word;
	}
	// Gives the condition name in a string
	inline std::string lastTokenOf(std::string& str) {
		// Read the first token from an input string stream constructed with the string in reverse
		// And read until the first white space
		std::string reversedToken, conditionName;
		std::istringstream({ str.rbegin(), str.rend() }) >> reversedToken;
		// Read until either delimeters
		for (size_t i = 0; i < reversedToken.size(); i++) {
			if (reversedToken[i] == ';' || reversedToken[i] == '}') {
				break;
			}
			conditionName += reversedToken[i];
		}
		// Reverse back the token
		return { conditionName.rbegin(), conditionName.rend() };
	}
	// A custom stoi function that detects the input number base
	inline int stoi(std::string number) {
		if (number[0] == '0' && number.size() > 1) {
			if (number[1] == 'x') {
				// Input is HEX
				return std::stoi(number, 0, 16);
			}
			// Input is OCTAL
			return std::stoi(number, 0, 8);
		}
		// Input is DEC
		return std::stoi(number, 0, 10);
	}
}

#endif /* ldf_parser_helper_hpp */
