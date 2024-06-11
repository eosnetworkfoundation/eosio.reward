#pragma once

#include <eosio/eosio.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio.saving/eosio.saving.hpp>
#include <eosio/singleton.hpp>

using namespace std;

namespace eosio {

    const set<name> STRATEGIES = {
        "eosio.rex"_n,
        "eosio.bonds"_n,
    };

    /**
     * The `eosio.reward` contract handles system reward distribution.
     */
    class [[eosio::contract("eosio.reward")]] reward : public contract {
    public:
        using contract::contract;

        /**
         * ## TABLE `strategies`
         *
         * - `{name} strategy` - strategy name
         * - `{uint16_t} weight` - strategy weight (proportional to the total weight of all strategies)
         *
         * ### example
         *
         * ```json
         * [
         *   {
         *     "strategy": "eosio.rex",
         *     "weight": 100
         *   },
         *   {
         *     "strategy": "eosio.bonds",
         *     "weight": 10
         *   }
         * ]
         * ```
         */
        struct [[eosio::table("strategies")]] strategies_row {
            name                strategy;
            uint16_t            weight;

            uint64_t primary_key() const { return strategy.value; }
        };
        typedef eosio::multi_index< "strategies"_n, strategies_row > strategies_table;

        /**
         * Set a strategy with a weight.
         *
         * @param strategy - strategy name
         * @param weight - strategy weight
         */
        [[eosio::action]]
        void setstrategy( const name strategy, const uint16_t weight );

        /**
         * Delete a strategy.
         *
         * @param strategy - strategy name to delete
         */
        [[eosio::action]]
        void delstrategy( const name strategy );

        /**
         * Distribute rewards to all defined strategies.
         */
        [[eosio::action]]
        void distribute();

        // ACTION WRAPPERS
        using distribute_action = eosio::action_wrapper<"distribute"_n, &reward::distribute>;
        using setstrategy_action = eosio::action_wrapper<"setstrategy"_n, &reward::setstrategy>;
        using delstrategy_action = eosio::action_wrapper<"delstrategy"_n, &reward::delstrategy>;

        /**
         * Get the total weight of all strategies.
         *
         * @param contract - contract name
         * @return uint16_t - total weight
         */
        static uint32_t get_total_weight( const name contract = "eosio.reward"_n ) {
            strategies_table _strategies( contract, contract.value );
            uint32_t total_weight = 0;
            for (auto& row : _strategies) {
                total_weight += row.weight;
            }
            return total_weight;
        }
    };
} /// namespace eosio
