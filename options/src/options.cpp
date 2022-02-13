#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <regex>
#include <fmt/core.h>
#include <boost/algorithm/string.hpp>

#include "options.h"

using namespace std;

namespace {  // Although a module in the captlib also defines the following
    // functions, we duplicate the definitions since we want this module
    // depends no other modules.

string toString(int i)
{
	stringstream  os;
    os << i;
    return os.str();
}

double toDouble(const string & s)
{
	stringstream is(s);
    double d;
    is >> d;
    return d;
}

int toInt(const string & s)
{
	stringstream is(s);
    int i;
    is >> i;
    return i;
}

bool contains(const string& s, char c)
{
    for (int i=0; i<s.size();i++){
        if ( s[i] == c )
            return true;
    }
    return false;
}

}

namespace options {

using namespace boost;
using namespace std;

Token::Token(const std::string& value_, TokenType type_ )
{
    value = value_;
    type  = type_;
}

Options::Options(int argc, char* argv[])
{
    vector<string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    vector<string> merged = mergeString(args);
    parse(merged);
}

Options::Options(const string& optionsFilename)
{
    parseOptionFile(optionsFilename);
}

vector<string> Options::mergeString( const vector<string> & args )
{
    vector<string> result;
    for (int i=0; i<args.size(); ){
        if ( contains(args[i], '"') ) {
            string s = args[i++];
            // 寻找右边的双引号
            while ( i < args.size() ) {
                s+= string(" ") + args[ i ];
                if ( contains(args[i], '"') ){
                    i++;
                    break;
                }
                i++;
            }
            result.push_back( s );
        }else{
            result.push_back( args[i++] );
        }
    }
    return result;
}

void Options::parse(const vector<string>& args)
{
    bool anyOptionProcessed = false;

    for (size_t i = 0; i < args.size(); i++) {
        const std::string& currentArg = args[i];
        if ( currentArg[0] == '-' && currentArg.size() > 1) {
            // see what's next, differentiate whether it's short or long:
            if (currentArg[1] == '-'){
                if ( currentArg.size() > 2 ){
                    // long option
                    tokens.push_back( Token(currentArg.substr(2), LongOption) );
                    options[ currentArg.substr(2) ] = tokens.size() - 1;
                    anyOptionProcessed = true;
                } else {
                    // it's the -- option alone
                    throw runtime_error("a long option must have name");
                }
            } else {
                if ( regex_match(currentArg, regex("-[0-9.]*")) ){
                    tokens.push_back( Token(currentArg, anyOptionProcessed ? OptionArgument: GlobalArgument) );
                } else {
                    for( size_t j = 1; j < currentArg.size(); j++ ) {
                        string optionName(1, currentArg[j]);
                        tokens.push_back( Token( optionName, ShortOption) );
                        options[ optionName ] = tokens.size() - 1;
                    }
                    anyOptionProcessed = true;
                }
            }
        } else
        if ( currentArg[0] == '@' && currentArg.size() > 1 ) {
            parseOptionFile(currentArg.substr(1));
        } else {
            tokens.push_back( Token(currentArg, anyOptionProcessed ? OptionArgument: GlobalArgument) );
        }
    }
}

void Options::parseOptionFile(const string& file)
{
    ifstream ifile(file.c_str());
    if (!ifile)
        throw runtime_error( string("can't find file ") + file );

    vector<string> args;
    string line;
    while (getline(ifile, line)){
        if ( line.length() > 0 && line[0] == '#' ){
            continue;
        }
        istringstream ss( line );
        string arg;
        while ( ss >> arg )
            args.push_back(arg);
    }

    args = mergeString(args);
    parse(args);
}


string Options::getString(int pos)
{
    if ( pos >= tokens.size() )
        throw runtime_error( string("the speficifed postion ") + toString(pos) + " is larger than the number of tokens");
    if ( tokens[pos].type != GlobalArgument ){
        throw runtime_error( string("the token at ") + toString(pos) + " is not a global parameter" );
    }
    return tokens[pos].value;
}

int Options::getInt(int pos)
{
    string content = getString(pos);
    if ( content.empty() )
        throw runtime_error(
            fmt::format("there is no double as a global option at the specified position {}",
                        pos) );
    return toInt( content );
}

double Options::getDouble(int pos)
{
    string content = getString(pos);
    if ( content.empty() )
        throw runtime_error(
            fmt::format("there is no double as a global option at the specified position {}",
                        pos) );
    return toDouble( getString(pos) );
}

bool Options::presents(const string & optionName)
{
    return options.count(optionName) > 0;
}

string Options::getString(const string & optionName)
{
    if ( !presents(optionName) )
        return string();
    int tokenIndex = options.at(optionName) + 1;
    if (tokens[tokenIndex].type != OptionArgument )
        throw runtime_error( string("no string parameter for option ") + optionName );
    return tokens[tokenIndex].value;
}

int Options::getInt(const string & optionName, int defaultValue)
{
    string value;
    value = getString(optionName);
    if ( value.empty() )
        return defaultValue;
    return toInt(value);
}

double Options::getDouble(const string & optionName, double defaultValue)
{
    string value;
    value = getString(optionName);
    if ( value.empty() )
        return defaultValue;
    return toDouble(value);
}

vector<string> Options::getVectorString(const string & optionName)
{
    vector<string> result;
    if ( ! presents(optionName) ){
        throw std::runtime_error(
                    fmt::format("no program option: {}", optionName) );
    }

    int tokenIndex = options.at(optionName) + 1;
    while ( tokens[tokenIndex].type == OptionArgument ){
        result.push_back( tokens[tokenIndex].value );
        tokenIndex++;
    }
    if ( result.empty() )
        throw runtime_error( string("can't find a vector of string for ") + optionName );
    return result;
}

vector<int> Options::getVectorInt(const string & optionName)
{
    vector<string> values;
    values = getVectorString(optionName);
    vector<int> result;
    for (string value: values){
        result.push_back( toInt(value) );
    }
    return result;
}

vector<double> Options::getVectorDouble(const string & optionName)
{
    vector<string> values;
    values = getVectorString(optionName);
    vector<double> result;
    for (string value: values){
        result.push_back( toDouble(value) );
    }
    return result;
}


// read suboptions 
string Options::getString(const string & mainOptionName, const string& subOptionName)
{
   vector<string> args = getVectorString(mainOptionName);
   for (string arg: args ) {
        int pos = arg.find( subOptionName + "=" );
        if ( pos == 0 ) {
            return arg.substr( subOptionName.length() + 1 );
        }
    }
    return string();
}

int Options::getInt(const string & mainOptionName, const string& subOptionName, int defaultValue)
{
    string value = getString(mainOptionName, subOptionName);
    if ( value.empty() )
        return defaultValue;
    return  toInt(value);
}

double Options::getDouble(const string & mainOptionName, const string& subOptionName, double defaultValue)
{
    string value = getString(mainOptionName, subOptionName);
    if ( value.empty() )
        return defaultValue;
    return toDouble( value );
}

vector<string> Options::getVectorString(const string & mainOptionName, const string& subOptionName)
{
    vector<string> result;
    string value = getString(mainOptionName, subOptionName);
    if ( value.empty() )
        return result;
    trim_if(value, is_any_of("\"") );
    trim(value);
    split(result, value, is_any_of(" "), boost::token_compress_on);
    return result;
}

vector<Token> Options::getTokens()
{
    return tokens;
}

Options * OptionsInstance::m_instance = NULL;

}
