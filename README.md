# Pacboy

A simple library manager to handle my custom C libraries, to copy them as needed

## How it works

You create a folder name `~/paclibs`, you put one folder for each library you need, each folder containing the necessaries C and header files.

Then you run `pacboy <the name of the folder> <path/to/where/it/will/be/pasted>`

The **content** of the folder will be copied, not the folder itself

### Database

Optionnally, you can use a database, which is a distant git repository containing the `~/paclibs` folder on a branch named **mirror**. You can configure this by running `apcboy --change-config` and entering your url, or mine if you just want a general database : [https://github.com/Greensky-gs/paclibs-database](https://github.com/Greensky-gs/paclibs-database). If you use mine, just make sure you run `pacboy -P` to get the latest changes

## Installation

If you want to install it for yourself, you will first need :

* gcc
* make
* git (to clone the repo)

Then you can do :

1. `git clone https://github.com/Greensky-gs/pacboy && cd pacboy` to clone the repo
2. `make cleanbuild` to create the executable
3. Move the binary to a folder in your path. If you want to have it globally : `sudo cp bin/main.uwu /usr/bin/pacboy`, I usually put it in my bin folder : `cp bin/main.uwu ~/bin/pacboy` - just make sure the `~/bin` is in your $PATH
4. Optionnaly create the man page entry : `sudo make manpage`

## Usage

You can read the help first by running `pacboy -h`

We have more detailled informations in the **man page**

