//============================================================================
// Name        : CppConsole.cpp
// Author      : Richard Hoblitzell
// Version     : 0.0.1
// Description : Console Application for Cpp inspired by the Ruby irb.
//============================================================================
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

static ostringstream sCodeBuffer; /**< stream used to store user input.*/

static const string sApp = "cpp_console"; /**< string of the application name.
                                           * This is used for naming various files that are generated.*/
static string sMainPath = sApp + ".cpp"; /**< string of the main.cpp path that the console writes to.*/
static string sConfigPath = sApp + ".config"; /**< string of the config path that the console generate the main.cpp from.*/
static string sExecPath = sApp + ".exe"; /**< string of the executable path that the console compiles to.*/

static string sProjMainPath = ""; /**< string of the project's main.cpp path.
                                   * This file is temporarily replaced by sMainPath before compilation.*/
static string sMakeDir = "."; /**< string of the project's makefile directory.*/

static int sBraceCount = 0;

string ltrim(string str) {
  size_t startpos = str.find_first_not_of(" \t");
  if (string::npos != startpos)  return str.substr(startpos);
  else return str;
}

bool file_exists(const char* filename) {
  ifstream ifile(filename);
  return ifile;
}

bool directory_exists(const char* directory) {
  struct stat st;
  if (stat(directory, &st) == 0) return S_ISDIR(st.st_mode);
  else return false;
}

void copy(string from, string to) {
  ifstream inputFile;
  inputFile.open(from.c_str());

  ofstream outputFile;
  outputFile.open(to.c_str());

  string line;
  while (getline(inputFile, line)) {
    outputFile << line << "\n";
  }
  outputFile.close();
  inputFile.close();
}

void backup_file(string filename) {
  rename(filename.c_str(), (filename + ".bak").c_str());
}

void rollback_file(string filename) {
  rename((filename + ".bak").c_str(), filename.c_str());
}

/**
 * This method will compile the sMainPath file by performing a make in the
 * the directory sMakeDir.  The method is assuming that sExecPath does not
 * currently exist.  This is neccessary since the file existence is checked
 * to determine if the compile was successful.
 * @return 0 if successful
 */
int compile() {
  //If the file exists, it means that it was not backed up.
  if (file_exists(sExecPath.c_str())) {
    cout << "Error: compile() failed since the executable was not backed up.\n";
    return -1;
  } else {
    string cmd;
    if (sProjMainPath == "") {
      //The g++ command prints compilation warnings that are not desired.
      //Therefore, the output is piped to grep, which will only display errors.
      //Since the pipe prevents the make status from being read, success is
      //determined by checking if the executable exists.
      cmd = string("g++ -w -o ") + sExecPath + " " + sMainPath + " 2>&1 | grep error:";
      system(cmd.c_str());

    } else {
      //Overwrite the project main.cpp with the Cpp Console main.cpp.
      backup_file(sProjMainPath);
      copy(sMainPath, sProjMainPath);

      char cwd[256];
      getcwd(cwd, 256);

      chdir(sMakeDir.c_str());

      //Perform a make in the makefile directory. Output is piped to grep for
      //the reasons explained above.
      system("make --silent 2>&1 | grep error:");

      chdir(cwd);

      //Restore the project main.cpp.
      rollback_file(sProjMainPath);
    }

    //Check the executable's existence to determine compilation success.
    if (file_exists(sExecPath.c_str())) return 0;
    else return -1;
  }
}

void reload_code_buffer() {
  sCodeBuffer.str("");
  sCodeBuffer.clear();

  ifstream inputFile;
  inputFile.open(sMainPath.c_str());
  string line;
  while (getline(inputFile, line)) {
    if (ltrim(line) == "return EXIT_SUCCESS;") {
      break;
    } else if (strncmp(ltrim(line).c_str(), "cpp_console_print(", 18) != 0) {
      sCodeBuffer << line << "\n";
    }
  }

  inputFile.close();
}

int execute(ostringstream* codeStream) {
  int output = 0;
  backup_file(sExecPath);
  backup_file(sMainPath);

  ofstream outputFile;
  outputFile.open(sMainPath.c_str());
  outputFile << codeStream->str();
  outputFile.close();

  output = compile();
  if (output == 0) {
    char full_path[256];
    realpath(sExecPath.c_str(), full_path);
    system(full_path);
  } else {
    rollback_file(sMainPath);
  }

  rollback_file(sExecPath);
  reload_code_buffer();

  return output;
}

void create_config_template() {
  ostringstream stream;
  stream << "#include <stdlib.h>\n";
  stream << "#include <iostream>\n";
  stream << "#include <sstream>\n";
  stream << "using namespace std;\n";
  stream << "\n";
  stream << "template <typename T>\n";
  stream << "void cpp_console_print(T value){\n";
  stream << "  stringstream ss;\n";
  stream << "  ss << value << \"\\n\";\n";
  stream << "  cout << ss.str();\n";
  stream << "}\n";
  stream << "\n";
  stream << "int main(int argc, char** argv) {\n";
  stream << "  return EXIT_SUCCESS;\n";
  stream << "}\n";

  ofstream outputFile;
  outputFile.open(sConfigPath.c_str());
  outputFile << stream.str();
  outputFile.close();
}

void load_config_template(ostringstream* stream) {
  ifstream inputFile;
  inputFile.open(sConfigPath.c_str());
  string line;
  while (getline(inputFile, line)) {
    *stream << line << "\n";
  }
  inputFile.close();
}

int reload() {
  cout << "Reloading...\n";

  if (!file_exists(sConfigPath.c_str()))
    create_config_template();

  ostringstream stream;
  load_config_template(&stream);

  return execute(&stream);
}

void add_includes(string str) {
  ostringstream output;
  output << str << "\n";

  ifstream inputFile;
  inputFile.open(sMainPath.c_str());
  string line;
  while (getline(inputFile, line)) {
    output << line << "\n";
  }
  inputFile.close();

  execute(&output);
}

void add_namespace(string str) {
  add_includes(str);
}

void execute_code_buffer() {
  sCodeBuffer << "return EXIT_SUCCESS;" << "\n";
  sCodeBuffer << "}\n";
  execute(&sCodeBuffer);
}

void add_code(string str) {
  sBraceCount += count(str.begin(), str.end(), '{');
  sBraceCount -= count(str.begin(), str.end(), '}');
  if (sBraceCount < 0) sBraceCount = 0;

  sCodeBuffer << str << "\n";

  if (sBraceCount == 0) execute_code_buffer();
}

void print_commandline_help() {
  cout << "Usage: ccpconsole [<project main cpp> <project executable file> [options]]\n";
  cout << "Options:\n";
  cout << "  -m\tProvide project makefile directory. (defaults to '.')\n";
}

int set_parameters(int argc, char** argv) {
  int success = 0;

  if (argc == 2) {
    cout << "cppconsole: *** The Project executable file was not provided.\n";
    print_commandline_help();
    success = -1;
  } else if (argc > 2) {
    sProjMainPath = string(argv[1]);
    sExecPath = string(argv[2]);

    if (!file_exists(sProjMainPath.c_str())) {
      cout << "cppconsole: *** Could not find project main file: " << sProjMainPath << "\n";
      print_commandline_help();
      success = -1;
    } else {
      if (!file_exists(sExecPath.c_str())) {
        cout << "cppconsole: *** Could not find project executable file: " << sExecPath << "\n";
        print_commandline_help();
        success = -1;
      } else {
        for (int i = 3; i < argc; i += 2) {
          if (string(argv[i]) == "-m") {
            sMakeDir = argv[i + 1];
            if (!directory_exists(sMakeDir.c_str())) {
              cout << "cppconsole: *** Could not find makefile directory: " << sMakeDir << "\n";
              print_commandline_help();
              success = -1;
              break;
            }
          } else {
            cout << "cppconsole: *** Invalid option '" << argv[i] << "'.\n";
            print_commandline_help();
            success = -1;
            break;
          }
        }
      }
    }
  }

  return success;
}

void clean_files() {
  string cmd;
  cmd = string("rm -rf " + sMainPath);

  remove(sMainPath.c_str());
  remove((sMainPath + ".bak").c_str());

  if (sProjMainPath == "") {
    remove(sExecPath.c_str());
  }
}

void print_console_prefix() {
  string brace_str;
  if (sBraceCount == 0) brace_str = ":";
  for (int i = 0; i < sBraceCount; i++) {
    brace_str += "{";
  }

  cout << "CppConsole" << brace_str << ">";
}

void evaluate_command(string cmd) {
  if ((sBraceCount == 0) && (cmd != "")) {
    replace(cmd.begin(), cmd.end(), ';', ' ');
    string print_cmd = string("cpp_console_print(") + cmd + string(");");
    add_code(print_cmd);
  }
}

/*
 *
 */
int main(int argc, char** argv) {

  if (set_parameters(argc, argv) == 0) {
    if (reload() != 0) {
      cout << "cppconsole: *** Compile failed. Ensure " << sConfigPath << " has no syntax errors.\n";
    } else {
      string last_input;
      string input_str = "";
      while (input_str != "exit") {
        print_console_prefix();

        last_input = input_str;
        getline(cin, input_str);

        if (input_str == "") {
          evaluate_command(last_input);
        } else if (strncmp(input_str.c_str(), "#include", 8) == 0) {
          add_includes(input_str);
        } else if (strncmp(input_str.c_str(), "using namespace", 15) == 0) {
          add_namespace(input_str);
        } else if (input_str == "reload!") {
          reload();
        } else if (input_str == "exit") {
          break;
        } else {
          add_code(input_str);
        }
      }
    }
  }

  clean_files();
  return EXIT_SUCCESS;
}
