
#ifndef __CONFIG_20060220_H_
#define __CONFIG_20060220_H_
#include <map>
#include <string>
#include <iostream>
#include <sstream>

class Config{
 private:
	std::string              pathname;
	typedef std::map<std::string, std::string, std::greater<std::string> > Pairs;
	Pairs                    pairs;
	std::map<std::string, Pairs, std::greater<std::string> > paragraph;
	
 public:
	static const std::string GROUP_DEFAULT;
	Config(){};
	Config(std::string filename);
	int loadfile(std::string filename);
	
	static void        trimString(std::string &str);
	void               set(std::string, std::string, std::string);
	void               set(std::string, std::string, int);
	std::string        getValue(std::string paragraph, std::string name);
	bool               load();
	bool               load(int &errLine);

	int				getInt(std::string group, std::string name, int defValue = 0);
	double			getDouble(std::string group, std::string name, double defValue = 0.0);
	std::string		getString(std::string group, std::string name, std::string defValue = "");
	
	void               dump(std::ostream &);
	void               save();
	
	template<typename ResType, typename DefaultResType>
    void get(ResType& Data, const std::string &group, const std::string &name, DefaultResType DefaultData) 
    {
    	std::string value = getValue(group, name);
        std::stringstream ss;
    	if (!value.empty())         
            ss << value;
        else
            ss << DefaultData;
        ss >> Data;
    }
    

    void get(std::string& Data, const std::string &group, const std::string &name, std::string DefaultData) 
    {
    	Data = getValue(group, name);   	
    	if (Data.empty()) 
    	    Data = DefaultData;
    }
    
    template<typename ResType>
    void get(ResType& Data, const std::string &group, const std::string &name) 
    {
    	std::string value = getValue(group, name);
        std::stringstream ss;
    	if (!value.empty()) 
    	{        
            ss << value;
            ss >> Data;
        }
    }
    
    void get(std::string& Data, const std::string &group, const std::string &name) 
    {
    	Data = getValue(group, name);
    }
};
	
#endif /*__CONFIG_20060220_H_ */


