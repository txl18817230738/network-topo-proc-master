# Nebula Graph C++ SDK

Nebula Graph C++ SDK provides a C++ interface for Nebula Graph.
This SDK enables developers of Nebula Graph to write customizable plugins.
Plugin is the main interface to define plugable component of Nebula Graph Database kernel.

## How to Build

### Step1: Install cpp sdk

```shell
$ ./nebula-graph-cpp-sdk-5.0-x86_64-glibc-2.35.sh
Installing nebula-graph into ./nebula-graph-cpp-sdk-5.0-x86_64-glibc-2.35...
Done

$ tree nebula-graph-cpp-sdk-5.0-x86_64-glibc-2.35 -L 1
nebula-graph-cpp-sdk-5.0-x86_64-glibc-2.35
├── cmake
├── CMakeLists.txt
├── include
├── install-third-party.sh
├── lib
├── README.md
└── src
```

### Step2: Install nebula third-party

```shell
$ cd nebula-graph-cpp-sdk-5.0-x86_64-glibc-2.35
$ ./install-third-party.sh --version=5.1 [--prefix=/path/to/nebula/third-party]
```

### Step3: Build shared library

```shell
$ mkdir build && cd build
$ cmake -DNEBULA_THIRDPARTY_ROOT=/path/to/nebula/third-party
$ make
[ 33%] Building CXX object CMakeFiles/computing-algo.dir/src/computing/algorithm/traversal/BFS.cpp.o
[ 66%] Building CXX object CMakeFiles/computing-algo.dir/src/computing/algorithm/ranking/PageRank.cpp.o
[100%] Linking CXX shared library lib/computing-algo.so
[100%] Built target computing-algo

$ ls lib/
lib/computing-algo.so
```

## How to Deploy shared library

Following guide provides detailed instructions on how to deploy a shared library
to the Nebula installation directory. Here's how to do it:

### Step 1: Place the Compiled Shared Library in the Plugins Directory

First, you need to place the compiled shared library in the `plugins` directory
of the Nebula installation directory. For instance, if the library is named
`computing-algo.so`, and your Nebula is installed in the `/usr/local/nebula`
directory, you would place the shared library in the following location:

```
/usr/local/nebula/plugins/computing-algo.so
```

### Step 2: Configure the Nebula Configuration

Next, you need to modify the `etc/nebula-graphd.conf` file in the Nebula
installation directory. Specifically, you need to append the `computing-algo.so`
to the `--plugins` configuration flag. The updated configuration item should
look similar to this:

```
--plugins=dbms.so,computing-algo.so
```

### Step 3: Restart the Nebula Service Process

After configuring the Nebula configuration file, you need to restart the Nebula
service process. You can do this by running the following command:

```shell
$ scripts/nebula.service restart all
```

### Step 4: Verify the Shared Library Load

Finally, you can verify that the shared library has been successfully loaded by
running the following GQL statement:

```
(root@nebula) [(none)]> SHOW PLUGINS
+-----------------+-------------------+--------------+--------------+--------------------------------------------+---------------+---------+-------------+
| module          | name              | type         | author       | description                                | license       | version | api_version |
+-----------------+-------------------+--------------+--------------+--------------------------------------------+---------------+---------+-------------+
| "file_audit.so" | "FileAuditPlugin" | "AUDIT"      | "vesoft-inc" | "nebula's default file based audit plugin" | "PROPRIETARY" | "5.0"   | "5.0"       |
| "logrotate.so"  | "LogRotatePlugin" | "LOG_ROTATE" | "vesoft-inc" | "nebula's log rotator"                     | "PROPRIETARY" | "5.0"   | "5.0"       |
| "dbms.so"       | "dbms"            | "PROCEDURE"  | "vesoft-inc" | "nebula's default dbms procedures"         | "PROPRIETARY" | "5.0"   | "5.0"       |
| "algo.so"       | "algo"            | "PROCEDURE"  | "vesoft-inc" | "nebula's default algorithm set"           | "PROPRIETARY" | "5.0"   | "5.0"       |
+-----------------+-------------------+--------------+--------------+--------------------------------------------+---------------+---------+-------------+
Got 4 rows (time spent 0s/4.064929ms)

Mon, 29 Jan 2024 11:46:12 CST
```

If the shared library has been loaded successfully, it should appear in the list
of plugins displayed by the `SHOW PLUGINS` command.


## Plugin development

For more details, please refer to the document: `include/nebula/plugins/README.md`.
