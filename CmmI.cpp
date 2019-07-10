#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

std::vector<std::string> warnings;
std::string warnStr;

//An object of this class represents a variable in the program. Meant to be stored in a umap called var_map
//contains a static vector of datatypes
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

	void updateValue(std::string value) {
		//Integer
		if (type == getTypes().at(0)) {
			try {
				iValue = stoi(value);
				valueString = std::to_string(iValue);
			}
			catch (std::out_of_range) {
				warnings.push_back(warnStr + " value out of range");
			}
			catch (std::invalid_argument) {
				warnings.push_back(warnStr + " invalid value");
			}
		}
		//decimal
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
				warnings.push_back(warnStr + " invalid value");
			}
		}
		//char
		else if (type == getTypes().at(2)) {
			valueString = "";
			cValue = value[0];
			valueString += cValue;
			if (value.size() > 1) {
				warnings.push_back(warnStr + " value for char more than one character");
			}
		}
		//boolean
		else if (type == getTypes().at(3)) {
			if (value == "true" || value == "1" || value == "t") {
				bValue = true;
				valueString = "true";
			}
			else if (value == "false" || value == "0" || value == "f") {
				bValue = false;
				valueString = "false";
			}
			else {
				warnings.push_back(warnStr + " invalid value for type bol, try true/false/1/0/t/f");
			}
		}
		//string
		else if (type == getTypes().at(4)) {
			sValue = value;
			valueString = sValue;
		}
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
				warnings.push_back(warnStr + " value for char more than one character");
			}
		}
		//BOOL
		else if (type == getTypes().at(3)) {
			if (value == "true" || value == "1" || value == "t") {
				bValue = true;
				valueString = "true";
			}
			else if (value == "false" || value == "0" || value == "f") {
				bValue = false;
				valueString = "false";
			}
			else {
				warnings.push_back(warnStr + " invalid declaration for type bol, try true/false/1/0/t/f");
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

//This function takes two vectors that make up an expressions terms and operators and returns a string of the result FOR INTEGER MATH
std::string integerMath(std::vector<std::string> numbers, std::vector<std::string> operatorOrder, std::unordered_map<std::string, Cmmvariable>& var_map) {

	int operatorCounter = 0;
	int result = stoi(numbers.at(0));

	for (int i = 1; i < numbers.size(); i++) {
		if (operatorOrder.at(operatorCounter) == "+") {
			result += stoi(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "-") {
			result -= stoi(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "/") {
			result /= stoi(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "*") {
			result *= stoi(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "%") {
			result %= stoi(numbers.at(i));
		}
		operatorCounter++;
	}

	return std::to_string(result);
}

//Function operates the same as the above function but for decimal math
std::string decimalMath(std::vector<std::string> numbers, std::vector<std::string> operatorOrder, std::unordered_map<std::string, Cmmvariable>& var_map) {

	int operatorCounter = 0;
	double result = stod(numbers.at(0));

	for (int i = 1; i < numbers.size(); i++) {
		if (operatorOrder.at(operatorCounter) == "+") {
			result += stod(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "-") {
			result -= stod(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "/") {
			result /= stod(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "*") {
			result *= stod(numbers.at(i));
		}
		else if (operatorOrder.at(operatorCounter) == "%") {
			warnings.push_back(warnStr + " modulus is not an acceptable operator on a dec type");
		}
		operatorCounter++;
	}

	return std::to_string(result);
}

std::string calculate(std::string expression, std::string type, std::unordered_map<std::string, Cmmvariable>& var_map) {
	//If you want to add an operator you need to also add it in the integerMath and decimalMath methods
	std::vector<std::string> operators = { "+", "-", "/", "*", "%" };
	std::vector<std::string> operatorOrder;
	std::vector<std::string> numbers;
	std::string tempNumber;

	//This will populate the numbers vector with each number defined as being seperated by an operator. This also populates the operatorOrder vector
	for (int i = 0; i < expression.size(); i++) {
		for (auto elem : operators) {
			if (expression[i] == elem[0]) {

				std::string tempOp;
				tempOp += expression[i];
				operatorOrder.push_back(tempOp);

				numbers.push_back(tempNumber);
				tempNumber = "";
				i++;
			}
		}
		tempNumber += expression[i];
	}

	numbers.push_back(tempNumber);

	//This will search through the numbers vector to find variable names. If one is found it will replace it with its value string
	for (auto& elem : numbers) {
		if (var_map.find(elem) != var_map.end()) {
			elem = var_map.at(elem).getValueString();
		}
	}

	//FOR INTEGER MATH
	if (type == Cmmvariable::getTypes().at(0)) {
		return integerMath(numbers, operatorOrder, var_map);
	}
	//FOR DECIMAL MATH
	else if (type == Cmmvariable::getTypes().at(1)) {
		return decimalMath(numbers, operatorOrder, var_map);
	}
}

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

//returns true if the word is considered a bad word, false if the word is ok
bool isBadWord(std::string word) {

	//If the word begins with a non-letter or a non-number
	if (!isalpha(word[0])) {
		if (isdigit(word[0])) {
			if (word.size() == 1) {
				return true;
			}
			return false;
		}
		return true;
	}

	if (word == "true" || word == "false" || word == "t" || word == "f") {
		return true;
	}

	return false;
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

		//if the value is another variable than set this variable to that variable
		if (var_map.find(varValue) != var_map.end()) {
			varValue = var_map.at(varValue).getValueString();
		}
		//if the type is a string set the value to the string literal for that line
		else if (type == "str") {
			varValue = thisLine.literal;
		}

		//The variable name was a bad word
		if (isBadWord(varName)) {
			warnings.push_back(warnStr + " variable name was invalid");
			return;
		}

		//The variable name was incorrect
		if (var_map.find(varName) != var_map.end()) {
			warnings.push_back(warnStr + " variable already exists");
			return;
		}

		var_map.emplace(varName, Cmmvariable(type, varValue));
	}
}

//This will update a variables value if it needs to be
void updateVar(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {

	std::string varName = "";
	std::string expression = "";

	bool doesVarExist = false;
	bool isExpressionVar = false;

	//If an equal sign was found set the left to varname and the right to expression
	size_t found = thisLine.lineStr.find("=");
	if (found != std::string::npos) {

		for (int i = 0; i < found; i++) {
			varName += thisLine.lineStr[i];
		}

		for (int i = found + 1; i < thisLine.lineStr.size(); i++) {
			expression += thisLine.lineStr[i];
		}

	}

	//make sure that varname exists in the map and then set the bool to true. if it doesnt create a warning
	if (var_map.find(varName) != var_map.end()) {
		doesVarExist = true;
	}
	else {
		warnings.push_back(warnStr + " " + varName + " is not an existing variable");
	}

	//if the expression is just a single variable
	if (var_map.find(expression) != var_map.end()) {
		isExpressionVar = true;
	}

	//FOR STRING
	if (doesVarExist) {
		if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(4)) {
			if (isExpressionVar) {
				var_map.at(varName).updateValue(var_map.at(expression).getValueString());
			}
			else {
				var_map.at(varName).updateValue(thisLine.literal);
			}
		}
		//FOR CHAR
		else if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(2)) {
			if (isExpressionVar) {
				var_map.at(varName).updateValue(var_map.at(expression).getValueString());
			}
			else {
				var_map.at(varName).updateValue(expression);
			}
		}
		//FOR INTEGER AND DECIMAL
		else if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(0) || var_map.at(varName).getType() == Cmmvariable::getTypes().at(1)) {
			if (isExpressionVar) {
				var_map.at(varName).updateValue(var_map.at(expression).getValueString());
			}
			else {
				var_map.at(varName).updateValue(calculate(expression, var_map.at(varName).getType(), var_map));
			}
		}
		//FOR BOOLEAN 
		else if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(3)) {
			if (isExpressionVar) {
				var_map.at(varName).updateValue(var_map.at(expression).getValueString());
			}
			else {
				var_map.at(varName).updateValue(expression);
			}
		}
	}


}

//Function for reading each individual line... everything is called from here
void readLine(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {
	warnStr = "WARNING[line " + std::to_string(thisLine.num) + "]:";

	bool wasVarCreated = false;

	//clears the whitespace and creates literals
	thisLine = removeWhitespace(thisLine, var_map);

	//If the line is blank do nothing
	if (thisLine.lineStr.size() == 0) {
		return;
	}

	//This will check if a variable is to be created on this line and do that
	std::string type = "";
	for (int i = 0; i < 3; i++) {
		type += thisLine.lineStr[i];
	}
	for (auto elem : Cmmvariable::getTypes()) {
		if (type == elem) {
			createVar(thisLine, type, var_map);
			wasVarCreated = true;
		}
	}

	//This will check for keywords
	//TODO

	//This will update a variable
	if (!wasVarCreated) {
		updateVar(thisLine, var_map);
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