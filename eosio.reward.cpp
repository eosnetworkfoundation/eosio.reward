#include "eosio.reward.hpp"

namespace eosio {

[[eosio::action]]
void reward::setstrategy( const name strategy, const uint16_t weight ) {
    require_auth( get_self() );

    strategies_table _strategies( get_self(), get_self().value );

    // validate input
    check(weight > 0, "weight must be greater than 0");
    check(STRATEGIES.find(strategy) != STRATEGIES.end(), "strategy not defined");

    // update weights
    auto itr = _strategies.find(strategy.value);
    if (itr == _strategies.end()) {
        _strategies.emplace(get_self(), [&](auto& row) {
            row.strategy = strategy;
            row.weight = weight;
        });
    } else {
        _strategies.modify(itr, get_self(), [&](auto& row) {
            row.weight = weight;
        });
    }
}

[[eosio::action]]
void reward::delstrategy( const name strategy )
{
    require_auth( get_self() );

    strategies_table _strategies( get_self(), get_self().value );
    auto itr = _strategies.find(strategy.value);
    check(itr != _strategies.end(), "strategy not found");
    _strategies.erase(itr);
}

[[eosio::action]]
void reward::distribute()
{
    // any authority is allowed to call this action

    strategies_table _strategies( get_self(), get_self().value );
    const uint32_t total_weight = get_total_weight();

    // dispatch actions
    eosiosystem::system_contract::donatetorex_action donatetorex( "eosio"_n, { get_self(), "active"_n });
    eosio::token::transfer_action transfer( "eosio.token"_n, { get_self(), "active"_n });
    saving::claim_action claim( "eosio.saving"_n, { get_self(), "active"_n });

    // claim available rewards from eosio.saving
    const asset saving_balance = saving::get_balance( get_self() );
    if ( saving_balance.amount ) claim.send( get_self() );

    // distributing rewards in EOS
    const asset balance = eosio::token::get_balance( "eosio.token"_n, get_self(), symbol_code("EOS") ) + saving_balance;
    check(balance.amount > 0, "no balance to distribute");

    for ( auto& row : _strategies ) {
        const asset reward_to_distribute = balance * row.weight / total_weight;
        if (reward_to_distribute.amount <= 0) continue; // skip if no fee to distribute

        // Donate to REX
        if ( row.strategy == "eosio.rex"_n) {
            donatetorex.send( get_self(), reward_to_distribute, "staking rewards" );

        // EOS T-Bonds
        } else if ( row.strategy == "eosio.bonds"_n) {
            transfer.send( get_self(), "eosio.bonds"_n, reward_to_distribute, "staking rewards" );
        }
    }
}

} /// namespace eosio
