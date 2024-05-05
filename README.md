# EOS Reward (`eosio.reward`) [![Bun Test](https://github.com/eosnetworkfoundation/eosio.reward/actions/workflows/test.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.reward/actions/workflows/test.yml)

## Overview

The `eosio.reward` contract handles system reward distribution.

## Strategies

The `eosio.reward` contract is designed to distribute a linear amount of rewards to a single or multiple strategies. Each strategy has an associated weight which determines the percentage of rewards that will be distributed to that strategy.

| Strategy      | Description |
| ------------- | ----------- |
| `eosio.rex` | Donate to REX - Distributes rewards to REX pool which is distributed to REX holders by staking for 21 days |
| `eosio.bonds` | Donate to Bonds - Distributes rewards to Bonds pool which is distributed to Bonds holders from 4 weeks up to 1 year |

## Explain the formula

```toml
supply = 2100000000.0000 EOS
annual_rate = 150 => 1.5%
epoch_time_interval = 600 => 10 minutes
year = 86400 * 365 => 1 year
precision = 10000 => 2 decimal points
(2100000000 * 150 * 600) / (86400 * 365) / 10000 = 599.3150 EOS
```

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
