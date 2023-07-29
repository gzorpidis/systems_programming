#include "../include/parsing.hpp"
#include <sstream>

namespace parsing {

    void print(std::vector<std::string> vec) {
        for(int i = 0; i < vec.size();i++) {
            std::cout << i << " " << vec[i] << std::endl;
        }
    }

    void removeDoubleQuotes(std::string& str) {
        str.erase(str.begin());
        str.erase(str.end()-1);
    }

    bool isDoubleQuoted(std::string& str) {
        return str.size() >= 2 && str.front() == '"' && str.back() == '"';
    }

    std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
        std::vector<std::string> tokens;
        std::size_t startPos = 0;
        std::size_t endPos = 0;

        while ((endPos = input.find(delimiter, startPos)) != std::string::npos) {
            std::string token = input.substr(startPos, endPos - startPos);
            tokens.push_back(token);
            tokens.push_back(delimiter);
            // tokens.push_back(token + delimiter);  // Include the delimiter in the token
            startPos = endPos + delimiter.length();
        }

        // Add the last token after the last delimiter
        std::string lastToken = input.substr(startPos);
        tokens.push_back(lastToken);

        return tokens;
    }

    // Get a vector of strings:
    //      If inside double quotes, do not split on the append delimiter
    //      If outside of double quotes, split properly
    std::vector<std::string> splitOnAppend(std::vector<std::string>& vec) {
        std::vector<std::string> outputStrings;

        for (std::string inputString : vec) {
            if (parsing::isDoubleQuoted(inputString)) {
                outputStrings.push_back(inputString);
            } else {
                std::vector<std::string> split_on_append = splitString(inputString, ">>");
                outputStrings.insert( outputStrings.end(), split_on_append.begin(), split_on_append.end() );
            }
        }

        return outputStrings;

    }

    // split i-th string of "tokens", depending on char: delimiter
    // return true if delimiter was found
    // false, if not found
    bool splitOnSymbol(std::vector<std::string> &tokens, int i, char delimiter) {
        // Single character string cannot be split up
	    if (tokens[i].size() < 2) { return false; }
	    int pos = pos = tokens[i].find(delimiter);
        
        // if the delimiter was found inside the string
	    if (pos != std::string::npos) {
	    	if (pos == 0) {
	    		//starts with symbol
	    		tokens.insert(tokens.begin() + i + 1, tokens[i].substr(1, tokens[i].size() - 1));
	    		tokens[i] = tokens[i].substr(0, 1);
	    	}
	    	else {
	    		//symbol in between
	    		tokens.insert(tokens.begin() + i + 1, std::string{delimiter});
	    		std::string leftover = tokens[i].substr(pos + 1, tokens[i].size() - pos - 1);
	    		if (!leftover.empty()) {
	    			tokens.insert(tokens.begin() + i + 2, leftover);
	    		}
	    		tokens[i] = tokens[i].substr(0, pos);
	    	}
	    	return true;
	    }
	    else
	    {
	    	return false;
	    }
    }   

    // Final function to create the tokens of the command
    std::vector<std::string> tokenize(const std::vector<std::string>& input) {
        std::vector<std::string> output;
        for (const auto& str : input) {
            if (str.empty()) continue;
            if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
                // String is quoted, add as is
                output.push_back(str);
            } else {
                int pos = 0;
	            int space;
	            std::string s = str;
                while ((space = s.find(' ', pos)) != std::string::npos) {
	    	        std::string word = s.substr(pos, space - pos);
	    	        if (!word.empty()) {
            			output.push_back(word);		            
	    	        }
	            	pos = space + 1;
	            }

	            std::string lastWord = s.substr(pos, s.size() - pos);
	            if (!lastWord.empty() || lastWord == "" || lastWord == " ") {
	            	output.push_back(lastWord);
	            }
            }
        }
    
        for (int i = 0; i < output.size(); ++i) {
            if (isDoubleQuoted(output[i])) continue;
            if (output[i] == ">>") continue;

	        for (char c : {'&', '<', '>', '|', ';'}) {
	            if (splitOnSymbol(output, i, c)) {
	            	--i;
	            	break;
	            }
	        }
	    }

        return output;
    }

    // gets the input and outputs strings depending on if they are inside ""
    // everything inside double quotes is added to the same string (token)
    // else it's split as it should
    std::vector<std::string> parseString(std::string input) {
        std::vector<std::string> result;
        bool inQuote = false;
        std::string current;
    
        for (char c : input) {
            // if reach "\"" and not in quotes, its the opening one
            if (c == '"' && !inQuote) {
                inQuote = true;
                current += "\"";
            } else if (c == '"' && inQuote) {
            // else if we reached a "\"" that was already opened, it closes here
                current += "\"";
                inQuote = false;
                result.push_back(current);
                current = "";
            } else if (inQuote) {
            // else if the character is in inside quotes, as is, except
            // if it a "\\" character, that means there exists an escaped double quote
                if (c == '\\') {
                    current += "\"";
                    if (input.size() > 1) {
                        input.erase(0, 1);
                    }
                } else {
                    current += c;
                }
            } else if (c == ' ') {
                if (current.size() > 0) {
                    result.push_back(current);
                    current = "";
                }
            } else {
                current += c;
            }
        }
    
        if (current.size() > 0 && current != "" && current[current.size()-1] != '\0' && !isblank(current[current.size()-1])) {
            result.push_back(current);
        }
    
        return result;
    }
}


//You shouldn't need to touch these or call them directly
bool splitOnSymbol(std::vector<std::string>& words, int i, char c);
std::vector<std::string> myshell_tokenize(const std::string& s);

// You'll need to fork/exec for each one of these!
// Initially, assume the user tries to only execute 1 command.
struct Command{
  std::string exec; //the name of the executable
  //remember argv[0] should be the name of the program (same as exec)
  //Also, argv should end with a nullptr!
  std::vector<const char*> argv; 
  int fdStdin, fdStdout;
  bool background;
  int _argc;
};


//useful for debugging (implemented for you)
std::ostream& operator<<(std::ostream& outs, const Command& c);

//Read this function.  You'll need to fill in a few parts to implement
//I/O redirection and (possibly) backgrounded commands.
//I think all the places you need to fill in contain an assert(false), so you'll
//discover them when you try to use more functionality
std::vector<Command> getCommands(const std::vector<std::string>& tokens);