CppConsole
==========

This application is a C++ console which allows a developer to quickly execute C++ code without going through the painful process of recompiling.  The application gives the developer a sandbox to play with new code without disturbing their current project.  Using this application can greatly reduce downtime by allowing the developer to spend more time coding and less time compiling.  CppConsole is intended to mimic scripting language consoles such as Ruby's irb.   

Installation
============

CppConsole performs several Linux system calls and will not be compatible for Windows.  If you are are fortunate enough to be on Linux, you can quickly set-up CppConsole by compiling main.cpp into an executable.  Using the make command should generate the executable 'cppconsole'.  Add the executable to /usr/bin OR add the project directory to the PATH environment variable.  Verify that the executable can be called by typing 'which cppconsole'.  The command-line calls are listed below:

```bash
cd <CppConsole project directory>
make
sudo mv cppconsole /usr/bin
which cppconsole
```

If the executable directory is displayed, then CppConsole should be properly configured.  You should now be ready to use CppConsole.

Getting Started
===============

Before starting CppConsole, you should be familiar with the commands.  CppConsole has a few simple commands:
- **#include** - same syntax as C.  This will include the specified header file.
- **using namespace** - same sytax as C.  This will include the specified namespace.
- **static** - same syntax as C.  This will declare static variables and methods.  **All methods defined within CppConsole must be declared as static or you'll recieve a syntax error.
- **!** - use in place of a semi-colon to force a single execution.  This command cannot be used within methods or if-statements.  However, it can be applied to the end of a large if-statement block.  See the examples below for more information about preventing re-execution.
- **@** - works the same as '!' except it will evaluate the command and print it to the screen.  Some commands cannot be printed, and will not execute if they cannot be printed.  This should be used in place of cout/printf to avoid re-executed output.
- **reload!** - reloads CppConsole and clears out all user declared variables and commands.  
- **dump!** - this displays all the user commands since the last reload. This is good to use when you want to copy/paste console commands.
- **exit** - This will exit CppConsole.
 
Everything else will be interpreted as C code and the user will be notified of any syntax errors.  Semi-colons are necessary!

As an example we'll test out the strftime() method in the console.  Before anything else, ensure that CppConsole is running:

```bash
cppconsole
```

Once CppConsole starts up, we are ready to being the example.  Since we'll be using the strftime() method, we'll need to include the time.h header.  Enter this into the console:

```bash
#include <time.h>
```

Now we must type in the remaining code to call the strftime() method:

```bash
time_t rawtime;
struct tm * now;
char buffer[80];
time(&rawtime);
now = localtime(&rawtime);
strftime(buffer,80,"The date is: %B %d, %Y",now);
```

Now we would like to see the contents of buffer.  To get the contents of buffer, enter this:

```bash
buffer@
=> The date is: June 26, 2013
```

This should display today's date in the console.

For this example it took more time declaring the variables than it took to execute the desired method.  To speed up this process, CppConsole generates a template that can be modified to include initialization code.  The template name is cpp_console.config, and it is generated in the current working directory each time cppconsole is executed.  We are going to repeat this example by using variables preinitialized by the template.  Quickly exit CppConsole:

```bash
exit
```

The current directory should contain the config file.  Open the file to make the necessary changes:

```bash
vim cpp_console.config
```

Add the 'time.h' header to the top of the config file:

```bash
#include <time.h>
```

In addtion, type the following code in the template's main method:

```bash
time_t rawtime;
struct tm * now;
char buffer[256];
time(&rawtime);
now = localtime(&rawtime);
```

Close the file and execute CppConsole:

```bash
cppconsole
```

The console should be initialized with the contents of the template file.  Now simply enter the lines:

```bash
strftime(buffer,256,"The date is: %B %d, %Y",now);
buffer@
=> The date is: June 26, 2013
```
As you can see, having commonly used headers and variables in the template can save time.  It is a good idea to give the predefined varaiables generic names such as 'now' to help remember them in the future.

Declaring Methods
=================
In this example we'll cover adding methods through the console.  The only trick for declaring a method is to remember that they must be static.  Add the following lines in the console:

```bash
static int s=0;

static string increment(int v, int *p) {
  s++; v++; 
  *p = *p+1;
  ostringstream stream;
  stream << "Inside the method: s=" << s << " v=" << v << " p=" << *p << "\n";
  return stream.str();
}
```

Now that the method has been declared, we can use it inside the code:

```bash
int v=0, p=0;
string str = increment(v,&p);
str@
=> Inside the method: s=1 v=1 p=1
```

We can check the variables after the method and should only expect p and s to be incremented:

```bash
v@
=> 0
```
```bash
p@
=> 1
```
```bash
s@
=> 1
```

Prevent Re-execution
====================
When using CppConsole, it is important to remember that C++ is not a scripting language.  All code typed into the console is stored until reload! is called.  Each successive command will re-execute all the stored code.  Therefore it is important to use reload! when your code is no longer needed.  This next example shows when it is appropriate to use the '!' command.

Open cpp_console.config and copy/paste the following code above the main() method: 

```bash
#include <sstream>
#include <fstream>

static string read_file(){
  ostringstream stream; //Input the file into a stream.
  ifstream input;
  input.open("tmp.txt");
  string line;
  while (getline(input, line)) {
    stream << line << "\n";
  }
  input.close();
  
  return stream.str(); //Return the string.
}

static void append_file(string str){
  ostringstream stream; //Get the original file contents
  stream << read_file();
  
  stream << str << "\n"; //Append the string
  
  ofstream output; //Write the appended text to the file.
  output.open("tmp.txt");
  output << stream.str();
  output.close();
} 

static ostringstream sStream;
static string read_string(){
  return sStream.str();
} 
static void append_string(string str){
  sStream << str << "\n";
} 

```

Now start CppConsole and use the append method:

```bash
cppconsole
append_string("Adding line 1 to string.");
append_file("Adding line 1 to file.");
```

Now use the read method:

```bash
read_file()@
=> Adding line 1 to file.
=> Adding line 1 to file.
read_string()@
=> Adding line 1 to string.
```

As you can see, the appending to a string worked correctly, but not for the file.  Re-execution will reset console variables, but not external entities.  Lets retry this example using the '!' method:

```bash
remove("tmp.txt");
reload!
append_string("Adding line 1 to string.")!
append_file("Adding line 1 to file.")!
read_string()@
=> 
read_file()@
=> Adding line 1 to file.
```

This time the string method failed to give the appropriate result.  It is important to realize that using '!' command will not store changes made to variables in the command. To give the correct results, you must know when to use '!' only when it is appropriate:

```bash
remove("tmp.txt");
reload!
append_string("Adding line 1 to string.");
append_file("Adding line 1 to file.")!
read_string()@
=> Adding line 1 to string.
read_file()@
=> Adding line 1 to file.
```


Run on Existing Projects
========================

Executing generic code is great, but what if you wanted to tweak some custom class methods in your application.  Lucky for you, CppConsole has the capability to tie into an existing C Project.  This can be done by opening the terminal and moving to the project's main directory.  Once inside, CppConsole can be tied to the project by typing the command:

```bash
cppconsole <path of project main.cpp> <path of project executable>
```

CppConsole assumes that the makefile is located in the project's main directory.  If it is located elsewhere, use the -m option to specify the makefile path. Once CppConsole is initialized, the #include command should be able to add any of the project's header files.  It would be a good idea to customize your config file to include commonly used project headers and write necessary initialization code for your project.  Reread the getting started section if you are not familiar with the .config file. For larger projects CppConsole may run slow due to longer compilation times.  To improve performance, you can create custom makefiles to compile subsets of your project.  Just remember to use the -m option to specify the makefile.
