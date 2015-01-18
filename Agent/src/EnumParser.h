/*
 * EnumParser.h
 *
 *  Created on: Jan 14, 2015
 *      Author: root
 */

#ifndef SRC_AGENT_ENUMPARSER_H_
#define SRC_AGENT_ENUMPARSER_H_

#include <map>

template<typename T>
class EnumParser
{
	std::map <std::string, T> enumMap;
	public:
	    EnumParser() {};
	    virtual ~EnumParser() {};

	    T parse(const std::string &value)
	    {
	        auto iValue = enumMap.find(value);
	        if (iValue  == enumMap.end())
	            throw std::runtime_error("Enum not found!");
	        return iValue->second;
	    }
};
template<>
EnumParser<HttpMethod>::EnumParser()
{
	enumMap["GET"] = GET;
	enumMap["POST"] = POST;
	enumMap["PUT"] = PUT;
	enumMap["HEAD"] = HEAD;
	enumMap["DELETE"] = DELETE;
	enumMap["TRACE"] = TRACE;
	enumMap["OPTIONS"] = OPTIONS;
	enumMap["CONNECT"] = CONNECT;
	enumMap["PATCH"] = PATCH;
}

#endif /* SRC_AGENT_ENUMPARSER_H_ */
