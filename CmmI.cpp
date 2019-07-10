#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

class variables {
public:
	variables(std::string n, std::string t, int loc, std::string value) {
		name = n;
		type = t;
		location = loc;
		setValue(value);
	}

	void setValue(std::string value) {
		//INTEGER
		if (type == datatypes.at(0)) {
			try {
				iValue = stoi(value);
				valueString = std::to_string(iValue);
			}
			catch (std::out_of_range) {
				//create warning
			}
			catch (std::invalid_argument) {
				//create warning
			}

		}
		//DECIMAL
		else if (type == datatypes.at(1)) {
			try {
				dValue = stod(value);
				valueString = std::to_string(dValue);
				int precision = 0;
				for (int i = 0; i < valueString.size(); i++) {
					if (valueString[i] != '0') {
						precision = i;
					}
				}

				valueString.erase(valueString.begin() + precision + 1, valueString.end());
			}
			catch (std::out_of_range) {
				//create warning
			}
			catch (std::invalid_argument) {
				//create warning
			}
		}
		//CHAR
		else if (type == datatypes.at(2)) {
			cValue = value[0];
			valueString += cValue;
			if (value.size() > 1) {
				//Create warning
			}
		}
		//BOOL
		else if (type == datatypes.at(3)) {
			if (value == "true" || value == "1") {
				bValue = true;
				valueString = "true";
			}
			else if (value == "false" || value == "0") {
				bValue = false;
				valueString = "false";
			}
			else {
				//create warning
			}
		}
		//STRING
		else if (type == datatypes.at(4)) {
			sValue = value;
			valueString = sValue;
		}

	}
	
	std::string getValueString() {
		return valueString;
	}

	std::string getName() {
		return name;
	}

	std::string getType() {
		return type;
	}
	
	int getLocation() {
		return location;
	}

private:
	std::vector<std::string> datatypes = { "int", "dec", "chr", "bol", "str" };
	std::string name;
	std::string type;
	int location;
	int iValue;
	double dValue;
	char cValue;
	bool bValue;
	std::string sValue;
	std::string valueString = "";
};

struct line {
	std::string lineStr;
	std::string literal = "";
	int num;
};
//this function opens the file to be read
void openFile(std::string fileName) {
	std::ifstream ifs(fileName);

	line thisLine;
	thisLine.num = 1;
	while (getline(ifs, thisLine.lineStr)) {
		//readLine(thisLine);
		//thisLine.num++;
	}
}

//Main
int main() {

	std::unordered_map<std::string, int> var_map;
	std::vector<variables> var_vector;
	int numOfVars = var_vector.size();

	var_vector.push_back(variables("numOfDogs", "int", numOfVars, "4"));
	var_map[var_vector.at(numOfVars).getName()] = var_vector.at(numOfVars).getLocation();
	numOfVars = var_vector.size();

	var_vector.push_back(variables("numOfCats", "dec", numOfVars, "12.000020"));
	var_map[var_vector.at(numOfVars).getName()] = var_vector.at(numOfVars).getLocation();
	numOfVars = var_vector.size();

	var_vector.push_back(variables("numOfBirds", "chr", numOfVars, "83"));
	var_map[var_vector.at(numOfVars).getName()] = var_vector.at(numOfVars).getLocation();
	numOfVars = var_vector.size();

	var_vector.push_back(variables("doYouHaveAName", "bol", numOfVars, "true"));
	var_map[var_vector.at(numOfVars).getName()] = var_vector.at(numOfVars).getLocation();
	numOfVars = var_vector.size();

	for (auto elem : var_vector) {
		std::cout << "Name: " << elem.getName() << "\t\t\tType: " << elem.getType() << "\tValue: " << elem.getValueString() << std::endl;
	}

	std::cout << std::endl;

	/*
	string fileName = "";
	cout << "Welcome to C-- v1" << endl << "Enter file name to run" << endl;
	cin >> fileName;
	cout << "\n\n";
	openFile(fileName);
	*/
}