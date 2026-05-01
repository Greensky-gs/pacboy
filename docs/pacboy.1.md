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

Pacboy has 2 modes : installation mode, which is triggered when no other modes are found, requiring the **first argument** being the library that will be included, and the **second argument** being the path where it is installed. The other modes are triggered by the various command options : `--generate-config`, that will generate a configuration file, `-D` or `-I` for Database and Information modes, `--change-config`, to update pacboy's configuration, `-P` pull changes from database, and `--help` or `-h` for help

The order of detecting modes is :

1. Help, through                  : `-h` or `--help`
2. Database                       : `-D`
3. Config mode                    : `--generate-config`
4. Information mode               : `-I`
5. Change config                  : `--change-config`
6. Pull database                  : `-P`
7. Default is installation mode

## HELP

Help mode will display help page, no option can alter the behavior of this mode.

## DATABASE

In Database mode, it will simply list all the registered libraries. No option can modify the behavior of this mode

## CONFIGURATION

In configuration mode, pacboy will generate a configuration file, with the given function names. You must give the function names in comma-separated list, using the `--deps` option, and you can redirect the output from *stdout* to `/path/to/dir/paquet.boy` by specifying `/path/to/dir` in the `-od` option (standing for **output directory**). Pacboy will automatically add the `/paquet.boy` at the end of the path.

In order to specify the **features** of the paquet, you need to use `--features` in a comma-separated list of values. If some features require specific functions, specify them inside brackets ( `[]` ), in a comma-separated list of values. For more informations about features, refer the to the **FEATURES** section

    pacboy --generate-config -od ~/paclibs/args --deps "streq,parse_int"
    pacboy --generate-config --deps "streq"
    pacboy -od "./paclibs/string-chained-list" --deps "streq" --generate-config
    pacboy --generate-config --deps "streq" --features "clone,comparison[ordcompare,eqcompare]"

Pacboy can have configuration files for its libraries. It contains the dependencies. In pacboy, depencies are functions that the library expected in its C or H files, assumed it existed but never defined. The names inside the configuration file, below the `[deps]` are the functions the library is assuming are defined. An example of configuration file (*paquet.boy*) can be this :

     [deps]
     streq
     disassemble_array
     [features]
     sync
     async
      - thread
      - process

In this example, the library is expecting the user to give the according H files for the function declaration. The `[features]` part concerns **FEATURES**

## INFORMATION

In information mode, Pacboy will show the informations of the given library, such informations being :

- The name
- The location
- If it has any, the list of expected functions
- An example of a command that could be used to install it

## Change config

In Change config, pacboy will prompt the user for successive parameters, to answer in the terminal. Leaving a field blank doesn't change it. Fields that can be modified :

- `repo_url` : The url of the repo. More on that in **PULL**

## PULL

In pull mode, pacboy will execute a succession of commands, being :

1. Remove local packages
2. Cloning the configured database : `git clone -b mirror --only-branch <configured url> <path of libs>`
3. Removing the `.git` folder

Note the **-b mirror** that specifies the mirror branch only. This is because the database must contain exclusively the folders for the libraries, and nothing else, so a dedicated branch should be set up.

## INSTALLATION

In installation mode, Pacboy looks for a folder in `~/paclibs`, given as the first argument of the command. It then reads the configuration file it it exists to try to find the required functions declaration files. More on this below. If not all the functions are found, it will throw an error. When all the headers file are given through the `--include` parameter, in the format of : `function_name=header_file.h,function_name2=header_file2.h`, it will copy-paste the names in the includes. Since it is custom libraries and custom usage, it is up to the user to make sure the paths are correct, and valid in relative pathing.

    pacboy make ./
    pacboy string-chained-list ./src/structs --include "streq=../aux/tools.h"
    pacboy pyassembler ./src/assemblers/ --include "*=../aux/tools.h"
    pacboy args ./src/aux --include "streq=../aux/tools/string.h,parse_int=../aux/parsers/int.h"

# FEATURES

Pacboy offers a features system, to filter the needs.

Basically, a feature is a folder in a library. For instance, a library having "tool.c" at the root, and "aux" as a folder at root, "aux" is considered a feature. Every folder at root is considered a feature, and every feature is a folder at root, so, for instance, in `./aux/parsers/`, **parsers** is not considered a feature

A feature can have specific dependencies. In order to generate a config file with a feature needing dependencies, use the `--features` flag, followed by something like :
`"feature_name[dependency_1,dependency_2,...]`. If a feature required no dependency, don't pass the brackets.

To include features in installation, use the `--features` flag. It will automatically add the dependencies of the feature in the required dependencies needed to be passed in `--include`

# OPTIONS

-h, \--help : Display help page

-D : Enters database mode. It just shows the list of available libraries.

-od *path to directory* : Redirect the output of **CONFIGURATION MODE** to a file named `paquet.boy` in the given directory. This option only has effect in configuration mode.

-I : Enters information mode. The name of the library must be specified as the first argument of the program

\--generate-config : Enter config mode. This option must go along with the **\--deps** option, in the form of comma-separated function names that are expected to be defined by the library.

\--include *comma-separated pairs of function-names/header files* : Specify the header files in installation mode that the library requires externally declared functions. The paths must be valid in relative pathing.

\--deps *comma-separeted function names* : Specify the names of the required libraries for **CONFIGRATION MODE**

\--change-config : Enter the configuration mode

-P : Pull the database from git.

\--features : Specify the features, for either **INSTALLATION MODE**, or **CONFIGURATION MODE**. Refer to the according part of the manual for more information

# RETURN VALUE

The command returns 0 if everything went alright, 1 otherwise

# SEE ALSO

`pacman` (8)
