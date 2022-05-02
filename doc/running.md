# Running zcashd

Included with this project are two pre-built binaries, [zcashd_osx](https://github.com/zapalabs/zcash/blob/master/src/zcashd_osx) and [zcashd_linux](https://github.com/zapalabs/zcash/blob/master/src/zcashd_linux).
You can either use the binary that suits your machine or build from source. Regardless of whether you build from source or use the pre-built binary, you must download the 
cryptographic parameters for Zcash first by running `./zcutil/fetch-params.sh`. 

When Zcash is run as part of the Zapa project, its data directory (with the exception of wallet.dat) needs to be cleared out before startup. Zcash learns about the initial blockchain state via
the boostrapping mechanism of Avalanche, and thus does not rely on blockchain state written to files. 

### 1. Build zcashd (optional)

- Follow instructions from zcash docs for building zcash from source: https://zcash.readthedocs.io/en/latest/rtd_pages/macOS-build.html
- For building on my machine which is a Mid-2013 MacBook Air running OS `10.14.5` with 4 GB RAM I needed to take the following additional measures:
    - Instead of `./zcutil/build.sh -j$(sysctl -n hw.ncpu)`, I need to run  `./zcutil/build.sh -j2` as building with a concurrency level of 4 causes deadlocks on my machine.
    - Specify the following environment variable `CONFIGURE_FLAGS='CXXFLAGS=-O0'`
    - My full command looks like: `CONFIGURE_FLAGS='CXXFLAGS=-O0' ./zcutil/build.sh -j2`
- Note that when you update cpp code, you don't have to run this command again. You can simply run `make` to compile your code.

### 2. Create a Configuration

- By default (on OSX), Zcash looks for configuration in `"$HOME/Library/Application\ Support/Zcash/zcash.conf"`. Let's make a new directory and explicitly state where zcash should get its conf so we can easily toggle between different configurations for different node numbers. Note that it doesn't matter where the directory is located.

```
mkdir $HOME/Library/Application\ Support/Zcash0/
```

Then, create a file called `zcash.conf` in the above directory.

```
regtest=1 # Run in the regtest environment
gen=0 # Disable mining
txindex=1 # Maintain a full transaction index
genproclimit=1 # Use 1 thread for coin generation
printtoconsole=1 # Log out to console instead of a debug.log file
rpcport=8232 # define port to expose rpc on. we typically use 8234 --> 8238 for testing and 8232 for production nodes. 
# The rpcport value isentirely up to the developer/validator and there are no requirements around these values.
rpcuser=test # for dev, change when going to prod
rpcpassword=pw # for dev, change when going to prod
```

### 3. Run zcashd

- From terminal: `./src/zcashd`
- Once your program has booted up, fire an rpc call to ensure the program started successfully:
```
=>./src/zcash-cli getblockcount
2
```

## Running zcashd: As Part of your Avalanche Validator

To run Zcash in a production or testnet environment, navigate to the `/src` directory and run:

```
./restart_zcash.sh
```

This script:
- Terminates an existing zcashd if it's running.
- Establishes your configuration, log, and data directories relative to your avalanche home directory ($AVAHOME or `~/.avalanchego` if not specified). To override this value, 
provide an alternate directory as an argument to the script. e.g. `./restart_zcash.sh ~/mybasedir/`
    - Creates a default confguratioon file with the above defaults at `$AVAHOME/configs/zcash/fuji.conf`.
    - Creates a data directory at `$AVAHOME/db/zcash`.
    - Creates and writes to a log file at `$AVAHOME/logs/zcash.log`
- Clears out the data directory
- Launches Zcash as a background process

## Running zcashd: As Part of a 5 Node Local Setup

This section contains directions for spinning up five zcashd processes which each validate a local blockchain. Each of these processes are meant to communicate
with a zapavm plugin launched via ava-sim.

### 1. Create 5 different configuration files. 
It's not important where they live, but we'll refer to them later. For example:
```
touch $HOME/zcash_conf/regtest0.conf
touch $HOME/zcash_conf/regtest1.conf
touch $HOME/zcash_conf/regtest2.conf
touch $HOME/zcash_conf/regtest3.conf
touch $HOME/zcash_conf/regtest4.conf
```
Each file should get the configuration contents listed above, with one exception: node 0 should have `rpcport=8234`, node 1 configuration should have `rpcport=8235`, node 2 configuration should have `rpcport=8236`, node 3 configuration should have `rpcport=8237`, and node 4 configuration should have `rpcport=8238`.

### 2. Create 5 different data directories. 
These will be empty directories which get cleared out between each run. It's not important where they live, but we'll refer to them later. For example:
```
mkdir -p $HOME/zcash_data/regtest0/
mkdir -p $HOME/zcash_data/regtest1/
mkdir -p $HOME/zcash_data/regtest2/
mkdir -p $HOME/zcash_data/regtest3/
mkdir -p $HOME/zcash_data/regtest4/
```
### 3. Run each node in a separate process. 
Open 5 terminal windows, and run one commmand in each window. The below commands assume:

- The data and config directories are as specified above -- change accordingly.
- zcashd has been built from source, as directed in [Build zcashd](#1-build-zcashd-optional). If it has not been built from source, use the pre-built binary for your OS, either `zcashd_linux` or `zcashd_osx`.
- We are in the `/src/` directory.

#### Node 0
```
rm -rf $HOME/zcash_data/regtest0/ && mkdir $HOME/zcash_data/regtest0/ && ./zcashd -conf=$HOME/zcash_conf/regtest0.conf -datadir=$HOME/zcash_data/regtest0/ -port="18252"
```

#### Node 1
```
rm -rf $HOME/zcash_data/regtest1/ && mkdir $HOME/zcash_data/regtest1/ && ./zcashd -conf=$HOME/zcash_conf/regtest1.conf -datadir=$HOME/zcash_data/regtest1/ -port="18253"
```

#### Node 2
```
rm -rf $HOME/zcash_data/regtest2/ && mkdir $HOME/zcash_data/regtest2/ && ./zcashd -conf=$HOME/zcash_conf/regtest2.conf -datadir=$HOME/zcash_data/regtest2/ -port="18254"
```

#### Node 3
```
rm -rf $HOME/zcash_data/regtest3/ && mkdir $HOME/zcash_data/regtest3/ && ./zcashd -conf=$HOME/zcash_conf/regtest3.conf -datadir=$HOME/zcash_data/regtest3/ -port="18255"
```


#### Node 4
```
rm -rf $HOME/zcash_data/regtest4/ && mkdir $HOME/zcash_data/regtest4/ && ./zcashd -conf=$HOME/zcash_conf/regtest4.conf -datadir=$HOME/zcash_data/regtest4/ -port="18256"
```

## Running zcashd: Debugging on a Mac

### 1. Download Visual Studio

https://code.visualstudio.com/download

### 2. Download the C++ VS Code Extension

Extension id: ms-vscode.cpptools

### 3. Create a launch.json

This doc teaches you how to attach to an already running zcashd launched from the command line.

Create a file called `./vscode/launch.json`. See below for what mine looks like.

```
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(lldb) Attach",
            "type": "lldb",
            "request": "attach",
            "program": "/Users/rkass/repos/zapa/zcash/src/zcashd",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceRoot}",
            "externalConsole": true,
            "MIMode": "lldb",
            "processId": "${command:pickProcess}",
            "logging": {"trace": true, "engineLogging": true, "traceResponse": true}
        },
    ]
}
```

### 4. Debug

- Ensure that zcashd is running on your machine via the earlier steps. Get the process id for this run (run `ps aux | grep zcashd`). 
- Click `Run and Debug` along the left hand side of your VS code window and you'll be prompted for the process id you want to attach to. Input the process id you found in the previous step. 
- To verify, once your program is running, put a breakpoint in an RPC handler, for example try adding a breakpoint to `getblockcount` in `src/rpc/blockchain.cpp` and invoking the same rpc method as we did above: `./src/zcash-cli getblockcount`. Verify that your breakpoint is hit.
