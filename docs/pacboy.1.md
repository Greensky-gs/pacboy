% Pacboy(1) Pacboy User Manuals
% Greensky-gs
% April 2026

# NAME

pacboy - Custom paquet manager

# SYNOPSIS

pacboy <*library name*> [*options*]
pacboy <*options*>

See **OPTIONS** below

# DESCRIPTION

Pacboy is a small derivative of pacman, to manage local custom libraries. It is written in C and functions by copying the files it finds at the location into the destination

Pacboy has 2 modes : installation mode, which is triggered when no other modes are found, requiring the **first argument** being the library that will be included, and the **second argument** being the path where it is installed. The other modes are triggered by the various command options : `--generate-config`, that will generate a configuration file, `-D` or `-I` for Database and Information modes, and `--help` or `-h` for help

The order of detecting modes is :

1. Help, through `-h` or `--help`
2. Database, throught `-D`
3. Config mode : `--generate-config`
4. Information mode : `-I`
5. Default is installation mode

Help mode will display help page, no option can alter the behavior of this mode.

In Database mode, it will simply list all the registered libraries. No option can modify the behavior of this mode

In configuration mode, pacboy will generate a configuration file, with the given function names. You must give the function names in comma-separated list, using the `--deps` option, and you can redirect the output from *stdout* to `/path/to/dir/paquet.boy` by specifying `/path/to/dir` in the `-od` option (standing for **output directory**). Pacboy will automatically add the `/paquet.boy` at the end of the path

    pacboy --generate-config -od ~/paclibs/args --deps "streq,parse_int"
    pacboy --generate-config --deps "streq"
    pacboy -od "./paclibs/string-chained-list" --deps "streq" --generate-config

In information mode, Pacboy will show the informations of the given library, such informations being :

- The name
- The location
- If it has any, the list of expected functions
- An example of a command that could be used to install it

In installation mode, Pacboy looks for a folder in `~/paclibs`, given as the first argument of the command. It then reads the configuration file it it exists to try to find the required functions declaration files. More on this below. If not all the functions are found, it will throw an error. When all the headers file are given through the `--include` parameter, in the format of : `function_name=header_file.h,function_name2=header_file2.h`, it will copy-paste the names in the includes. Since it is custom libraries and custom usage, it is up to the user to make sure the paths are correct, and valid in relative pathing.

    pacboy make ./
    pacboy string-chained-list ./src/structs --include "streq=../aux/tools.h"
    pacboy pyassembler ./src/assemblers/ --include "*=../aux/tools.h"
    pacboy args ./src/aux --include "streq=../aux/tools/string.h,parse_int=../aux/parsers/int.h"

Pacboy can have configuration files for its libraries. It contains the dependencies. In pacboy, depencies are functions that the library expected in its C or H files, assumed it existed but never defined. The names inside the configuration file, below the `[deps]` are the functions the library is assuming are defined. An example of configuration file (*paquet.boy*) can be this :

     [deps]
     streq
     disassemble_array

In this example, the library is expecting the user to give the according H files for the function declaration.

# OPTIONS

-h, \--help : Display help page

-D : Enters database mode. It just shows the list of available libraries.

-od *path to directory* : Redirect the output of **CONFIGURATION MODE** to a file named `paquet.boy` in the given directory. This option only has effect in configuration mode.

-I : Enters information mode. The name of the library must be specified as the first argument of the program

\--generate-config : Enter config mode. This option must go along with the **\--deps** option, in the form of comma-separated function names that are expected to be defined by the library.

\--include *comma-separated pairs of function-names/header files* : Specify the header files in installation mode that the library requires externally declared functions. The paths must be valid in relative pathing.

\--deps *comma-separeted function names* : Specify the names of the required libraries for **CONFIGRATION MODE**

# RETURN VALUE

The command returns 0 if everything went alright, 1 otherwise

# SEE ALSO

`pacman` (8)
