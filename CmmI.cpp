#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

std::vector<std::string> warnings;
std::string warnStr;

class Cmmvariable {
public:
	//With a value
	Cmmvariable(std::string t, std::string value) {
		type = t;
		setValue(value);
	}
	//If declared with no value
	Cmmvariable(std::string t) {
		type = t;
		setValueStringDefault();
	}

	
	std::string getValueString() {
		return valueString;
	}

	std::string getType() {
		return type;
	}

	static std::vector<std::string> getTypes() {
		std::vector<std::string> types;
		types.push_back("int");
		types.push_back("dec");
		types.push_back("chr");
		types.push_back("bol");
		types.push_back("str");

		return types;
	}


private:
	std::string type;
	int iValue = 0;
	double dValue = 0;
	char cValue;
	bool bValue = true;
	std::string sValue = "";
	std::string valueString = "";

	//This sets the precision on the value string for doubles
	void setPrecision() {
		int precision = 0;

		for (int i = 0; i < valueString.size(); i++) {
			if (valueString[i] != '0') {
				precision = i;
			}
		}

		valueString.erase(valueString.begin() + precision + 1, valueString.end());
		if (valueString[valueString.size() - 1] == '.') {
			valueString += '0';
		}
	}

	//This handles setting the value string when no value was declared
	void setValueStringDefault() {
		if (type == getTypes().at(0)) {
			valueString = std::to_string(iValue);
		}
		else if (type == getTypes().at(1)) {
			valueString = std::to_string(dValue);
			setPrecision();
		}
		else if (type == getTypes().at(3)) {
			valueString = "true";
		}
	}

	//This will set var values only if a value was passed
	void setValue(std::string value) {
		//INTEGER
		if (type == getTypes().at(0)) {
			try {
				iValue = stoi(value);
				valueString = std::to_string(iValue);
			}
			catch (std::out_of_range) {
				warnings.push_back(warnStr + " value out of range");
			}
			catch (std::invalid_argument) {
				warnings.push_back(warnStr + " invalid declaration for type int");
			}

		}
		//DECIMAL
		else if (type == getTypes().at(1)) {
			try {
				dValue = stod(value);
				valueString = std::to_string(dValue);
				setPrecision();
			}
			catch (std::out_of_range) {
				warnings.push_back(warnStr + " value out of range");
			}
			catch (std::invalid_argument) {
				warnings.push_back(warnStr + " invalid declaration for type dec");
			}
		}
		//CHAR
		else if (type == getTypes().at(2)) {
			cValue = value[0];
			valueString += cValue;
			if (value.size() > 1) {
				//Create warning
			}
		}
		//BOOL
		else if (type == getTypes().at(3)) {
			if (value == "true" || value == "1") {
				bValue = true;
				valueString = "true";
			}
			else if (value == "false" || value == "0") {
				bValue = false;
				valueString = "false";
			}
			else {
				warnings.push_back(warnStr + " invalid declaration for type bol. Try true, false, 1, or 0.");
			}
		}
		//STRING
		else if (type == getTypes().at(4)) {
			sValue = value;
			valueString = sValue;
		}

	}
};

//Struct for each individual line in the cmm file
struct line {
	std::string lineStr;
	std::string literal = "";
	int num;
};

//This will create a string literal if a line has one. This is to exempt it from the clear whitespace rule
line createStrLiteral(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {
	bool inLiteral = false;
	bool gettingVar = false;
	std::string varName = "";

	//This is some mumbo jumbo to search through the line find literals
	for (int i = 0; i < thisLine.lineStr.size(); i++) {

		if (thisLine.lineStr[i] == '"' && thisLine.lineStr[i - 1] != '\\') {
			if (inLiteral) {
				inLiteral = false;
			}
			else {
				inLiteral = true;
			}
		}
		if (inLiteral && thisLine.lineStr[i] != '\"') {
			thisLine.literal += thisLine.lineStr[i];
		}
		//Support for \n character
		if (inLiteral && thisLine.lineStr[i] == 'n' && thisLine.lineStr[i - 1] == '\\') {
			int size = thisLine.literal.size();
			thisLine.literal.erase(size - 1);
			thisLine.literal.erase(size - 2);
			thisLine.literal += "\n";
		}

		//Ending line statement is a '.'
		if (!inLiteral && thisLine.lineStr[i] == '.') {
			thisLine.literal += "\n";
		}

		//Vars appended to strings must be surrounded by ':'
		if (!inLiteral && thisLine.lineStr[i] == ':') {
			if (!gettingVar) {
				gettingVar = true;

			}
			else {
				gettingVar = false;
				varName.erase(varName.begin());
				
				try {
					thisLine.literal += var_map.at(varName).getValueString();
				}
				catch (std::out_of_range) {
					warnings.push_back(warnStr + " " + varName + " is not an existing variable");
				}

				varName = "";
			}
		}

		//If a var is found within ':' this will store each char in varName.
		if (gettingVar) {
			varName += thisLine.lineStr[i];
		}

	}

	return thisLine;
}

//this function removes whitespace from a line and then returns the line
line removeWhitespace(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {

	//IMPORTANT: If this is not called, all string literals will have NO whitespace. It is important to use line. strLiteral when reffering to literals
	thisLine = createStrLiteral(thisLine, var_map);

	for (int i = 0; i < thisLine.lineStr.size(); i++) {
		if (thisLine.lineStr[i] == ' ') {
			thisLine.lineStr.erase(thisLine.lineStr.begin() + i);
		}
	}

	return thisLine;
}

//This function will create a new variable
void createVar(line thisLine, std::string type, std::unordered_map<std::string, Cmmvariable>& var_map) {

	std::string dVarName = "";
	std::string varName = "";
	bool defaultDec = true;
	int equalLocation = 0;

	//this finds the var name
	for (int i = 3; i < thisLine.lineStr.size(); i++) {
		if (thisLine.lineStr[i] == '=') {
			equalLocation = i;
			defaultDec = false;
			for (int j = 3; j < i; j++) {
				varName += thisLine.lineStr[j];
			}
		}
		dVarName += thisLine.lineStr[i];
	}

	if (defaultDec) {
		var_map.emplace(dVarName, Cmmvariable(type));
	}
	else {

		std::string varValue = "";
		for (int i = equalLocation + 1; i < thisLine.lineStr.size(); i++) {
			varValue += thisLine.lineStr[i];
		}

		if (type == "str") {
			varValue = thisLine.literal;
		}

		var_map.emplace(varName, Cmmvariable(type, varValue));
	}
}


//Function for reading each individual line... everything is called from here
void readLine(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {
	warnStr = "WARNING[line " + std::to_string(thisLine.num) + "]:";
	thisLine = removeWhitespace(thisLine, var_map);

	//This will check if a variable is to be created on this line and do that
	std::string type = "";
	for (int i = 0; i < 3; i++) {
		type += thisLine.lineStr[i];
	}
	for (auto elem : Cmmvariable::getTypes()) {
		if (type == elem) {
			createVar(thisLine, type, var_map);
		}
	}
}

//this function opens a cmm file to be read
void openFile(std::string fileName, std::unordered_map<std::string, Cmmvariable>& var_map) {
	std::ifstream ifs(fileName);

	line thisLine;
	thisLine.num = 1;
	while (getline(ifs, thisLine.lineStr)) {
		readLine(thisLine, var_map);
		thisLine.num++;
	}
}

int main() {

	std::unordered_map<std::string, Cmmvariable> var_map;


	//for (auto elem : var_map) {
	//	std::cout << "Name: " << elem.first << "\tValue: " << elem.second.getValueString() << std::endl;
	//}


	std::string fileName = "";
	std::cout << "Welcome to C-- v1" << std::endl << "Enter file name to run" << std::endl;
	std::cin >> fileName;
	std::cout << "\n\n";
	openFile(fileName, var_map);

		std::cout << "\n\n";
	for (auto elem : warnings) {
		std::cout << elem << std::endl;
	}

	for (auto elem : var_map) {
		std::cout << "Name: " << elem.first << "\tType: " << elem.second.getType() << "\tValue: " << elem.second.getValueString() << std::endl;
	}

}