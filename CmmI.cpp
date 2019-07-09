#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

class variables {
public:
	variables(std::string n, std::string t, int loc) {
		name = n;
		type = t;
		location = loc;
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
	std::string name;
	std::string type;
	int location;
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
	var_vector.push_back(variables("numOfDogs", "int", var_vector.size()));
	var_map[var_vector.at(0).getName()] = var_vector.at(0).getLocation();

	for (auto elem : var_vector) {
		std::cout << "Name: " << elem.getName() << "\tType: " << elem.getType();
	}

	std::cout << std::endl;

	for (auto elem : var_map) {
		std::cout << "Name: " << elem.first << "\tLocation: " << elem.second;
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