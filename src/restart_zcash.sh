#!/bin/bash
# zcash config and data directory are found relative to avahome

# if an argument is provided to this script, that argument becomes $avahome
# if no argument is provided, this script attempts to use the env var, $AVAHOME as $avahome
# if both the above are not set, $avahome defaults to ~/.avalanchhego

# if 2 arguments are provided, the second argument is the zcash port, otherwise port is set to 18250

avahome="${1:-${AVAHOME:-$HOME/.avalanchego}}"
echo "(Re)starting zcashd with avahome set to $avahome" 

confdir="$avahome/configs/zcash/"
datadir="$avahome/db/zcash/"

mkdir -p $confdir
mkdir -p $datadir

port="${2:-18250}"
conf="$avahome/configs/zcash/fuji.conf"
datadir="$avahome/db/zcash/"
logfile="$avahome/logs/zcash.log"

if [ ! -f "$conf" ]; then
  echo "No configuration file exists, creating default config file"
  echo "regtest=1 # Run in the regtest environment
gen=0 # Disable mining
txindex=1 # Maintain a full transaction index
genproclimit=1 # Use 1 thread for coin generation
printtoconsole=1 # Log out to console instead of a debug.log file
rpcport=8232 # define port to expose rpc on
rpcuser=test # for dev, change when going to prod
rpcpassword=pw # for dev, change when going to prod" >> $conf   
fi

existing_proc=$(ps aux | grep $conf | grep -v grep | awk '{print $2}')

if [ ! -z "$existing_proc" ]; then
  kill $existing_proc
  echo "Killed existing zcash process"
fi

find $avahome/db/zcash/regtest -not -name wallet.dat -delete

echo "Launching zcash with port=$port, conf=$conf, and datadir=$datadir"

if [ ! -f "zcashd" ]; then
  if [ "$(uname)" == "Darwin" ]; then
    echo "Detected we are on OSX and we have not built from source. Copying zcashd_osx to zcashd."
    cp zcashd_osx zcashd
  elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    echo "Detected we are on OSX and we have not built from source. Copying zcashd_osx to zcashd."
    cp zcashd_linux zcashd
  else
    echo "Zcashd only supported on Linux and OSX"
    exit 1
  fi
fi

./zcashd -conf=$conf -datadir=$datadir -port=$port -reindex >> $logfile &

zcash_pid=$!

echo "Successfully launched with pid $zcash_pid"
