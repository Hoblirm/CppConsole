//============================================================================
// Name        : CppConsole.cpp
// Author      : Richard Hoblitzell
// Version     : 0.0.1
// Description : Console Application for Cpp inspired by the Ruby irb.
//============================================================================

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

static ostringstream sCodeBuffer;

static const string sApp = "cpp_console";
static string sMainPath = sApp + ".cpp";
static string sExecPath = sApp + ".exe";

static string sProjMainPath = "";
static string sMakeDir = ".";

static int sBraceCount = 0;

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
  rename(filename.c_str(),(filename+".bak").c_str());
}

void rollback_file(string filename) {
  rename((filename+".bak").c_str(),filename.c_str());
}

int compile() {
  string cmd;
  if (sProjMainPath == "") {
    cmd = string("g++ -o ") + sExecPath + " " + sMainPath;
    return system(cmd.c_str());
  } else {

    backup_file(sProjMainPath);
    copy(sMainPath,sProjMainPath);

    int output = system("make");

    rollback_file(sProjMainPath);

    return output;
  }
}

void reload_code_buffer() {
  sCodeBuffer.str("");
  sCodeBuffer.clear();

  ifstream inputFile;
  inputFile.open(sMainPath.c_str());
  string line;
  while (getline(inputFile, line)) {
    if (line == "return 0;") {
      break;
    } else if (strncmp(line.c_str(), "puts(", 5) != 0) {
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
    realpath(sExecPath.c_str(),full_path);
    system(full_path);
  } else {
    rollback_file(sMainPath);
  }

  rollback_file(sExecPath);
  reload_code_buffer();

  return output;
}

int reload() {
  cout << "Reloading...\n";

  ostringstream stream;
  stream << "#include <stdlib.h>\n";
  stream << "#include <iostream>\n";
  stream << "#include <sstream>\n";
  stream << "using namespace std;\n";
  stream << "\n";
  stream << "template <typename T>\n";
  stream << "void puts(T value){\n";
  stream << "stringstream ss;\n";
  stream << "ss << value << \"\\n\";\n";
  stream << "cout << ss.str();\n";
  stream << "}\n";
  stream << "int main(int argc, char** argv) {\n";
  stream << "return 0;\n";
  stream << "}\n";

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
  sCodeBuffer << "return 0;" << "\n";
  sCodeBuffer << "}\n";
  execute(&sCodeBuffer);
}

void add_code(string str) {
  if (str[str.length()-1] == '{') sBraceCount++;
  else if ((sBraceCount>0)&&(str[str.length()-1] == '}')) sBraceCount--;

  sCodeBuffer << str << "\n";

  if (sBraceCount == 0) execute_code_buffer();
}



int set_parameters(int argc, char** argv) {
  int success = 0;

  if (argc > 1) {
    sProjMainPath = string(argv[1]);

    for (int i = 2; i < argc; i += 2) {
      if (argv[i] == "-m") {
        sMakeDir = argv[i + 1];
      } else if (argv[i] == "-e") {
        sExecPath = argv[i + 1];
      } else {
        cout << "Flag of '" << argv[i] << "' is invalid.\n";
        cout << "Valid flags are -e (executable file name) and -m (makefile directory)\n";
        success = -1;
        break;
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

/*
 *
 */
int main(int argc, char** argv) {

  if ((set_parameters(argc, argv) == 0) && (reload() == 0)) {

    string input_str;
    while (input_str != "exit") {

      string brace_str;
      if (sBraceCount == 0) brace_str = ":";
      for (int i=0;i<sBraceCount;i++){
        brace_str += "{";
      }

      cout << "CppConsole" << brace_str << ">";
      getline(cin, input_str);

      if (input_str == "") {
        //no-op;
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

  clean_files();
  return 0;
}