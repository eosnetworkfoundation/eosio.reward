import { Asset, Name } from '@wharfkit/antelope'
import { describe, expect, test } from 'bun:test'
import { Blockchain, expectToThrow } from '@eosnetwork/vert'

const blockchain = new Blockchain()
const rex = 'eosio.rex'
const bonds = 'eosio.bonds'
const saving = 'eosio.saving'
blockchain.createAccounts(rex, bonds, "eosio")

const reward_contract = 'eosio.reward'
const contracts = {
    reward: blockchain.createContract(reward_contract, reward_contract, true),
    token: blockchain.createContract('eosio.token', 'external/eosio.token/eosio.token', true),
    system: blockchain.createContract('eosio', 'external/eosio.system/eosio', true),
    saving: blockchain.createContract(saving, 'external/eosio.saving/eosio.saving', true),
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

function getBalances(){
    const get = (account:string) => ({
        balance: getTokenBalance(account, 'EOS')
    })
    return {
        reward: get(reward_contract),
        rex: get(rex),
        saving: get(saving),
        bonds: get(bonds),
    }
}

describe(reward_contract, () => {
    test('eosio::init', async () => {
        await contracts.system.actions.init([]).send()
    })

    test('eosio::saving', async () => {
        await contracts.saving.actions.setdistrib([[{account: "eosio.reward", percent: 10000}]]).send()
    })

    test('eosio.token::issue::EOS', async () => {
        const supply = `2100000000.0000 EOS`
        await contracts.token.actions.create(['eosio.token', supply]).send()
        await contracts.token.actions.issue(['eosio.token', supply, '']).send()
        await contracts.token.actions.transfer(['eosio.token', "eosio", '350000000.0000 EOS', '']).send();
        await contracts.token.actions.open(['eosio.reward', "4,EOS", "eosio.token"]).send();
    })

    test("eosio.reward::setstrategy", async () => {
        await contracts.reward.actions.setstrategy(['eosio.rex', 100]).send(); // 100%
    });

    test("eosio.reward::distibute - 100% to rex", async () => {
        await contracts.token.actions.transfer(['eosio', "eosio.saving", '1000.0000 EOS', '']).send();
        const before = getBalances();
        await contracts.reward.actions.distribute([]).send();
        const after = getBalances();

        expect(after.rex.balance - before.rex.balance).toBe(10000000)
        expect(after.saving.balance - before.saving.balance).toBe(-10000000)
        expect(after.reward.balance - before.reward.balance).toBe(0)
    });

    test("eosio.reward::distibute - 90/10% to rex/bonds", async () => {
        await contracts.token.actions.transfer(['eosio', "eosio.saving", '1000.0000 EOS', '']).send();
        await contracts.reward.actions.setstrategy(['eosio.bonds', 10]).send(); // 10%
        await contracts.reward.actions.setstrategy(['eosio.rex', 90]).send(); // 90%
        const before = getBalances();
        await contracts.reward.actions.distribute([]).send();
        const after = getBalances();

        expect(after.rex.balance - before.rex.balance).toBe(9000000)
        expect(after.bonds.balance - before.bonds.balance).toBe(1000000)
        expect(after.reward.balance - before.reward.balance).toBe(0)
    });

    test('eosio.reward::distibute::error - no balance to distribute', async () => {
        const action = contracts.reward.actions.distribute([]).send();
        await expectToThrow(action, 'eosio_assert: no balance to distribute')
    })

    test('eosio.reward::delstrategy::error - strategy not found', async () => {
        const action = contracts.reward.actions.delstrategy(["foo"]).send();
        await expectToThrow(action, 'eosio_assert: strategy not found')
    })

    test('eosio.reward::setstrategy::error - strategy not defined', async () => {
        const action = contracts.reward.actions.setstrategy(["foo", 100]).send();
        await expectToThrow(action, 'eosio_assert: strategy not defined')
    })

    test('eosio.reward::setstrategy::error - weight must be greater than 0', async () => {
        const action = contracts.reward.actions.setstrategy(["eosio.rex", 0]).send();
        await expectToThrow(action, 'eosio_assert: weight must be greater than 0')
    })
})