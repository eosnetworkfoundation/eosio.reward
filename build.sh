#!/bin/bash

cdt-cpp eosio.reward.cpp -I ./include -I ./external
wasm2wat eosio.reward.wasm | sed -e 's|(memory |(memory (export "memory") |' > eosio.reward.wat
wat2wasm -o eosio.reward.wasm eosio.reward.wat
rm eosio.reward.wat
