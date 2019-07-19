#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

std::vector<std::string> warnings;
//warnings.push_back(warnStr + " whatever you need the warning to say");
std::string warnStr;

//Struct for each individual line in the cmm file
struct line {
	std::string lineStr;
	std::string literal = "";
	std::vector<int> varNameLocations;
	std::vector<std::string> vars;
	int num;
	int scope = -1;
};

//Struct that stores information about a loop
struct Loop {
	bool doingLoop = false;
	int begin = 0;
	int end = 0;
	int lineBegin = 0;
	int lineEnd = 0;
	int loopScope = 0;
	int counter = 0;
	std::string iterator = "i";
	std::string expression;
	std::string loopType = "";
};

//An object of this class represents a variable in the program. Meant to be stored in a umap called var_map
//contains a static vector of datatypes
class Cmmvariable {
public:
	//With a value
	Cmmvariable(std::string t, std::string value, int scope) {
		type = t;
		setValue(value);
		this->scope = scope;
	}
	//If declared with no value
	Cmmvariable(std::string t, int scope) {
		type = t;
		setValueStringDefault();
		this->scope = scope;
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

	int getScope() {
		return scope;
	}


private:
	int scope;
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

//This class is meant to be static and used as a container for all of the keyword functions
class Keyword {
public:

	//This function expects a line and var_map. The literal stores variable names, not their value
	static void display(line thisLine, std::unordered_map<std::string, Cmmvariable> var_map) {

		std::string tempVarName = "";
		bool gettingVarName = false;
		int varStartLocation, varStrSize;

		//Searches through the var location and var name vectors in the line struct and inserts them into the literal
		for (int i = 0; i < thisLine.varNameLocations.size(); i++ ) {
			thisLine.literal.insert(thisLine.varNameLocations.at(i), var_map.at(thisLine.vars.at(i)).getValueString());
		}

		std::cout << thisLine.literal;
	}

	//This function reads the next line and stores the lines value into that variable
	static void read(std::string lineStr, std::unordered_map<std::string, Cmmvariable>& var_map) {

		std::string varName;
		for (int i = 4; i < lineStr.size(); i++) {
			varName += lineStr[i];
		}

		if (var_map.find(varName) != var_map.end()) {
			std::string temp;
			std::cin >> temp;

			var_map.at(varName).updateValue(temp);
		}
		else {
			warnings.push_back(warnStr + " attempted to read to a variable that does not exist");
		}
	}
	
	//This function returns true if an if statement evaluated to true; else it returns false
	static bool conditionalIF(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {
		//The if code is 1
		return evaluate(thisLine, var_map, 1);
	}

	//This function takes a line, code and var_map and compares two values on either side of an expression.
	static bool evaluate(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map, int code) {
		//the code is representing what keyword needs to use evaluate
		// 1-> if	2-> elif	3-> loop while

		switch (code) {
			case 1: 
				code = 2;
				break;
			case 2: 
				code = 4;
				break;
			case 3: 
				code = 9;
				break;
		}
		
		std::string leftValue, rightValue;
		bool doingLeftValue = true;
		bool notEqual = false;
		bool greater = false;
		bool greaterE = false;
		bool lesser = false;
		bool lesserE = false;

		//This loop will set the left and the right values
		for (int i = code; i < thisLine.lineStr.size(); i++) {
			//For equality and non-equality
			if (thisLine.lineStr[i] == '=') {
				if (thisLine.lineStr[i - 1] == '!') {
					notEqual = true;
					leftValue.pop_back();
				}
				doingLeftValue = false;
				i++;
			}
			//for greater than
			else if (thisLine.lineStr[i] == '>') {
				if (thisLine.lineStr[i + 1] == '=') {
					greaterE = true;
					greater = false;
					i++;
				}
				doingLeftValue = false;
				greater = true;
				i++;
			}
			//for less than
			else if (thisLine.lineStr[i] == '<') {
				if (thisLine.lineStr[i + 1] == '=') {
					lesserE = true;
					lesser = false;
					i++;
				}
				doingLeftValue = false;
				lesser = true;
				i++;
			}
			if (doingLeftValue) {
				leftValue += thisLine.lineStr[i];
			}
			else {
				rightValue += thisLine.lineStr[i];
			}
		}

		//this is for a single bol value in the conditional
		if (doingLeftValue) {
			if (var_map.find(leftValue) != var_map.end()) {
				leftValue = var_map.at(leftValue).getValueString();
				if (leftValue == "true") {
					return true;
				}
				else if (leftValue == "false") {
					return false;
				}
			}
		}

		//If either of the values are actually variables
		if (var_map.find(leftValue) != var_map.end()) {
			if (var_map.at(leftValue).getType() == "dec") {
				leftValue = wholeDecChecker(leftValue, var_map);
			}

			else {
				leftValue = var_map.at(leftValue).getValueString();
			}
		}

		if (var_map.find(rightValue) != var_map.end()) {
			if (var_map.at(rightValue).getType() == "dec") {
				rightValue = wholeDecChecker(rightValue, var_map);
			}
			else {
				rightValue = var_map.at(rightValue).getValueString();
			}
		}

		//If the value was found to be a string literal then do the regular handling for it 
		if (leftValue[0] == '\"') {
			leftValue = thisLine.literal;
		}
		if (rightValue[0] == '\"') {
			rightValue = thisLine.literal;
		}
		
		//The following commented out lines are just a debugger for this function
		//std::cout << "left: " << leftValue << " right: " << rightValue << std::endl;
		//std::cout << "g: " << greater << " ge: " << greaterE << " l: " << lesser << " le: " << lesserE << " ne: " << notEqual << std::endl;
		//This is the main stuff right here... returning true if the stuffs TRUE
		if (greater) {
			if (greaterE) {
				if (leftValue >= rightValue) {
					return true;
				}
				return false;
			}
			if (leftValue > rightValue) {
				return true;
			}
			return false;
		}
		if (lesser) {
			if (lesserE) {
				if (leftValue <= rightValue) {
					return true;
				}
				return false;
			}
			if (leftValue < rightValue) {
				return true;
			}
			return false;
		}
		if (leftValue == rightValue) {
			if (notEqual) {
				return false;
			}
			return true;
		}
		else {
			if (notEqual) {
				return true;
			}
			return false;
		}


	}

	//This will check to see if a line is going to be a loop, used in checkForKeywords
	static bool checkLoop(std::string line) {
		std::string thisShouldBeLoop;
		for (int i = 0; i < 4; i++) {
			thisShouldBeLoop += line[i];
		}

		if (thisShouldBeLoop == "loop") {
			return true;
		}
		else {
			return false;
		}
	}

	//This is the function for doing loops
	static Loop startLoop(std::string line, std::unordered_map<std::string, Cmmvariable>& var_map) {
		//loop from 0-12: i will do a for loop

		Loop loop;
		std::string forLoop = "from";
		std::string whileLoop = "while";
		std::string loopType;

		for (int i = 4; i < line.size(); i++) {
			loopType += line[i];
			if (loopType == forLoop) {
				break;
			}
			else if (loopType == whileLoop) {
				break;
			}
		}

		loop.loopType = loopType;
		loop.doingLoop = true;
	
		if (loopType == forLoop) {
			std::string rangeFloor, rangeCeil;
			int maxNumberOfLoops = 0;
			int loopIteratorLocation = 0;
			std::string loopIteratorName = "";
			bool isFloor = true;
			//Get the floor and ceiling for the loop range
			for (int i = 8; i < line.size(); i++) {
				if (line[i] == ':') {
					loopIteratorLocation = i + 1;
					break;
				}
				if (line[i] == '-') {
					isFloor = false;
				}
				if (isFloor && line[i] != '-') {
					rangeFloor += line[i];
				}
				else if (!isFloor && line[i] != '-') {
					rangeCeil += line[i];
				}
			}

			//If either of the range numbers are an int value then set the range numbers to that variables value
			if (var_map.find(rangeFloor) != var_map.end()) {
				if (var_map.at(rangeFloor).getType() == "int") {
					rangeFloor = var_map.at(rangeFloor).getValueString();
				}
			}
			if (var_map.find(rangeCeil) != var_map.end()) {
				if (var_map.at(rangeCeil).getType() == "int") {
					rangeCeil = var_map.at(rangeCeil).getValueString();
				}
			}

			for (int i = loopIteratorLocation; i < line.size(); i++) {
				loopIteratorName += line[i];
			}
			
			loop.begin = stoi(rangeFloor);
			loop.end = stoi(rangeCeil);
			loop.iterator = loopIteratorName;
			return loop;

		}
		//For while loops
		else if (loopType == whileLoop) {
			return loop;
		}
	}

	//This function will return a string vector of all the implemented keywords
	static std::vector<std::string> getKeywords() {
		return createKeywords();
	}
private:

	//This function will store all of the keywords
	static std::vector<std::string> createKeywords() {
		std::vector<std::string> keywords;
		keywords.push_back("display");
		keywords.push_back("read");
		keywords.push_back("if");
		keywords.push_back("loop");
		return keywords;
	}

	//If the dec is a whole number this will return it completed whole-ified to compare its value
	static std::string wholeDecChecker(std::string leftValue, std::unordered_map<std::string, Cmmvariable>& var_map) {
			bool isWhole = true;
			bool afterDot = false;
			for (int i = 0; i < leftValue.size(); i++) {
				if (afterDot) {
					if (leftValue[i] != '0') {
						isWhole = false;
						break;
					}
				}
				if (leftValue[i] == '.') {
					afterDot = true;
				}
			}
			if (isWhole) {
				leftValue = std::to_string(stoi(leftValue));
			}
				return leftValue;
	}
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

//This function takes an expression sans the '=' and returns a string that is equal to the result
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

	//If the literal has already been set then just return the line
	if (thisLine.literal != "") {
		return thisLine;
	}

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
				
				if (var_map.find(varName) != var_map.end()) {
					thisLine.varNameLocations.push_back(thisLine.literal.size());
					thisLine.vars.push_back(varName);
				}
				else {
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
			thisLine.lineStr.erase(i, 1);
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

//This function takes direct inputs and creates a variable
void directVarConstructor(std::string name, std::string type, std::string value, std::unordered_map<std::string, Cmmvariable>& var_map, int scope) {

	//if the value is another variable than set this variable to that variable
	if (var_map.find(value) != var_map.end()) {
		value = var_map.at(value).getValueString();
	}

	//The variable name was a bad word
	if (isBadWord(name)) {
		warnings.push_back(warnStr + " variable name was invalid");
		return;
	}

	//The variable name already exists
	if (var_map.find(name) != var_map.end()) {
		warnings.push_back(warnStr + " variable already exists");
		return;
	}

	var_map.emplace(name, Cmmvariable(type, value, scope));
}

//Calling this function will kill all the variables of whatever scope has been passed.
void scopeVarDestroyer(std::unordered_map<std::string, Cmmvariable>& var_map, int scope) {
	for (auto elem : var_map) {
		if (elem.second.getScope() == scope) {
			var_map.erase(elem.first);
		}
	}
}

//This function will create a new variable
void createVar(line thisLine, std::string type, std::unordered_map<std::string, Cmmvariable>& var_map, int scope) {

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
		var_map.emplace(dVarName, Cmmvariable(type, scope));
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
			//Searches through the var location and var name vectors in the line struct and inserts them into the literal
			for (int i = 0; i < thisLine.varNameLocations.size(); i++) {
				thisLine.literal.insert(thisLine.varNameLocations.at(i), var_map.at(thisLine.vars.at(i)).getValueString());
			}
			varValue = thisLine.literal;
		}

		//The variable name was a bad word
		if (isBadWord(varName)) {
			warnings.push_back(warnStr + " variable name was invalid");
			return;
		}

		//The variable name already exists
		if (var_map.find(varName) != var_map.end()) {
			warnings.push_back(warnStr + " variable already exists");
			return;
		}

		var_map.emplace(varName, Cmmvariable(type, varValue, scope));
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
	
	if (doesVarExist) {
		//FOR STRING
		if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(4)) {

			if (isExpressionVar) {
				var_map.at(varName).updateValue(var_map.at(expression).getValueString());
			}
			else {
				//Searches through the var location and var name vectors in the line struct and inserts them into the literal
				for (int i = 0; i < thisLine.varNameLocations.size(); i++) {
					thisLine.literal.insert(thisLine.varNameLocations.at(i), var_map.at(thisLine.vars.at(i)).getValueString());
				}
				
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

//This function checks for keywords and calls the required functions for them to work
int checkForKeyWords(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {

	//CODE: 1->display 2->read 3->if
	std::vector<std::string> keywords = Keyword::getKeywords();

	for (auto elem : keywords) {
		size_t found = thisLine.lineStr.find(elem);
		if (found != std::string::npos) {
			//DISPLAY
			if (elem == keywords.at(0)) {
				Keyword::display(thisLine, var_map);
				return 1;
			}
			//READ
			else if (elem == keywords.at(1)) {
				Keyword::read(thisLine.lineStr, var_map);
				return 1;
			}
			//IF 
			else if (elem == keywords.at(2)) {
				if (Keyword::conditionalIF(thisLine, var_map)) {
					return 2;
				}
				else {
					return 1;
				}
			}
			//LOOP
			else if (elem == keywords.at(3)) {
				if (Keyword::checkLoop(thisLine.lineStr)) {
					return 3;
				}
				else {
					return 1;
				}
			}
		}
	}

	return false;
}

//This only allows the scope for a line to be set once
void setLineScope(line& thisLine, int newScope) {
	if (thisLine.scope == -1) {
		thisLine.scope = newScope;
		thisLine.lineStr.erase(0, thisLine.scope);
	}
}

//This is the control function for the language. Everything happens from here.
void readLine(std::vector<line> lines, std::unordered_map<std::string, Cmmvariable>& var_map) {
	//This controls whether the program is currently running
	bool running = true;
	bool inLoop = false;
	//Loop loop;
	std::vector<Loop> loops;
	//int loopCounter = 0;
	int nestedLoopCounter = -1;
	//Line num is the access value for the lines vector 
	int lineNum = 0;
	//This int is what level the line has access to. If a conditional evaluates true, the program would gain a level of access
	int access = 0;

	do {

		//This will determine the scope of the line and set the lines vector scope 
		int newScope = 0;
		while (lines.at(lineNum).lineStr[newScope] == '\t') {
			newScope++;
		}
		setLineScope(lines.at(lineNum), newScope);
		if (access > lines.at(lineNum).scope) {
			access = newScope;
		}

		//This chunk turns a loop off if it needs to be turned off or it sets the do-while back to the start of the loop
		if (nestedLoopCounter != -1) {
			if (loops.at(nestedLoopCounter).doingLoop) {
				access = lines.at(lineNum).scope;
				if (loops.at(nestedLoopCounter).loopScope > lines.at(lineNum).scope) {
					//When a from loop reaches its ending line
					if (loops.at(nestedLoopCounter).loopType == "from") {
						if (loops.at(nestedLoopCounter).counter == loops.at(nestedLoopCounter).end) {
							loops.at(nestedLoopCounter).doingLoop = false;
							access = lines.at(lineNum).scope;
							scopeVarDestroyer(var_map, loops.at(nestedLoopCounter).loopScope);
							if (nestedLoopCounter > 0) {
								loops.pop_back();
								nestedLoopCounter--;
							}
						}
						else {
							loops.at(nestedLoopCounter).counter++;
							var_map.at(loops.at(nestedLoopCounter).iterator).updateValue(std::to_string(loops.at(nestedLoopCounter).counter));
							lineNum = loops.at(nestedLoopCounter).lineBegin;
							continue;
						}
					}
					//When a while loop reaches its ending line
					else if (loops.at(nestedLoopCounter).loopType == "while") {
						loops.at(nestedLoopCounter).doingLoop = Keyword::evaluate(lines.at(loops.at(nestedLoopCounter).lineBegin - 1), var_map, 3);
						if (loops.at(nestedLoopCounter).doingLoop) {
							lineNum = loops.at(nestedLoopCounter).lineBegin;
						}
						continue;
					}
				}
			}
		}

		//If the line is blank OR if the program does not have access to something at this scope then skip the line
		if (lines.at(lineNum).lineStr.size() == 0 || access < lines.at(lineNum).scope) {
			//Increase the lineNum and if its equal to the size of the array then break the loop
			lineNum++;
			if (lineNum == lines.size()) {
				return;
			}
			continue;
		}


		//String for warnings
		warnStr = "WARNING[line " + std::to_string(lineNum) + "]:";

		//This is a bool to check if a var was created on this line, used later
		bool wasVarCreated = false;

		//A code for the return values of certain keywords
		int keywordCode = 0; //0->no keyword found     1->keyword found     2->conditional code		3->loop code

		//clears the whitespace and creates literals
		lines.at(lineNum) = removeWhitespace(lines.at(lineNum), var_map);	

		//This will check if a var is being created on this line, and then create the variable storing it in var_map
		std::string type = "";
		for (int i = 0; i < 3; i++) {
			type += lines.at(lineNum).lineStr[i];
		}
		for (auto elem : Cmmvariable::getTypes()) {
			if (type == elem) {
				createVar(lines.at(lineNum), type, var_map, lines.at(lineNum).scope);
				wasVarCreated = true;
			}
		}

		//This function will check for a keyword and execute its stuff
		keywordCode = checkForKeyWords(lines.at(lineNum), var_map);

		//If the code was 2 up the access
		if (keywordCode >= 2) {
			access = access + 1;
			if (keywordCode == 3) {
				Loop newLoop = Keyword::startLoop(lines.at(lineNum).lineStr, var_map);
				loops.push_back(newLoop);
				nestedLoopCounter++;
				loops.at(nestedLoopCounter).loopScope = lines.at(lineNum).scope + 1;
				loops.at(nestedLoopCounter).lineBegin = lineNum + 1;

				if (loops.at(nestedLoopCounter).loopType == "from") {
					loops.at(nestedLoopCounter).counter = loops.at(nestedLoopCounter).begin;
					directVarConstructor(loops.at(nestedLoopCounter).iterator, "int", std::to_string(loops.at(nestedLoopCounter).begin), var_map, lines.at(lineNum).scope + 1);
				}

				if (loops.at(nestedLoopCounter).loopType == "while") {
					//This is horrible, but 3 is the code for while loops
					loops.at(nestedLoopCounter).doingLoop = Keyword::evaluate(lines.at(lineNum), var_map, 3);
				}
			}
		}

		//This will update a variable
		if (!wasVarCreated && keywordCode == 0 && lines.at(lineNum).lineStr.size() != 0) {
			updateVar(lines.at(lineNum), var_map);
		}

		//This increments the line vector 
		lineNum++;

		//If the program has read the final line then terminate the readline loop
		if (lineNum == lines.size()) {
			running = false;
		}


	} while (running);
}

//this function opens a cmm file to be read
void openFile(std::string fileName, std::unordered_map<std::string, Cmmvariable>& var_map) {
	std::ifstream ifs(fileName);
	std::vector<line> lines;

	//This dictates the scope of something. 0 would be global scope and 1 would be inside a conditional or loop block
	int scope = 0;

	line newLine;
	while (getline(ifs, newLine.lineStr)) {
		lines.push_back(newLine);
	}

	readLine(lines, var_map);

}

int main() {

	std::unordered_map<std::string, Cmmvariable> var_map;

	std::string fileName = "";
	std::cout << "Welcome to C-- v2" << std::endl << "Enter file name to run" << std::endl;
	std::cin >> fileName;
	std::cout << "\n\n";
	openFile(fileName, var_map);

	//This will display all created warnings to the terminal at the end of the program
	std::cout << "\n\n";
	for (auto elem : warnings) {
		std::cout << elem << std::endl;
	}

	//This section is for debugging. If uncommented it will display all the variables, name/type/value to the terminal
	//for (auto elem : var_map) {
	//	std::cout << "Name: " << elem.first << "\tType: " << elem.second.getType() << "\tValue: " << elem.second.getValueString() << "\tScope: " << elem.second.getScope() << std::endl;
	//}

}