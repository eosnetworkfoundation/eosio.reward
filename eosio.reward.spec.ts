import { Asset, Int64, Name } from '@wharfkit/antelope'
import { describe, expect, test } from 'bun:test'
import { Blockchain, expectToThrow } from '@eosnetwork/vert'
import { TimePointSec } from "@greymass/eosio";

// Vert EOS VM
const blockchain = new Blockchain()
const rex = 'eosio.rex'
const bob = 'bob'
blockchain.createAccounts(rex, bob)

const reward_contract = 'eosio.reward'
const contracts = {
    reward: blockchain.createContract(reward_contract, reward_contract, true),
    token: blockchain.createContract('eosio.token', 'external/eosio.token/eosio.token', true),
    system: blockchain.createContract('eosio', 'external/eosio.system/eosio', true),
    fake: {
        token: blockchain.createContract('fake.token', 'external/eosio.token/eosio.token', true),
    },
}

function getTokenBalance(account: string, symcode: string) {
    const scope = Name.from(account).value.value
    const primary_key = Asset.SymbolCode.from(symcode).value.value
    const row = contracts.token.tables
        .accounts(scope)
        .getTableRow(primary_key)
    if (!row) return 0;
    return Asset.from(row.balance).units.toNumber()
}

function getStrategies() {
    const scope = Name.from(reward_contract).value.value
    const row = contracts.reward.tables
        .strategies(scope)
        .getTableRows()
    return row;
}

function getRamBytes(account: string) {
    const scope = Name.from(account).value.value
    const row = contracts.system.tables
        .userres(scope)
        .getTableRow(scope)
    if (!row) return 0
    return Int64.from(row.ram_bytes).toNumber()
}

const TEN_MINUTES = 600;

function incrementTime(seconds = TEN_MINUTES) {
    const time = TimePointSec.fromInteger(seconds);
    return blockchain.addTime(time);
}

describe(reward_contract, () => {
    test('eosio::init', async () => {
        await contracts.system.actions.init([]).send()
    })

    test('eosio.token::issue::EOS', async () => {
        const supply = `2100000000.0000 EOS`
        await contracts.token.actions.create(['eosio.token', supply]).send()
        await contracts.token.actions.issue(['eosio.token', supply, '']).send()
        await contracts.token.actions.transfer(['eosio.token', reward_contract, '350000000.0000 EOS', '']).send();
    })

    test('eosio.reward::init', async () => {
        await contracts.reward.actions.init([TEN_MINUTES, 150]).send() // 1.5% annual rate of active supply
    })

    test("eosio.reward::setstrategy", async () => {
        await contracts.reward.actions.setstrategy(['eosio.rex', 10000]).send(); // 100%
    });

    test("eosio.reward::distibute", async () => {
        incrementTime();
        const before = {
            reward: {
                balance: getTokenBalance(reward_contract, 'EOS'),
            },
            rex: {
                balance: getTokenBalance(rex, 'EOS'),
            },
        }
        await contracts.reward.actions.distribute([]).send();
        const after = {
            reward: {
                balance: getTokenBalance(reward_contract, 'EOS'),
            },
            rex: {
                balance: getTokenBalance(rex, 'EOS'),
            },
        }

        // EOS
        expect(after.rex.balance - before.rex.balance).toBe(5993150)
        expect(after.reward.balance - before.reward.balance).toBe(-5993150)
    });

    test('eosio.reward::distibute::error - epoch not finished', async () => {
        const action = contracts.reward.actions.distribute([]).send();
        await expectToThrow(action, 'eosio_assert: epoch not finished')
    })

    test("eosio.reward::distibute - after 10 minutes & user authority", async () => {
        incrementTime();
        await contracts.reward.actions.distribute([]).send(bob); // any user is authorized to call distribute
    });
})