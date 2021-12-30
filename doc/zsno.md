This doc contains notes on changes to the codebase made in order to support zsno.

## DISABLE_POW flag

When this environment variable is set to true (`DISABLE_POW=true`), zcash does not use
any proof of work when generating transactions, and when validating blocks, zcash does not
validate proof of work or the equihash solution. Setting this flag allows zcash to rely on Avalanche's consensus engine for selecting blocks.

## Change to rpc: z_sendmany

This procedure call now returns the serialized transaction. Since all networking is handled by the Avalanche client, it's necessary to return the transaction to the client, so that the client can notify the other nodes in the network of this transaction.

## New rpc: receivetx

Accepts a serialized transaction which gets added to the node's memory pool. The input to this procedure call matches the output of the `z_sendmany` rpc.

## New rpc: suggest

Suggests a new block. This differs from the existing procedure call `generate`, in that this block is only a suggestion of what the next block in the chain should be. Whereas `generate` adds the new block to the active chain, this rpc returns the block and waits further instruction before adding it to the active chain.

## New rpc: validateBlock

Validates all of the transactions in the input block. Returns a null response if the block is valid, and otherwise returns a non-null response with the reason(s) the block is invalid.

## New rpc: submitblock

Tells this node that the input block is part of the active chain.
