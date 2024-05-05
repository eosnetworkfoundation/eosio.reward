#pragma once

#include <eosio/eosio.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
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
         * ## TABLE `settings`
         *
         * - `{uint32} epoch_time_interval` - epoch time interval in seconds (time between epoch distribution events)
         * - `{time_point_sec} next_epoch_timestamp` - next epoch timestamp event to trigger strategy distribution
         *
         * ### example
         *
         * ```json
         * {
         *   "epoch_time_interval": 600,
         *   "next_epoch_timestamp": "2024-04-07T00:00:00"
         * }
         * ```
         */
        struct [[eosio::table("settings")]] settings_row {
            uint32_t            epoch_time_interval = 600; // 10 minutes
            time_point_sec      next_epoch_timestamp;
            int64_t             annual_rate = 150; // 1.5% annual rate
        };
        typedef eosio::singleton< "settings"_n, settings_row > settings_table;

        /**
         * Initialize the contract with the epoch period.
         *
         * @param epoch_period - epoch period in seconds
         * @param annual_rate - Annual rate of the core token supply.
         *     (eg. For 5% Annual rate => annual_rate=500
         *          For 1.5% Annual rate => annual_rate=150
         */
        [[eosio::action]]
        void init( const uint32_t epoch_period, const int64_t annual_rate );

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
         * @param strategies - strategy names to delete
         */
        [[eosio::action]]
        void delstrategy( const vector<name> strategies );

        /**
         * Distribute rewards to all defined strategies.
         */
        [[eosio::action]]
        void distribute();

        // ACTION WRAPPERS
        using distribute_action = eosio::action_wrapper<"distribute"_n, &reward::distribute>;
        using setstrategy_action = eosio::action_wrapper<"setstrategy"_n, &reward::setstrategy>;
        using delstrategy_action = eosio::action_wrapper<"delstrategy"_n, &reward::delstrategy>;
        using init_action = eosio::action_wrapper<"init"_n, &reward::init>;

        /**
         * Get the total weight of all strategies.
         *
         * @param contract - contract name
         * @return uint16_t - total weight
         */
        static uint16_t get_total_weight( const name contract = "eosio.reward"_n ) {
            strategies_table _strategies( contract, contract.value );
            uint16_t total_weight = 0;
            for (auto& row : _strategies) {
                total_weight += row.weight;
            }
            return total_weight;
        }

    private:
        void update_next_epoch();
        asset calculate_amount_to_distribute();
    };
} /// namespace eosio
