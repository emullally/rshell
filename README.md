# rshell

## A Basic Command Shell

This program, rshell, is a basic command shell similar to a limited version
of the Bash shell. This program allows for multiple commands to be entered at one time using any combination of `;`, `||`, and `&&` commands and executes commands found in the `PATH` and loops continuously until the `exit` command is entered. Comments can be entered after a `#` symbol, similar to Bash comments.

### How to Use

Simply run the program with no arguments. The program will repeatedly prompt 
the user to enter commands, using `$:`, and will attempt to execute the 
arguments once the user presses enter. Once executed, the program will 
prompt the user for the next argument list. To exit the program, use the 
command `exit`. Specific commands are executable:

#### Acceptable Commands

Commands that are executables located the `PATH` are accepted commands. The only
built-in command for this program is `exit` which exits the program. 

#### Accepted Connectors

This shell allows multiple commands to be entered in the prompt at one time using the connectors `;`, `||`, and `&&`. If provided, the next command after a `;` connector is always executed. As in Bash, the `||` conditional connector executes the next command after the connector only if the command before the connector fails. Also, the `&&` conditional connector executes the next command after the connector only if the command before the connector passes. Unlike Bash, the `&&` and `||` connectors are ignored if no command argument is provided after the connector. Also, if no command argument is provided before a `||` connector, the command after the connector will execute, since no command was successfully executed before the connector. Similarly, if no command argument is provided before a `&&` connector, the command after the connector will not execute, since no command was successfully executed before the connector. This varies from Bash, which considers these cases as illegal syntax.

example I/O for the special cases of the conditional connectors:

`$: ls ||`
`hello.cpp goodbye.c`
`$: asdf &&`
`execution failed: No such file or directory`
`$: && pwd`
`$: || touch heyo.c`
`$: ls &&`
`hello.cpp heyo.c goodbye.c`


#### Comments

Just as in Bash, any input after a `#` character is considered a 
comment as is not taken as a command argument.

## Installation

Commands to download and install rshell:

`git clone https://github.com/emullally/rshell.git`
`cd rsehll`
`git checkout hw0`
`make`
`bin/rshell`

## Known Bugs

This list of known bugs is a work in progress:
