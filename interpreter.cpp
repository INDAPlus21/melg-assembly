#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <any>
#include <tuple>
#include <algorithm>
using namespace std;

// List of instructions with name and arguments (register index or int value)
vector<pair<string, vector<int>>> instructions;

int registers[] = {0, 0, 0, 0};

// Declare functions so that they can be called in logical order
void readinput(char *argv[]);
void parseinstructions(vector<vector<string>> inputinstructions);
void runprogram();

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        cout << "Error: you need to enter a file to run. Run: ./interpreter.exe FILENAME.bbvv where FILENAME is the name of the file that you want to run\n";
        exit(1);
    }

    readinput(argv);
    runprogram();
    return 0;
}

void readinput(char *argv[])
{
    // Read file
    ifstream input(argv[1]);

    if (input.is_open() && input.good())
    {
        vector<vector<string>> inputinstructions;

        string line;
        while (getline(input, line))
        {
            // Split by space
            // Source for string split: https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
            stringstream ss(line);
            vector<string> result;

            bool linevalid = false;
            while (ss.good())
            {
                string substring;
                getline(ss, substring, ' '); // Get string until ,

                // Don't push comments
                if (substring.rfind("//", 0) == 0)
                {
                    break;
                }

                // Prevent pushing empty strings
                if (substring.length() > 0)
                {
                    if (!linevalid)
                    {
                        inputinstructions.push_back(vector<string>());
                    }

                    linevalid = true;
                    result.push_back(substring);
                    inputinstructions[inputinstructions.size() - 1].push_back(substring);
                }

                if (!linevalid)
                {
                    // Keep empty lines as jump depends on them
                    inputinstructions.push_back(vector<string>{"empty"});
                }
            }

            // Parse
        }
        input.close();

        parseinstructions(inputinstructions);
    }
    else
    {
        cout << "Error: the file you have entered cannot be opened, check that a file with the name exists\n";
        exit(1);
    }
}

// Parse and validate instructions
void parseinstructions(vector<vector<string>> inputinstructions)
{
    bool error = false;

    for (int i = 0; i < inputinstructions.size(); i++)
    {
        // Check that instructions have the correct size
        vector<string> instruction = inputinstructions[i];
        string instructionname = instruction[0];

        if (instructionname == "add" || instructionname == "sub" || instructionname == "set" || instructionname == "jeq")
        {
            if (instruction.size() != 4)
            {
                cout << "Error: wrong amount of arguments for instruction #" << i << "\n";
                error = true;
            }

            vector<int> registryindexes;

            // Check all registers
            for (int j = 1; j < instruction.size() - 1; j++)
            {
                if (instruction[j][0] != '#')
                {
                    cout << "Error: instruction #" << i << " argument #" << j << " is not prefixed with # which all registries shall be\n";
                    error = true;
                }

                replace(instruction[j].begin(), instruction[j].end(), '#', ' '); // Remove registry char

                int registryindex = stoi(instruction[j]);
                registryindexes.push_back(registryindex);
                if (registryindex < 0 || (j == 1 && registryindex < 1) || registryindex > 3)
                {
                    cout << "Error: instruction #" << i << " argument #" << j << " is not a valid registry. There are only 4 registries to choose from\n";
                    error = true;
                }
            }

            // Check intermediate
            int intermediate = stoi(instruction[3]);
            if (intermediate != 0 && intermediate != 1)
            {
                cout << "Error: instruction #" << i << " argument #2 is not a valid number. There are two options: 0 and 1\n";
                error = true;
            }

            // Add parsed instruction
            instructions.push_back(make_pair(instructionname, vector<int>{registryindexes[0], registryindexes[1], intermediate}));
        }
        else if (instructionname == "j")
        {
            if (instruction.size() != 2)
            {
                cout << "Error: wrong amount of arguments for instruction #" << i << "\n";
                error = true;
            }

            int jumpcount = stoi(instruction[1]);
            if (jumpcount > 15 || jumpcount < -16)
            {
                cout << "Error : instruction #" << i << " contains a value that is too small or large\n";
                error = true;
            }

            // Add parsed instruction
            instructions.push_back(make_pair(instructionname, vector<int>{jumpcount}));
        }
        else if (instructionname == "input" || instructionname == "print" || instructionname == "exit")
        {
            if (instruction.size() != 1)
            {
                cout << "Error: wrong amount of arguments for instruction #" << i << "\n";
                error = true;
            }

            // Add parsed instruction
            instructions.push_back(make_pair(instructionname, vector<int>{}));
        }
        else if (instructionname == "empty")
        {
            instructions.push_back(make_pair(instructionname, vector<int>{}));
        }
        else
        {
            cout << "Error: instruction #" << i << " is not a valid command\n";
            cout << "Error: the instruction that fails is: ";

            for (string part : instruction)
            {
                cout << part << " ";
            }

            cout << "\n";

            error = true;
        }
    }

    if (error)
    {
        exit(1);
    }
}

void runprogram()
{
    int currentline = 0;

    while (true)
    {
        // Prevent infinite loop
        if (currentline >= instructions.size())
        {
            cout << "Error: program has reached end of file, explicity exit with an exit command instead\n";
            break;
        }

        pair<string, vector<int>> instruction = instructions[currentline];
        string instructionname = instruction.first;
        vector<int> arguments = instruction.second;

        // Run instruction
        if (instructionname == "add")
        {
            registers[arguments[0]] += registers[arguments[1]];
            registers[arguments[0]] += arguments[2];
        }
        else if (instructionname == "sub")
        {
            registers[arguments[0]] -= registers[arguments[1]];
            registers[arguments[0]] -= arguments[2];
        }
        else if (instructionname == "set")
        {
            registers[arguments[0]] = registers[arguments[1]];
            registers[arguments[0]] += arguments[2];
        }
        else if (instructionname == "jeq")
        {
            if (registers[arguments[0]] == registers[arguments[1]])
            {
                currentline += arguments[2]; // Jump forward by imm + 1
            }
            else if (arguments[2] == 0)
            {
                currentline++; // Jump two lines forward
            }
        }
        else if (instructionname == "j")
        {
            currentline += arguments[0];
        }
        else if (instructionname == "input")
        {
            cin >> registers[1];
        }
        else if (instructionname == "print")
        {
            cout << registers[1] << "\n";
        }
        else if (instructionname == "exit")
        {
            break;
        }

        currentline++;
    }
}