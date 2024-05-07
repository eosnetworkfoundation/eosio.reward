# EOS Reward (`eosio.reward`) [![Bun Test](https://github.com/eosnetworkfoundation/eosio.reward/actions/workflows/test.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.reward/actions/workflows/test.yml)

## Overview

The `eosio.reward` contract handles system reward distribution.

```mermaid
graph TD
    eosio --> |unallocated bucket| eosio.saving
    eosio.saving --> |distribution claim| eosio.reward
    eosio.saving -.-> |distribution claim| eosio.grants
    eosio.saving -.-> |distribution claim| eoslabs.io
    eosio.reward --> weights{% weights}
    weights -.-> |% Donate To REX| eosio.rex
```

## Strategies

The `eosio.reward` contract is designed to distribute a linear amount of rewards to a single or multiple strategies. Each strategy has an associated weight which determines the percentage of rewards that will be distributed to that strategy.

| Strategy      | Description |
| ------------- | ----------- |
| `eosio.rex` | Donate to REX - Distributes rewards to REX pool which is distributed to REX holders by staking for 21 days |


## Development and Testing

### Build Instructions

To compile the contract, developers can use the following command:

```sh
cdt-cpp eosio.reward.cpp -I ./include
```

### Testing Framework

The contract includes a comprehensive testing suite designed to validate its functionality. The tests are executed using the following commands:

```sh
$ npm test

> test
> bun test
```
