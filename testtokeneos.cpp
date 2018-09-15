//
//    Created: 2018/09/16 2:33 AM
//     Author: Eugene V. Palchukovsky
//     E-mail: eugene@palchukovsky.com
//

#include "testtokeneos.hpp"
#include <eosiolib/dispatcher.hpp>
#include <boost/make_unique.hpp>

using namespace eosio;

EOSIO_ABI(TestTokenEos, (create)(issue)(activate)(transfer)(transferfrz)(show))

void TestTokenEos::create(asset asset,
                          const std::string &title,
                          const account_name &admin) {
  eosio_assert(asset.is_valid(), "Asset is invalid.");
  eosio_assert(asset.amount > 0, "Amount must be positive.");

  require_auth(_self);

  auto symbolSet = GetSymbols();
  eosio_assert(symbolSet.find(asset.symbol.name()) == symbolSet.end(),
               "Token symbol is not unique.");
  symbolSet.emplace(_self, [&asset, &title, &admin](auto &newRecord) {
    newRecord.asset.symbol = std::move(asset.symbol);
    newRecord.volume = asset.amount;
    newRecord.title = title;
    newRecord.admin = admin;
  });
}

void TestTokenEos::issue(const account_name &target,
                         const asset &activeAssetRequest,
                         const asset &frozenAssetRequest) {
  eosio_assert(activeAssetRequest.is_valid(), "Asset is invalid.");
  eosio_assert(activeAssetRequest.amount >= 0,
               "Amount must be positive or zero.");

  eosio_assert(frozenAssetRequest.is_valid(), "Asset is invalid.");
  eosio_assert(frozenAssetRequest.amount >= 0,
               "Amount must be positive or zero.");

  eosio_assert(
      activeAssetRequest.symbol.name() == frozenAssetRequest.symbol.name(),
      "Active and frozen assets have different symbols.");

  require_auth(_self);

  auto symbol = GetSymbol(activeAssetRequest.symbol);

  const auto newSymbolAmount = symbol.second->asset.amount +
                               activeAssetRequest.amount +
                               frozenAssetRequest.amount;
  eosio_assert(symbol.second->volume >= newSymbolAmount,
               "Requested volume exceeds the available volume.");

  {
    auto account = GetAccount(symbol.second->asset.symbol, target);
    account.first->modify(
        account.second, _self,
        [&activeAssetRequest, &frozenAssetRequest](auto &account) {
          account.activeBalance += activeAssetRequest.amount;
          account.frozenBalance += frozenAssetRequest.amount;
        });
  }

  symbol.first->modify(symbol.second, _self, [newSymbolAmount](auto &record) {
    record.asset.amount = newSymbolAmount;
  });
}

void TestTokenEos::activate(const account_name &owner, const asset &asset) {
  eosio_assert(asset.is_valid(), "Asset is invalid.");
  eosio_assert(asset.amount > 0, "Amount must be positive.");

  const auto &symbol = GetSymbol(asset.symbol);
  require_auth(symbol.second->admin);

  auto account = GetAccount(asset.symbol, owner);
  const auto amount = std::min(asset.amount, account.second->frozenBalance);
  account.first->modify(account.second, _self, [amount](auto &record) {
    record.activeBalance += amount;
    record.frozenBalance -= amount;
  });
}

void TestTokenEos::transfer(const account_name &source,
                            const account_name &target,
                            const asset &asset) {
  eosio_assert(asset.is_valid(), "Asset is invalid.");
  eosio_assert(asset.amount > 0, "Amount must be positive.");

  eosio_assert(source != target, "Target and source are the same accounts.");
  require_auth(source);

  // Checks symbol existent:
  GetSymbol(asset.symbol);

  auto sourceAccount = GetAccount(asset.symbol, source);
  eosio_assert(sourceAccount.second->activeBalance >= asset.amount,
               "Insufficient funds for transaction.");

  auto targetAccount = GetAccount(asset.symbol, target);

  sourceAccount.first->modify(
      sourceAccount.second, _self,
      [&asset](Account &account) { account.activeBalance -= asset.amount; });
  targetAccount.first->modify(
      targetAccount.second, _self,
      [&asset](Account &account) { account.activeBalance += asset.amount; });
}

void TestTokenEos::transferfrz(const account_name &target, const asset &asset) {
  eosio_assert(asset.is_valid(), "Asset is invalid.");
  eosio_assert(asset.amount > 0, "Amount must be positive.");

  const auto &symbol = GetSymbol(asset.symbol);
  eosio_assert(symbol.second->admin != target,
               "Target and source are the same accounts.");
  require_auth(symbol.second->admin);

  auto sourceAccount = GetAccount(asset.symbol, symbol.second->admin);
  eosio_assert(sourceAccount.second->frozenBalance >= asset.amount,
               "Insufficient funds for transaction.");

  auto targetAccount = GetAccount(asset.symbol, target);

  sourceAccount.first->modify(
      sourceAccount.second, _self,
      [&asset](Account &account) { account.frozenBalance -= asset.amount; });
  targetAccount.first->modify(
      targetAccount.second, _self,
      [&asset](Account &account) { account.frozenBalance += asset.amount; });
}

void TestTokenEos::show(const account_name &owner) {
  require_auth(_self);
  for (const auto &symbol : GetSymbols()) {
    const auto &account = FindAccount(symbol.asset.symbol, owner);
    if (!account) {
      print_f(R"("%": % has nothing; )", symbol.title.c_str(), name{owner});
      continue;
    }
    print_f(R"("%": % has % active and % frozen; )", symbol.title.c_str(),
            name{owner}, account->activeBalance, account->frozenBalance);
  }
}

TestTokenEos::Symbols TestTokenEos::GetSymbols() { return {_self, _self}; }

TestTokenEos::Accounts TestTokenEos::GetAccounts(const symbol_type &symbol) {
  return {_self, symbol.name()};
}

std::pair<std::unique_ptr<TestTokenEos::Symbols>,
          TestTokenEos::Symbols::const_iterator>
TestTokenEos::GetSymbol(const symbol_type &symbol) {
  auto index = boost::make_unique<Symbols>(GetSymbols());
  auto it = index->find(symbol.name());
  eosio_assert(it != index->cend(), "Symbol is unknown.");
  return {std::move(index), std::move(it)};
}

std::pair<std::unique_ptr<TestTokenEos::Accounts>,
          TestTokenEos::Accounts::const_iterator>
TestTokenEos::GetAccount(const symbol_type &symbol, const account_name &owner) {
  auto index = boost::make_unique<Accounts>(GetAccounts(symbol));
  auto it = index->find(owner);
  if (it == index->cend()) {
    it = index->emplace(_self,
                        [&owner](auto &account) { account.name = owner; });
  }
  return {std::move(index), std::move(it)};
}

boost::optional<TestTokenEos::Account> TestTokenEos::FindAccount(
    const symbol_type &symbol, const account_name &owner) {
  const auto &index = GetAccounts(symbol);
  auto it = index.find(owner);
  if (it == index.cend()) {
    return boost::none;
  }
  return *it;
}
