#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>
#include <map>

namespace options {

using std::string;
using std::vector;
using std::map;

enum TokenType{ GlobalArgument, ShortOption, LongOption, OptionArgument };
struct Token {
    Token(const std::string& value, TokenType type );

    TokenType type;
    std::string value;
};

class Options {
public:
    Options(int argc, char* argv[]);
    Options(const string& optionsFilename);

    std::string getString(int startPosition);
    int getInt(int startPosition);
    double getDouble(int startPosition);

    bool   presents(const string & optionName);

    // If the specified option doesn't exist, return an empty string.
    string getString(const string & optionName);

    int    getInt(const string & optionName, int defaultValue);
    double getDouble(const string & optionName, double defaullValue);
    vector<string>  getVectorString(const string & optionName);
    vector<int>     getVectorInt(const string & optionName);
    vector<double>  getVectorDouble(const string & optionName);

    // If the specified option doesn't exist, return an empty string.
    string getString(const string & mainOptionName, const string& subOptionName);
    int getInt(const string & mainOptionName, const string& subOptionName, int defaultValue);
    double getDouble(const string & mainOptionName, const string& subOptionName, double defaultValue);
    vector<string> getVectorString(const string & mainOptionName, const string& subOptionName);
    vector<int> getVectorInt(const string & mainOptionName, const string& subOptionName);
    vector<double> getVectorDouble(const string & mainOptionName, const string& subOptionName);

    vector<Token> getTokens();

private:
    vector<string> mergeString( const vector<string> & args );
    void parse(const vector<string>& args);
    void parseOptionFile(const string& file);

private:
    vector<Token> tokens;
    map<string, int> options;
};

class OptionsInstance {
public:
	OptionsInstance(int argc, char* argv[] ) {
        if (m_instance)
            throw std::runtime_error("OptionsInstance can be constructed only once");
        m_instance = new Options(argc, argv);
	}
    OptionsInstance(const string& optionsFilename ) {
        if (m_instance)
            throw std::runtime_error("OptionsInstance can be constructed only once");
        m_instance = new Options(optionsFilename);
    }
	~OptionsInstance(){
        delete m_instance;
	}
	static Options & get(){
        return * m_instance;
	}
private:
	static Options * m_instance;
};

}

#endif
