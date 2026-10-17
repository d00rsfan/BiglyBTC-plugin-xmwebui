// SPDX-License-Identifier: GPL-2.0-or-later
#include "rpc_identities.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
using namespace biglybtc::plugins::rpc;
void require(bool value) { if (!value) { std::abort(); } }
int main() {
  auto ledger = IdentityLedger::restore({1, 1, 0}, 999, true);
  require(ledger.has_value());
  const auto preview = ledger->preview(2, true);
  require(preview.has_value() && preview->value() == 3 && preview->writes().size() == 4);
  require(ledger->ids()[1] == 1 && ledger->ids()[2] == 0 && ledger->next() == 999 && ledger->needsInitialCheck());
  const auto otherPreview = ledger->preview(0, false);
  auto foreign = IdentityLedger::restore({1, 1, 0}, 999, true);
  require(foreign->apply(*preview).error() == IdentityError::StalePlan);
  require(ledger->apply(*preview).has_value());
  require(ledger->ids()[1] == 2 && ledger->ids()[2] == 3 && ledger->next() == 4 && !ledger->needsInitialCheck());
  require(ledger->apply(*preview).error() == IdentityError::StalePlan);
  require(ledger->apply(*otherPreview).error() == IdentityError::StalePlan);
  require(ledger->preview(3, true).error() == IdentityError::UnknownTorrent);
  require(ledger->next() == 4);
  const auto movePlan = ledger->preview(0, false);
  auto moved = std::move(*ledger);
  require(ledger->preview(0, true).error() == IdentityError::NoLedger);
  require(moved.apply(*movePlan).has_value());
  auto defaults = IdentityLedger::restore({0}, std::nullopt, false);
  require(defaults->preview(0, true)->value() == 1);
  require(!IdentityLedger::restore(std::vector<std::int64_t>(IdentityLedger::maximumTorrents + 1), 1));
  auto maximum = IdentityLedger::restore(std::vector<std::int64_t>(IdentityLedger::maximumTorrents, 1), 999);
  const auto plan = maximum->preview(IdentityLedger::maximumTorrents - 1, true);
  require(plan.has_value() && plan->writes().size() == IdentityLedger::maximumTorrents);
  require(plan->value() == static_cast<std::int64_t>(IdentityLedger::maximumTorrents));
  require(maximum->next() == 999 && maximum->ids().back() == 1);
  require(maximum->apply(*plan).has_value());
  require(maximum->next() == static_cast<std::int64_t>(IdentityLedger::maximumTorrents + 1));
  require(std::is_sorted(maximum->ids().begin(), maximum->ids().end()));
  std::cout << "PASS RPC identity preview/no mutation, owner/generation validation, moves and100000-torrent bounds\n";
}
