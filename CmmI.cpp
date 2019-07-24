#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <bits/stdc++.h>

std::vector<std::string> warnings;
//warnings.push_back(warnStr + " whatever you need the warning to say");
std::string warnStr;

//Struct for each individual line in the cmm file
struct lineIndexLocations {
	int matchingVarsIndex;
	std::string index;
};

struct line {
	std::string lineStr;
	std::string literal = "";
	std::vector<int> varNameLocations;
	std::vector<std::string> vars;
	std::vector<lineIndexLocations> varIndexs;
	int num;
	int scope = -1;
};

//Struct that stores information about a loop
struct Loop {
	bool doingLoop = false;
	bool complex = false;
	int begin = 0;
	int end = 0;
	int lineBegin = 0;
	int lineEnd = 0;
	int loopScope = 0;
	int counter = 0;
	std::string iterator;
	std::string expression;
	std::string increment;
	std::string loopType = "";
};

//An object of this class represents a variable in the program. Meant to be stored in a umap called var_map
//contains a static vector of datatypes
class Cmmvariable {
public:
	//With a value
	Cmmvariable(std::string t, std::string value, int scope, bool isList) {
		type = t;
		setValue(value);
		this->scope = scope;
		this->isList = isList;
	}
	//If declared with no value
	Cmmvariable(std::string t, int scope, bool isList) {
		type = t;
		setValueStringDefault();
		this->scope = scope;
		this->isList = isList;
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

	bool getListStatus() {
		return isList;
	}

	std::vector<std::string> getValueStringList() {
		return valueStringList;
	}

	void addToList(std::string nValue) {
		updateValue(nValue);
		valueStringList.push_back(valueString);
	}

	void updateList(std::string index, std::string nValue) {
		updateValue(nValue);
		try {
			valueStringList.at(stoi(index)) = valueString;
		}
		catch (std::out_of_range) {
			warnings.push_back(warnStr + " " + index + " is out of range.");
		}
		catch (std::invalid_argument) {
			warnings.push_back(warnStr + " Possible incorrectly coded list. listName[index] is standard.");
		}
	}

	std::string getValueAtIndex(std::string index) {
		std::string value = "";
		try {
			value = valueStringList.at(stoi(index));
		}
		catch (std::out_of_range) {
			warnings.push_back(warnStr + " " + index + " is out of range.");
		}
		catch (std::invalid_argument) {
			warnings.push_back(warnStr + " Possible incorrectly coded list, index entered is " + index + ". listName[index] is standard.");
		}

		return value;
	}

	int getListSize() {
		return valueStringList.size();
	}

	//This function will take a varName and if it is a list will return its name
	static std::string getListName(std::string varName) {
		int varNameEnd = 0;
		bool isVarList = false;
		for (int i = 0; i < varName.size(); i++) {
			if (varName[i] == '[') {
				isVarList = true;
				varNameEnd = i;
				break;
			}
		}

		if (isVarList) {
			varName = varName.substr(0, varNameEnd);
		}
		return varName;
	}

	//This function will return an index string, if it is not a list then the index will be blank
	static std::string getListIndex(std::string varName, std::unordered_map<std::string, Cmmvariable> var_map) {
		std::string index = "";
		bool gettingIndex = false;

		for (int i = 0; i < varName.size(); i++) {
			if (varName[i] == '[') {
				gettingIndex = true;
				i++;
			}
			if (varName[i] == ']') {
				gettingIndex = false;
			}
			if (gettingIndex) {
				index += varName[i];
			}
		}
		
		try {
			int tempI = stoi(index);
		}
		catch (std::invalid_argument) {
			std::string tempVarName = getListName(index);
			if (var_map.find(tempVarName) != var_map.end()) {
				if (var_map.at(tempVarName).getListStatus()) {
					std::string tempIndex = getListIndex(tempVarName, var_map);
					index = var_map.at(tempVarName).getValueAtIndex(tempIndex);
				}
				else {
					index = var_map.at(tempVarName).getValueString();
				}
			}
		}

		return index;
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
	std::vector<std::string> valueStringList;
	bool isList = false;

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
	//Currently supports lists as well, display a list with no index will display the last changed result or default if nothing was changed
	static void display(line thisLine, std::unordered_map<std::string, Cmmvariable> var_map) {

		std::string tempVarName = "";
		bool gettingVarName = false;	
		int varStartLocation, varStrSize;
		int nameLength = 0;

		//Searches through the var location and var name vectors in the line struct and inserts them into the literal
		for (int i = 0; i < thisLine.varNameLocations.size(); i++ ) {
			if (var_map.at(thisLine.vars.at(i)).getListStatus()) {
				for (auto lil : thisLine.varIndexs) {
					if (lil.matchingVarsIndex == i)
					thisLine.literal.insert(thisLine.varNameLocations.at(i) + nameLength, var_map.at(thisLine.vars.at(i)).getValueAtIndex(lil.index));
					nameLength += var_map.at(thisLine.vars.at(i)).getValueAtIndex(lil.index).size() - 1;
				}
			}
			else {
				thisLine.literal.insert(thisLine.varNameLocations.at(i) + nameLength, var_map.at(thisLine.vars.at(i)).getValueString());
				nameLength += var_map.at(thisLine.vars.at(i)).getValueString().size() - 1;
			}
		}
		
		std::cout << thisLine.literal;
	}

	//This function reads the next line and stores the lines value into that variable
	//Currently supports reading to lists
	static void read(std::string lineStr, std::unordered_map<std::string, Cmmvariable> var_map) {

		std::string varName;
		for (int i = 4; i < lineStr.size(); i++) {
			varName += lineStr[i];
		}

		std::string index = Cmmvariable::getListIndex(varName, var_map);
		varName = Cmmvariable::getListName(varName);

		if (var_map.find(varName) != var_map.end()) {
			std::string temp;
			std::cin >> temp;

			if (var_map.at(varName).getListStatus()) {
				var_map.at(varName).updateList(index, temp);
			}
			else {
				var_map.at(varName).updateValue(temp);
			}
		}
		else {
			warnings.push_back(warnStr + " attempted to read to a variable that does not exist");
		}
	}
	
	//This function returns true if an if statement evaluated to true; else it returns false
	//This function calls the evaluate function below
	static bool conditionalIF(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {
		//The if code is 1
		return evaluate(thisLine, var_map, 1);
	}

	//This function takes a line, code and var_map and compares two values on either side of an expression.
	//Evaluate has support for lists coded
	static bool evaluate(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map, int code) {
		//the code is representing what keyword needs to use evaluate
		// 1-> if	2-> elif	3-> loop while
		//this is very horrible... 
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
		std::string leftIndex, rightIndex;
		bool doingLeftValue = true;
		bool notEqual = false;
		bool greater = false;
		bool greaterE = false;
		bool lesser = false;
		bool lesserE = false;
		bool intCompare = false;
		bool doubleCompare = false;

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
		leftIndex = Cmmvariable::getListIndex(leftValue, var_map);
		std::string tempLeft = Cmmvariable::getListName(leftValue);

		if (var_map.find(tempLeft) != var_map.end()) {
			leftValue = tempLeft;
		
			if (var_map.at(leftValue).getType() == "dec") {
				doubleCompare = true;
			}
			else if (var_map.at(leftValue).getType() == "int") {
				intCompare = true;
			}

			if (var_map.at(leftValue).getListStatus()) {
				leftValue = var_map.at(leftValue).getValueAtIndex(leftIndex);
			}
			else {
				leftValue = var_map.at(leftValue).getValueString();
			}
		}

		rightIndex = Cmmvariable::getListIndex(rightValue, var_map);
		std::string tempRight = Cmmvariable::getListName(rightValue);

		if (var_map.find(tempRight) != var_map.end()) {
			rightValue = tempRight;
			if (var_map.at(rightValue).getType() == "dec") {
				doubleCompare = true;
			}
			else if (var_map.at(rightValue).getType() == "int") {
				intCompare = true;
			}

			if (var_map.at(rightValue).getListStatus()) {
				rightValue = var_map.at(rightValue).getValueAtIndex(rightIndex);
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
		if (intCompare) {
			try {
				int temp = stoi(leftValue);
				int temp2 = stoi(rightValue);
			}
			catch (std::invalid_argument) {
				warnings.push_back(warnStr + " Invalid type comparison");
				return false;
			}
		}
		else if (doubleCompare) {
			try {
				int temp = stod(leftValue);
				int temp2 = stod(rightValue);
			}
			catch (std::invalid_argument) {
				warnings.push_back(warnStr + " Invalid type comparison");
				return false;
			}
		}


		if (greater) {
			if (greaterE) {
				if (intCompare) {
					if (stoi(leftValue) >= stoi(rightValue)) {
						return true;
					}
					return false;
				}
				else if (doubleCompare) {
					if (stod(leftValue) >= stod(rightValue)) {
						return true;
					}
					return false;
				}
				if (leftValue >= rightValue) {
					return true;
				}
				return false;
			}
			if (intCompare) {
				if (stoi(leftValue) > stoi(rightValue)) {
					return true;
				}
				return false;
			}
			else if (doubleCompare) {
				if (stod(leftValue) > stod(rightValue)) {
					return true;
				}
				return false;
			}
			if  (leftValue > rightValue) {
				return true;
			}
			return false;
		}
		if (lesser) {
			if (lesserE) {
				if (intCompare) {
					if (stoi(leftValue) <= stoi(rightValue)) {
						return true;
					}
					return false;
				}
				else if (doubleCompare) {
					if (stod(leftValue) <= stod(rightValue)) {
						return true;
					}
					return false;
				}
				if (leftValue <= rightValue) {
					return true;
				}
				return false;
			}
			if (intCompare) {
				if (stoi(leftValue) < stoi(rightValue)) {
					return true;
				}
				return false;
			}
			else if (doubleCompare) {
				if (stod(leftValue) < stod(rightValue)) {
					return true;
				}
				return false;
			}
			if (leftValue < rightValue) {
				return true;
			}
			return false;
		}
		if (intCompare) {
			if (stoi(leftValue) == stoi(rightValue)) {
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
		else if (doubleCompare) {
			if (stod(leftValue) == stod(rightValue)) {
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

	//This function will check fill out some extra information for the loop if it needs it, like if the iterator is 'i += 2'
	static void complexLoopCreater(Loop& loop) {
		std::string newIterator;
		for (int i = 0; i < loop.iterator.size(); i++) {
			if (loop.iterator[i] == '=' || (loop.iterator[i] == '+' && (loop.iterator[i + 1] == '+' || loop.iterator[i + 1] == '=')) || (loop.iterator[i] == '-' && (loop.iterator[i + 1] == '-' || loop.iterator[i + 1] == '='))) {
				loop.complex = true;
				loop.increment += loop.iterator[i];
				i++;
				if (loop.iterator[i] == '+' || loop.iterator[i] == '=' || loop.iterator[i] == '-') {
					loop.increment += loop.iterator[i];
					i++;
				}
			}
			if (!loop.complex) {
				newIterator += loop.iterator[i];
			}
			else if (loop.complex) {
				loop.expression += loop.iterator[i];
			}

		}
		loop.iterator = newIterator;
	}

	//This is the function for doing loops
	//This function currently supports lists
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
				if (line[i] == '-' && rangeFloor !=  "") {
					i++;
					isFloor = false;
				}
				if (isFloor) {
					rangeFloor += line[i];
				}
				else if (!isFloor) {
					rangeCeil += line[i];
				}
			}

			//If either of the range numbers are an int value then set the range numbers to that variables value
			std::string indexFloor = Cmmvariable::getListIndex(rangeFloor, var_map);
			std::string tempFloor = Cmmvariable::getListName(rangeFloor);

			if (var_map.find(tempFloor) != var_map.end()) {
				rangeFloor = tempFloor;
				if (var_map.at(rangeFloor).getType() == "int") {
					if (var_map.at(rangeFloor).getListStatus()) {
						rangeFloor = var_map.at(rangeFloor).getValueAtIndex(indexFloor);
					}
					else {
						rangeFloor = var_map.at(rangeFloor).getValueString();
					}
				}
			}
			std::string indexCeil = Cmmvariable::getListIndex(rangeCeil, var_map);
			std::string tempCeil = Cmmvariable::getListName(rangeCeil);

			if (var_map.find(tempCeil) != var_map.end()) {
				rangeCeil = tempCeil;
				if (var_map.at(rangeCeil).getType() == "int") {
					if (var_map.at(rangeCeil).getListStatus()) {
						rangeCeil = var_map.at(rangeCeil).getValueAtIndex(indexCeil);
					}
					else {
						rangeCeil = var_map.at(rangeCeil).getValueString();
					}
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
		keywords.push_back("from");
		keywords.push_back("while");
		return keywords;
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

				std::string index = Cmmvariable::getListIndex(varName, var_map);
				varName = Cmmvariable::getListName(varName);
				
				if (var_map.find(varName) != var_map.end()) {
					thisLine.varNameLocations.push_back(thisLine.literal.size() + thisLine.varNameLocations.size());
					thisLine.vars.push_back(varName);
					if (var_map.at(varName).getListStatus()) {
						lineIndexLocations lil;
						lil.matchingVarsIndex = thisLine.vars.size() - 1;
						lil.index = index;
						thisLine.varIndexs.push_back(lil);
					}
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
		}
		return true;
	}
	std::string tempWord = "";
	for (auto elem : Keyword::getKeywords()) {
		for (int i = 0; i < elem.size(); i++) {
			tempWord += word[i];
		}

		if (tempWord == elem) {
			return true;
		}
		if (word == elem) {
			return true;
		}
		tempWord = "";
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

	var_map.emplace(name, Cmmvariable(type, value, scope, false));
}

//Calling this function will kill all the variables of whatever scope has been passed.
void scopeVarDestroyer(std::unordered_map<std::string, Cmmvariable>& var_map, int scope) {
	std::vector<std::string> erase_keys;
	for (auto elem : var_map) {
		if (elem.second.getScope() >= scope) {
			erase_keys.push_back(elem.first);
		}
	}

	for (auto elem : erase_keys) {
		var_map.erase(elem);
	}
}

//This will update a variables value if it needs to be
void updateVar(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map) {

	std::string varName = "";
	std::string expression = "";

	bool doesVarExist = false;
	bool isExpressionVar = false;
	bool isVarList = false;
	std::string index = "";

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

	index = Cmmvariable::getListIndex(varName, var_map);
	varName = Cmmvariable::getListName(varName);

	//make sure that varname exists in the map and then set the bool to true. if it doesnt create a warning
	if (var_map.find(varName) != var_map.end()) {
		doesVarExist = true;
		isVarList = var_map.at(varName).getListStatus();
	}
	else {
		warnings.push_back(warnStr + " " + varName + " is not an existing variable");
	}

	//if the expression is just a single variable
	if (var_map.find(expression) != var_map.end()) {
		isExpressionVar = true;
	}

	if (doesVarExist) {
		//FOR STRING -- List supported
		if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(4)) {
			if (isExpressionVar) {
				if (isVarList) {
					var_map.at(varName).updateList(index, var_map.at(expression).getValueString());
				}
				else {
					var_map.at(varName).updateValue(var_map.at(expression).getValueString());
				}
			}
			else {
				//Searches through the var location and var name vectors in the line struct and inserts them into the literal
				for (int i = 0; i < thisLine.varNameLocations.size(); i++) {
					thisLine.literal.insert(thisLine.varNameLocations.at(i), var_map.at(thisLine.vars.at(i)).getValueString());
				}
				if (isVarList) {
					var_map.at(varName).updateList(index, thisLine.literal);
				}
				else {
					var_map.at(varName).updateValue(thisLine.literal);
				}
			}
		}
		//FOR CHAR
		else if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(2)) {
			if (isExpressionVar) {
				if (isVarList) {
					var_map.at(varName).updateList(index, var_map.at(expression).getValueString());
				}
				else {
					var_map.at(varName).updateValue(var_map.at(expression).getValueString());
				}
			}
			else {
				if (isVarList) {
					var_map.at(varName).updateList(index, expression);
				}
				else {
					var_map.at(varName).updateValue(expression);
				}
			}
		}
		//FOR INTEGER AND DECIMAL -- List Supported
		else if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(0) || var_map.at(varName).getType() == Cmmvariable::getTypes().at(1)) {
			if (isExpressionVar) {
				if (isVarList) {
					var_map.at(varName).updateList(index, var_map.at(expression).getValueString());
				}
				else {
					var_map.at(varName).updateValue(var_map.at(expression).getValueString());
				}
			}
			else {
				if (isVarList) {
					var_map.at(varName).updateList(index, calculate(expression, var_map.at(varName).getType(), var_map));
				}
				else {
					var_map.at(varName).updateValue(calculate(expression, var_map.at(varName).getType(), var_map));
				}
			}
		}
		//FOR BOOLEAN -- List supported
		else if (var_map.at(varName).getType() == Cmmvariable::getTypes().at(3)) {
			if (isExpressionVar) {
				if (isVarList) {
					var_map.at(varName).updateList(index, var_map.at(expression).getValueString());
				}
				else {
					var_map.at(varName).updateValue(var_map.at(expression).getValueString());
				}
			}
			else {
				if (isVarList) {
					var_map.at(varName).updateList(index, expression);
				}
				else {
					var_map.at(varName).updateValue(expression);
				}
			}
		}
	}
}

//This will initialize a list
void initializeList(line thisLine, std::unordered_map<std::string, Cmmvariable>& var_map, std::string listName) {

	std::string expression = "";
	std::string listSize, listVals;
	bool doingSize = true;

	//If an equal sign was found set the left to varname and the right to expression
	size_t found = thisLine.lineStr.find("=");
	if (found != std::string::npos) {
		for (int i = found + 1; i < thisLine.lineStr.size(); i++) {
			expression += thisLine.lineStr[i];
		}
	}

	for (int i = 0; i < expression.size(); i++) {
		if (expression[i] == ',') {
			doingSize = false;
			i++;
		}
		if (doingSize) {
			listSize += expression[i];
		}
		else {
			listVals += expression[i];
		}
	}

	for (int i = 0; i < stoi(listSize); i++) {
		var_map.at(listName).addToList(listVals);
	}
}

//This function will create a new variable
void createVar(line thisLine, std::string type, std::unordered_map<std::string, Cmmvariable>& var_map, int scope) {

	std::string dVarName = "";
	std::string varName = "";
	bool defaultDec = true;
	int equalLocation = 0;
	bool isList = false;

	if (thisLine.lineStr.substr(3, 4) == "list") {
		isList = true;
	}

	//this finds the var name
	if (!isList) {
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
	}
	else {
		for (int i = 7; i < thisLine.lineStr.size(); i++) {
			if (thisLine.lineStr[i] == '=') {
				equalLocation = i;
				defaultDec = false;
				for (int j = 7; j < i; j++) {
					varName += thisLine.lineStr[j];
				}
			}
			dVarName += thisLine.lineStr[i];
		}
	}

	thisLine.lineStr.erase(0, 3);

	if (defaultDec) {
		//The variable name was a bad word
		if (isBadWord(dVarName)) {
			warnings.push_back(warnStr + " variable name was invalid");
			return;
		}

		//The variable name already exists
		if (var_map.find(dVarName) != var_map.end()) {
			warnings.push_back(warnStr + " variable already exists");
				return;
		}

		var_map.emplace(dVarName, Cmmvariable(type, scope, isList));
	}
	else {

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

		var_map.emplace(varName, Cmmvariable(type, scope, isList));
		if (!isList) {
			updateVar(thisLine, var_map);
		}
		else {
			initializeList(thisLine, var_map, varName);
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

	if (lines.size() == 0) {
		warnings.push_back("WARNING: Command or file typed incorrectly or does not exist.");
		return;
	}

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


		if (lineNum > 0) {
			if (lines.at(lineNum).scope < lines.at(lineNum - 1).scope) {
				if (nestedLoopCounter == -1) {
					scopeVarDestroyer(var_map, lines.at(lineNum - 1).scope);
				}
				else if (!loops.at(nestedLoopCounter).doingLoop) {
					scopeVarDestroyer(var_map, lines.at(lineNum - 1).scope);
				}
			}
		}

		//This chunk turns a loop off if it needs to be turned off or it sets the do-while back to the start of the loop
		if (nestedLoopCounter != -1) {
			if (loops.at(nestedLoopCounter).doingLoop) {
				if (access < loops.at(nestedLoopCounter).loopScope) {
					access = loops.at(nestedLoopCounter).loopScope;
				}

				//When the loop reaches the last line of the loop its gonna increment and do all of this stuff
				if (loops.at(nestedLoopCounter).loopScope > lines.at(lineNum).scope) {
					scopeVarDestroyer(var_map, loops.at(nestedLoopCounter).loopScope + 1);
					if (loops.at(nestedLoopCounter).loopType == "from") {
						std::string incrementBy;
						if (loops.at(nestedLoopCounter).complex) {
							incrementBy = calculate(loops.at(nestedLoopCounter).expression, "int", var_map);
							if (loops.at(nestedLoopCounter).increment == "=") {
								loops.at(nestedLoopCounter).counter = stoi(incrementBy);
							}
							else if (loops.at(nestedLoopCounter).increment == "+=") {
								loops.at(nestedLoopCounter).counter += stoi(incrementBy);
							}
							else if (loops.at(nestedLoopCounter).increment == "-=") {
								loops.at(nestedLoopCounter).counter -= stoi(incrementBy);
							}
							else if (loops.at(nestedLoopCounter).increment == "--") {
								loops.at(nestedLoopCounter).counter--;
							}
							else if (loops.at(nestedLoopCounter).increment == "++") {
								loops.at(nestedLoopCounter).counter++;
							}
						}
						else {
							loops.at(nestedLoopCounter).counter++;
						}

						if (loops.at(nestedLoopCounter).counter > loops.at(nestedLoopCounter).end) {
							loops.at(nestedLoopCounter).doingLoop = false;
							access = lines.at(lineNum).scope;
							scopeVarDestroyer(var_map, loops.at(nestedLoopCounter).loopScope);
							if (nestedLoopCounter > 0) {
								loops.pop_back();
								nestedLoopCounter--;
							}
						}
						else {
							var_map.at(loops.at(nestedLoopCounter).iterator).updateValue(std::to_string(loops.at(nestedLoopCounter).counter));
							lineNum = loops.at(nestedLoopCounter).lineBegin;
							continue;
						}
					}
					//When a while loop reaches its ending line
					else if (loops.at(nestedLoopCounter).loopType == "while") {
						//this line will check the evaluation of the loop parameters and change doingloop if needed
						loops.at(nestedLoopCounter).doingLoop = Keyword::evaluate(lines.at(loops.at(nestedLoopCounter).lineBegin - 1), var_map, 3);
						if (loops.at(nestedLoopCounter).doingLoop) {
							lineNum = loops.at(nestedLoopCounter).lineBegin;
							access = lines.at(lineNum).scope;
						}
						else {
							scopeVarDestroyer(var_map, loops.at(nestedLoopCounter).loopScope);
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
					Keyword::complexLoopCreater(loops.at(nestedLoopCounter));
					scopeVarDestroyer(var_map, loops.at(nestedLoopCounter).loopScope);
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
			if (nestedLoopCounter != -1) {
				if (loops.at(nestedLoopCounter).doingLoop) {
					line emptyLine;
					emptyLine.lineStr = "";
					lines.push_back(emptyLine);
					running = true;
				}
			}
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

	bool running = true;

	system("clear");
	std::cout << "\nWelcome to C--";

	while (running) {
		std::unordered_map<std::string, Cmmvariable> var_map;
		std::cout << "\n\n";
		std::string fileName = "";
		std::cout << "Enter file name to run, 'edit' to edit, or 'quit' to quit" << std::endl;
		std::cin >> fileName;
		std::cout << "\n\n";

		if (fileName == "quit") {
			running = false;
		}
		else if (fileName == "edit") {
			std::cout << "Enter a file name to edit(vim): " << std::endl;
			std::cin >> fileName;
			fileName = "vim " + fileName;
			const char* editCommand = fileName.c_str();
			std::cout << "command is: " << editCommand << std::endl;
			system(editCommand);
		}
		else {
			openFile(fileName, var_map);
		}

		//This will display all created warnings to the terminal at the end of the program
		std::cout << "\n\n";
		for (auto elem : warnings) {
			std::cout << elem << std::endl;
		}
		warnings.clear();
	}

	//This section is for debugging. If uncommented it will display all the variables, name/type/value to the terminal
	/*for (auto elem : var_map) {
		if (elem.second.getListStatus()) {
			std::cout << "List: " << elem.first << std::endl;
			for (auto list : elem.second.getValueStringList()) {
				std::cout << list << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "Name: " << elem.first << "\tType: " << elem.second.getType() << "\tValue: " << elem.second.getValueString() << "\tScope: " << elem.second.getScope() << std::endl;
	}*/
}