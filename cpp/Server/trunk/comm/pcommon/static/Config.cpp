
#include "Config.h"
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <iostream>

const std::string Config::GROUP_DEFAULT("default");

Config::Config(std::string filename)
{
	pathname = filename;
}
int Config::loadfile(std::string filename)
{
	pathname = filename;

	int line;
	return load(line);
}

void Config::trimString(std::string &str)
{
	int i = 0;
	for(; i < (int)str.length() && isspace(str[i]); i++);
	
	str = str.substr(i);
	
	for(i = (int)str.length() - 1; i >= 0 && isspace(str[i]); i--);

	str.erase(i + 1);
	
}

bool Config::load()
{
	int errLine;
	return load(errLine);
}

bool Config::load(int &errLine)
{
	std::ifstream in(pathname.c_str());
	
	std::string value;

	paragraph.clear();
	errLine = 0;

	if(false == in.is_open())
	{
		//mLastError = xml::errors::XML_ERROR_OPEN_FILE;
		return false;
	}

	std::string currentGroup = GROUP_DEFAULT;
	Pairs pairs;

	//while (false == _stream.eof()) {
	//	std::getline(_stream, read);
	//	mLine ++;
	//	mCol = 0;

	while(std::getline(in, value)){
		errLine++;
		trimString(value);
		if(value[0] == '#')
			continue;
		
		if(value.length() == 0)
			continue;


		std::string::size_type idx = value.find("=");

		if(idx == std::string::npos){
			if(value[0] == '[' && value[value.length() - 1] == ']'){
				value.erase(value.length() - 1);
				value.erase(0, 1);
				
				paragraph[currentGroup] = pairs;

				currentGroup = value;
				pairs.clear();
			} else
				return false;
		} else{
			std::string key = value.substr(0, idx);
			trimString(key);
			
			std::string val = value.substr(idx + 1);
			trimString(val);
			pairs[key] = val;
		}
	}
	paragraph[currentGroup] = pairs;
	return true;
}	

int Config::getInt(std::string group, std::string name, int defValue /*= 0*/)
{
	int val = defValue;
	std::string value = getValue(group, name);
	if (!value.empty())
	{
		std::istringstream in(value);
		in >> val;
	}
	return val;
}

double Config::getDouble(std::string group, std::string name, double defValue /*= 0.0*/)
{
	double val = defValue;
	std::string value = getValue(group, name);
	if (!value.empty())
	{
		std::istringstream in(value);
		in >> val;
	}
	return val;
}

std::string Config::getValue(std::string para, std::string name)
{
	std::string group = para;
	trimString(group);
	if(0 == (int)group.length())
		group = GROUP_DEFAULT;

   	std::map<std::string, Pairs, std::greater<std::string> >::iterator pos = paragraph.find(group);
	if(pos != paragraph.end()){
		Pairs pairs = pos->second;
		Pairs::iterator p = pairs.find(name);
		if(p != pairs.end())
			return p->second;
	}

	std::string str;
	return str;

}

std::string Config::getString(std::string group, std::string name, std::string defValue /*= ""*/)
{
	std::string value = getValue(group, name);
	
	return value.empty() ? defValue: value;
}

void Config::set(std::string para, std::string name, std::string value)
{
	std::string group = para;
	trimString(group);
	if(group.length() == 0)
		group = GROUP_DEFAULT;

	Pairs &pairs = paragraph[group];
	pairs[name] = value;
}

void Config::set(std::string paragraph, std::string name, int value)
{
	std::ostringstream os;
	os << value;
	set(paragraph, name, os.str());
}

void Config::dump(std::ostream &os)
{
//dump DEFAULT_GROUP group first
	Pairs pairs = paragraph[GROUP_DEFAULT];

	Pairs::iterator p = pairs.begin();
	for(; p != pairs.end(); ++p){
		os << p->first << '=' << p->second << std::endl;
	}

	std::map<std::string, Pairs, std::greater<std::string> >::iterator pos =  paragraph.begin();
       
	for(; pos != paragraph.end(); ++pos){
		pairs = pos->second;
		if(pos->first == GROUP_DEFAULT)
			continue;

		os << '[' << pos->first << ']' << std::endl;
		p = pairs.begin();
		for(; p != pairs.end(); ++p){
			os << p->first << '=' << p->second << std::endl;
		}
	}
}
void Config::save()
{
	std::ofstream out(pathname.c_str());
	dump(out);
}
			


