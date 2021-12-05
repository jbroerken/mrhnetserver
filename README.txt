#########################
#                       #
#  mrhnetserver ReadMe  #
#                       #
#########################

##
# About
##

The mrhnetserver project provides both the MRH connection and communication network servers. 
It is used to allow for communication between an app client (iOS, Android, etc.) and the MRH platform 
services.


##
# Requirements
##

Compilation:
------------
This project is built using CMake. You can find CMake here:

https://cmake.org/

Library Dependencies:
---------------------
This project requires other libraries and headers to function:

Dependency List:
libsodium: https://github.com/jedisct1/libsodium/
msquic: https://github.com/microsoft/msquic/
MySQL Connector/C++: https://dev.mysql.com/doc/connector-cpp/8.0/en/

For more information about the requirements, check the "Building" section found in the documentation.


##
# Documentation
##

All documentation is build with sphinx-doc using the Read The Docs theme.
To build the documentation, grab the requirements for it:

sphinx-doc:
-----------
https://www.sphinx-doc.org/en/master/

Read The Docs Theme:
--------------------
https://sphinx-rtd-theme.readthedocs.io/en/stable/

##
# Licence
##

This project is licenced under the GNU General Public 2 licence. 
Please read the included LICENCE.txt for the exact terms.


##
# Directories
##

This project supplies multiple directories for the development of said project. 
Their names and descriptions are as follows:

Directory List:
build: CMake build directory.
config: Server configuration files.
doc: Documentation files.
sql: Server database files.
src: Project source code.