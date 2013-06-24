CppConsole
==========

This application is a C++ console which allows a developer to quickly execute C++ code without going through the painful process of recompiling.  The application gives the developer a sandbox to play with new code without disturbing their current project.  Using this application can greatly reduce downtime by allowing the developer to spend more time coding and less time compiling.  CppConsole is intended to mimic scripting language consoles such as Ruby's irb.   

Installation
============

CppConsole performs several Linux system calls and will not be compatible for Windows.  If you are are fortunate enough to be on Linux, you can quickly set-up CppConsole by compiling main.cpp into an executable.  Using the make command should generate the executable 'cppconsole'.  Add the executable to /usr/bin OR add the project directory to the PATH environment variable.  Verify that the executable can be called by typing 'which cppconsole'.  The executable directory should be displayed.  That is it! You should now be ready to use CppConsole.

Getting Started
===============

To get started, simply open a terminal and type 'cppconsole'.  This should start CppConsole in the terminal.

CppConsole has five simple commands:
- **reload!** - reloads CppConsole and resets all declared variables and included header files.
- **include** - same syntax as C.  This will include the specified header file.
- **using namespace** - same sytax as C.  This will include the specified namespace.
- **"\n"** (Entering a blank line) - this will attempt to evaluate the last line of C code and print it to the console.
- **exit** - This will exit CppConsole.
 
Everything else will be interpreted as C code and the user will be notified of any syntax errors.  Semi-colons are necessary!

As an example we'll test out the strftime() method in the console.  The strftime() method requires time.h, so include it as a header by typing:

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
strftime(buffer,80,"The date is: %B, %d, %Y",now);
```

Now we would like to see the contents of buffer.  To print to the screen we can simply type a blank line.  However, the strftime() method returns an integer which we are not interested in.  To get the contents of buffer, enter this:

```bash
buffer;
```

Press Enter twice.  This should display today's date in the console.

For this example it took more time declaring the variables than it took to execute the desired method.  To speed up this process, CppConsole generates a template that can be modified to include initialization code.  The template name is cpp_console.config, and it is generated in the current working directory each time cppconsole is executed.  We are going to repeat this example by using variables preinitialized by the template.  Open cpp_console.config and add this to the top of the file:

```bash
#include <time.h>
```

Add the following code to the template's main method:

```bash
time_t rawtime;
struct tm * now;
char buffer[256];
time(&rawtime);
now = localtime(&rawtime);
```

Now restart CppConsole by typing 'exit' and restarting the application:

```bash
exit
cppconsole
```

The console should be initialized with the contents of the template file.  Now simply enter the lines:

```bash
strftime(buffer,256,"The date is: %B, %d, %Y",now);
buffer;
```

Press Enter twice.  As you can see, having commonly used headers and variables in the template can save time.  It is a good idea to give the predefined varaiables generic names such as 'now' to help remember them in the future.

Run on Existing Projects
========================

Executing generic code is great, but what if you wanted to tweak some custom class methods in your application.  Lucky for you, CppConsole has the capability to tie into an existing C Project.  This can be done by opening the terminal and moving to the project's main directory.  Once inside, CppConsole can be tied to the project by typing the command:

```bash
cppconsole <path of project main.cpp> <path of project executable>
```

CppConsole assumes that the makefile is located in the project's main directory.  If it is located elsewhere, use the -m option to specify the makefile path. Once CppConsole is initialized, the #include command should be able to add any of the project's header files.  It would be a good idea to add most of the project headers into the cpp_console.config file.  Reread the getting started section if you are not familiar with the .config file.  

Limitations
===========
Since C++ is not a scripting language, it is important to point out that CppConsole is not dynamically executing each line of code.  In reality, it is recompiling the code each time a new line is entered.  All lines of code are executed on each compile.  Therefore, it is important to type 'reload!' often to clear out old code that is no longer needed.  In addition, if the code communicates with external hardware/software, it can have unreliable results since the external entities may recieve duplicate requests due to re-execution. CppConsole is not intended for all projects, but it can be an excellent development tool when used correctly. 
