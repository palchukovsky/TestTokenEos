//
//    Created: 2018/09/16 2:33 AM
//     Author: Eugene V. Palchukovsky
//     E-mail: eugene@palchukovsky.com
//

#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/multi_index.hpp>
#include <boost/optional.hpp>
#include <string>

class TestTokenEos : public eosio::contract {
  //! Account state for some one symbols.
  struct Account {
    account_name name = 0;
    int64_t activeBalance = 0;
    int64_t frozenBalance = 0;

    uint64_t primary_key() const { return name; }
  };

  struct Symbol {
    eosio::asset asset;
    int64_t volume = 0;
    std::string title;
    account_name admin;

    uint64_t primary_key() const { return asset.symbol.name(); }
  };

  typedef eosio::multi_index<N(accounts), Account> Accounts;
  typedef eosio::multi_index<N(symbols), Symbol> Symbols;

 public:
  using contract::contract;
  TestTokenEos(TestTokenEos &&) = default;
  TestTokenEos(const TestTokenEos &) = delete;
  TestTokenEos &operator=(TestTokenEos &&) = delete;
  TestTokenEos &operator=(const TestTokenEos &) = delete;
  ~TestTokenEos() = default;

  //! Creates new symbols, sets maximum available issued volume for it, and
  //! assigns administrator for this symbols.
  void create(eosio::asset asset,
              const std::string &title,
              const account_name &admin);

  //! Issues tokens for administrator.
  /**
   * @param[in] owner Asset owner.
   * @param[in] active Active part of asset.
   * @param[in] frozen Frozen part if asset. Only administrator can transfer it,
   * until administrator will not activate it.
   */
  void issue(const account_name &owner,
             const eosio::asset &active,
             const eosio::asset &frozen);

  //! Activates specified part of frozen asset for specified user.
  void activate(const account_name &owner, const eosio::asset &asset);

  //! Transfers active part of asset.
  /**
   * Operation allowed only for administrator asset.
   * @sa transferfrz
   */

  void transfer(const account_name &source,
                const account_name &target,
                const eosio::asset &asset);
  //! Transfers frozen part of asset from an administrator account.
  /**
   * Operation allowed only for administrator asset.
   * @sa activate
   * @sa transfer
   */
  void transferfrz(const account_name &target, const eosio::asset &asset);

  //! Prints debug information about user assets.
  void show(const account_name &user);

 private:
  Symbols GetSymbols();
  Accounts GetAccounts(const eosio::symbol_type &);

  std::pair<std::unique_ptr<Symbols>, Symbols::const_iterator> GetSymbol(
      const eosio::symbol_type &);

  std::pair<std::unique_ptr<Accounts>, Accounts::const_iterator> GetAccount(
      const eosio::symbol_type &, const account_name &);
  boost::optional<Account> FindAccount(const eosio::symbol_type &,
                                       const account_name &);
};
