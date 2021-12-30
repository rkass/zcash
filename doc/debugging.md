## Mac OS X Debugging

The following document contains instructions for debugging `zcashd` on OSX. The instructions below were what worked for my machine which is a Mid-2013 MacBook Air running OS `10.14.5` with 4 GB RAM.

### 1. Build zcash

- Follow instructions from zcash docs for building zcash from source: https://zcash.readthedocs.io/en/latest/rtd_pages/macOS-build.html
- Instead of `./zcutil/build.sh -j$(sysctl -n hw.ncpu)`, I need to run  `./zcutil/build.sh -j2` as building with a concurrency level of 4 causes deadlocks on my machine.
- Specify the following environment variable `CONFIGURE_FLAGS='CXXFLAGS=-O0'`
- My full command looks like: `CONFIGURE_FLAGS='CXXFLAGS=-O0' ./zcutil/build.sh -j2`
- Note that when you update cpp code, you don't have to run this command again. You can simply run `make` to compile your code.

### 2. Create a Configuration

- By default, Zcash looks for my configuration in `"$HOME/Library/Application\ Support/Zcash/zcash.conf"`. Let's make a new directory and explicitly state where zcash should get its conf so we can easily toggle between different configurations for different node numbers. 

```
mkdir $HOME/Library/Application\ Support/Zcash0/
```

Then, create a file called `zcash.conf` in the above directory.

regtest=1 # Run in the regtest environment
gen=0 # Disable mining
txindex=1 # Maintain a full transaction index
genproclimit=1 # Use 1 thread for coin generation
printtoconsole=1 # Log out to console instead of a debug.log file
rpcport=8232 # define port to expose rpc on
rpcuser=test # for dev, change when going to prod
rpcpassword=pw # for dev, change when going to prod
```

### 3. Run zcashd without debugging

Verify that you can run zcashd without debugging.

- From terminal: `DISABLE_POW=true HOME=/Users/rkass ./src/zcashd`
- Once your program has booted up, fire an rpc call to ensure the program started successfully:
```
=>./src/zcash-cli getblockcount
2
```

### 4. Download Visual Studio

https://code.visualstudio.com/download

### 5. Download the C++ VS Code Extension

Extension id: ms-vscode.cpptools

### 6. Create a launch.json

This doc teaches you how to attach to an already running zcashd launched from the command line. An earlier version of this doc specified how to launch zcashd from vs code itself. I've found the new workflow of launching on command line and attaching separately to be more reliable.

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
            "program": "/Users/rkass/repos/z-sno/zcash/src/zcashd_debug",
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

### Debug

- Ensure that zcashd is running on your machine via the earlier steps. Get the process id for this run (run `ps aux | grep zcashd`). 
- Click `Run and Debug` along the left hand side of your VS code window and you'll be prompted for the process id you want to attach to. Input the process id you found in the previous step. 
- To verify, once your program is running, put a breakpoint in an RPC handler, for example try adding a breakpoint to `getblockcount` in `src/rpc/blockchain.cpp` and invoking the same rpc method as we did above: `./src/zcash-cli getblockcount`. Verify that your breakpoint is hit.