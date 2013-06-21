//============================================================================
// Name        : CppConsole.cpp
// Author      : Richard Hoblitzell
// Version     : 0.0.1
// Description : Console Application for C++ to dynamic execute code.
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

static int sBraceCount = 0; /**< keeps count of how deeply the braces are nested
                             *  within the code*/

/**
 * This method trims the whitespaces off the left side of a string.
 * @param str parameter string that will be trimmed
 * @return a string with the white spaces trimmed off the left side.
 */
string ltrim(string str) {
  size_t startpos = str.find_first_not_of(" \t");
  if (string::npos != startpos) return str.substr(startpos);
  else return str;
}

/**
 * This method determines if a given file exists.
 * @param filename the name of the file
 * @return 1 if the file exists, 0 otherwise
 */
bool file_exists(const char* filename) {
  ifstream ifile(filename);
  return ifile;
}

/**
 * This method determines if a directory exists.
 * @param directory name of the directory
 * @return 1 if the directory exists, 0 otherwise
 */
bool directory_exists(const char* directory) {
  struct stat st;
  if (stat(directory, &st) == 0) return S_ISDIR(st.st_mode);
  else return false;
}

/**
 * Copies a file to a new file.
 * @param from file being copied
 * @param to file being coped to
 */
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

/**
 * This method will 'back-up' a given file.  For the implementation of this
 * application, it is performed by simply renaming the file with the extention
 * .bak.  It is assumed that the original filename will be quickly replaced, or
 * rolled back to its original name.
 * @param filename file that will be renamed
 */
void backup_file(string filename) {
  rename(filename.c_str(), (filename + ".bak").c_str());
}

/**
 * This method will rollback a file by renaming the its backed file to the
 * original name.  It is assumed that backup_file() was previously called and
 * that a *.bak() version of the file exists.
 * @param filename name of the backed file.
 */
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

      //Get the current working directory.
      char cwd[256];
      getcwd(cwd, 256);

      //Change the directory to where the makefile is located.
      chdir(sMakeDir.c_str());

      //Perform a make in the makefile directory. Output is piped to grep for
      //the reasons explained above.
      system("make --silent 2>&1 | grep error:");

      //Return to the original directory to ensure successive system calls work.
      chdir(cwd);

      //Restore the project main.cpp.
      rollback_file(sProjMainPath);
    }

    //Check the executable's existence to determine compilation success.
    if (file_exists(sExecPath.c_str())) return 0;
    else return -1;
  }
}

/**
 * This method will reset the code buffer back to a state where it can accept
 * new user input.  It loads in the current main.cpp file and leaves out the
 * bottom of the file after the main.cpp return.  In this state, user code may
 * easily be appended to the end of the buffer.
 */
void reload_code_buffer() {
  //Wipe out the entire content of the buffer.
  sCodeBuffer.str("");
  sCodeBuffer.clear();

  //Open the main.cpp file to read data.
  ifstream inputFile;
  inputFile.open(sMainPath.c_str());

  string line;
  while (getline(inputFile, line)) {
    //We can quit reading once we reach the return in main.cpp.
    if (ltrim(line) == "return EXIT_SUCCESS;") {
      break;
    } else if (strncmp(ltrim(line).c_str(), "cpp_console_print(", 18) != 0) {
      //Code will be added to the buffer if it is NOT a console print statement.
      sCodeBuffer << line << "\n";
    }
  }

  inputFile.close();
}

/**
 * This method will execute the given codeStream passed in.
 * @param codeStream stream of C++ code that contains a compilable main() method
 * @return 0 if compilation is successful
 */
int execute(ostringstream* codeStream) {
  int output = 0;

  //Backup the main file before it gets overwritten by the code stream.
  backup_file(sMainPath);

  //Write the code stream to the main file.
  ofstream outputFile;
  outputFile.open(sMainPath.c_str());
  outputFile << codeStream->str();
  outputFile.close();

  //Backup the current executable before compiling.
  backup_file(sExecPath);

  //Attempt to compile
  output = compile();

  if (output == 0) {
    //If compiling is successful, get the full path of the executable and send
    //it to system() to compile.
    char full_path[256];
    realpath(sExecPath.c_str(), full_path);
    system(full_path);
  } else {
    //If compilation failed, rollback the main file to remove syntax errors.
    rollback_file(sMainPath);
  }

  //Rollback the executable after it is done.  This is to prevent a project
  //executable from getting modified.  This only applies when the console in
  //running in a project directory.
  rollback_file(sExecPath);

  //Now that the main file is in its final state, the code buffer can safely
  //be reloaded to accept new user input.
  reload_code_buffer();

  return output;
}

/**
 * This method creates the configuration template that is loaded into the
 * main.cpp file when the console is reloaded.
 */
void create_config_template() {
  ofstream output;
  output.open(sConfigPath.c_str());

  output << "//<Add additional includes and namespaces here.>\n";
  output << "#include <stdlib.h>\n";
  output << "#include <iostream>\n";
  output << "#include <sstream>\n";
  output << "using namespace std;\n";
  output << "\n";
  output << "//This method is required to allow the CppConsole to print.\n";
  output << "// *** Do not remove this method.\n";
  output << "template <typename T>\n";
  output << "void cpp_console_print(T value){\n";
  output << "  stringstream ss;\n";
  output << "  ss << value << \"\\n\";\n";
  output << "  cout << ss.str();\n";
  output << "}\n";
  output << "\n";
  output << "int main(int argc, char** argv) {\n";
  output << "//<Add custom initialization logic here.>\n";
  output << "  return EXIT_SUCCESS;\n";
  output << "}\n";

  output.close();
}

/**
 * This method loads the configuration template into a stream.
 * @param stream the pointer to the stream loaded with template data
 */
void load_config_template(ostringstream* stream) {
  ifstream inputFile;
  inputFile.open(sConfigPath.c_str());
  string line;
  while (getline(inputFile, line)) {
    *stream << line << "\n";
  }
  inputFile.close();
}

/**
 * This method will reload the main file back to its original state by loading
 * the configuration template.
 * @return 0 if execution was successful
 */
int reload() {
  cout << "Reloading...\n";

  //If the template doesn't exist, create a new one.
  if (!file_exists(sConfigPath.c_str()))
    create_config_template();

  //Load the template data into a stream.
  ostringstream stream;
  load_config_template(&stream);

  //Execute the stream to verify if the reload was successful.
  return execute(&stream);
}

/**
 * This method will add an #include or namespace to the main file.
 * @param str the line of code containing an include or namespace
 */
void add_includes(string str) {
  //Create a stream to store the code.
  ostringstream output;

  //Pre-pend the stream with the include/namespace information.
  output << str << "\n";

  //Add the code in the current main file to the stream.
  ifstream inputFile;
  inputFile.open(sMainPath.c_str());
  string line;
  while (getline(inputFile, line)) {
    output << line << "\n";
  }
  inputFile.close();

  //Execute the code to verify the syntax and update the main file.
  execute(&output);
}

/**
 * This method will add a new line of code to the main file.
 * @param str the new line of code to add
 */
void add_code(string str) {
  //Add the code to the buffer.
  sCodeBuffer << str << "\n";

  //Count the braces and keep track of how deeply they are nested.
  sBraceCount += count(str.begin(), str.end(), '{');
  sBraceCount -= count(str.begin(), str.end(), '}');
  if (sBraceCount < 0) sBraceCount = 0;

  //If the code is not nested in any braces, we can attempt to execute.
  if (sBraceCount == 0) {
    //The end of main() must be appended to the buffer before execution.
    sCodeBuffer << "return EXIT_SUCCESS;" << "\n";
    sCodeBuffer << "}\n";
    execute(&sCodeBuffer);
  }
}

/**
 * This method prints the command line help when the user encounters an error.
 */
void print_commandline_help() {
  cout << "Usage: ccpconsole [<project main cpp> <project executable file> [options]]\n";
  cout << "Options:\n";
  cout << "  -m\tProvide project makefile directory. (defaults to '.')\n";
}

/**
 * This method inspects the commandline arguments and applies the proper settings
 * @param argc number of commandline arguments
 * @param argv the commandline arguments
 * @return 0 if successful, -1 otherwise
 */
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

/**
 * This method removes all generated files.  Only the configuration template
 * should be left behind after termination.
 */
void clean_files() {
  remove(sMainPath.c_str());
  remove((sMainPath + ".bak").c_str());

  if (sProjMainPath == "") {
    remove(sExecPath.c_str());
  }
}

/**
 * This method prints the console header.
 */
void print_console_prefix() {
  string brace_str;
  if (sBraceCount == 0) brace_str = ":";
  for (int i = 0; i < sBraceCount; i++) {
    brace_str += "{";
  }

  cout << "CppConsole" << brace_str << ">";
}

/**
 * This method evaluates a command and prints it to the user.
 * @param cmd the command string being evaluated
 */
void evaluate_command(string cmd) {
  //Evaluation will only occur if the code is not nested and the command is set.
  if ((sBraceCount == 0) && (cmd != "")) {
    //To display the result to the user, the command must be inserted into the
    //cpp_console_print() method.
    replace(cmd.begin(), cmd.end(), ';', ' ');
    string print_cmd = string("cpp_console_print(") + cmd + string(");");

    //Execute the code to perform the evaluation.
    add_code(print_cmd);
  }
}

/**
 * The main method of the code the reads user input.
 * @param argc number of commandline arguments
 * @param argv the commandline arguments
 * @return EXIT_SUCCESS
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
          add_includes(input_str);
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
