# Zapa Changes

This doc lists changes made to Zcash in order to accomodate the goals of Zapa.

## Api Additions

Many endpoints have been added and modified to support the Zapa runtime environment. See [API](https://github.com/zapalabs/zcash/blob/master/doc/api.md) for a list of API changes.

## Disable POW

Proof of Work has been disabled. This means that when producing blocks the node doesn't solve the Proof of Work challenge, and when validating blocks the node doesn't validate the equihash solution.

This is configurable and decided via the `ShouldNotSolvePOW` and `ShouldNotVerifyPOW` which read their own configuration as well as the configuration for `ShouldDisablePOW`. If `ShouldDisablePOW` is true then `ShoudNotSolvePOW` and `ShouldNotVerifyPOW` both short circuit to true as well. 

Proof of Work has been disabled because the consensus mechanism used to determine which node produces the next block is decided by Snowman consensus within Avalanche.


