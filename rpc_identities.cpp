// SPDX-License-Identifier: GPL-2.0-or-later
// Original XMWebUIPlugin.getID at c9ab8f7cd62b2874f7385772992c0288a224e85d.
// Copyright2009 Vuze, Inc.; supplied ordered registered/archived torrent snapshot.
#include "rpc_identities.hpp"
#include <algorithm>
#include <bit>
#include <set>
namespace biglybtc::plugins::rpc {
namespace {
std::int64_t increment(std::int64_t value) {
  return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(value) + 1U);
}
}
std::expected<IdentityLedger, IdentityError> IdentityLedger::restore(std::vector<std::int64_t> ids,
    std::optional<std::int64_t> next, bool initialCheck) {
  if (ids.size() > maximumTorrents) { return std::unexpected(IdentityError::Limit); }
  return IdentityLedger(std::move(ids), next.value_or(1), initialCheck);
}
std::expected<IdentityPlan, IdentityError> IdentityLedger::preview(std::size_t index, bool allocate) const {
  if (!owner_) { return std::unexpected(IdentityError::NoLedger); }
  if (index >= ids_.size()) { return std::unexpected(IdentityError::UnknownTorrent); }
  IdentityPlan plan;
  plan.owner_ = owner_; plan.revision_ = revision_; plan.initialCheck_ = initialCheck_;
  auto next = next_;
  auto value = ids_[index];
  if (initialCheck_) {
    std::set<std::int64_t> seen;
    std::vector<std::size_t> duplicates;
    std::int64_t maximum = 0;
    for (std::size_t i = 0; i < ids_.size(); ++i) {
      if (ids_[i] <= 0) { continue; }
      maximum = std::max(maximum, ids_[i]);
      if (!seen.insert(ids_[i]).second) { duplicates.push_back(i); }
    }
    next = increment(maximum);
    for (const auto duplicate : duplicates) {
      plan.writes_.push_back({IdentityWrite::Kind::Torrent, duplicate, next});
      if (duplicate == index) { value = next; }
      next = increment(next);
    }
    plan.writes_.push_back({IdentityWrite::Kind::Counter, 0, next});
  }
  if (value == 0 && allocate) {
    value = next;
    next = increment(next);
    plan.writes_.push_back({IdentityWrite::Kind::Counter, 0, next});
    plan.writes_.push_back({IdentityWrite::Kind::Torrent, index, value});
  }
  plan.value_ = value;
  return plan;
}
std::expected<void, IdentityError> IdentityLedger::apply(const IdentityPlan& plan) {
  if (!owner_ || plan.owner_.lock() != owner_ || plan.revision_ != revision_) { return std::unexpected(IdentityError::StalePlan); }
  for (const auto& write : plan.writes_) {
    if (write.kind == IdentityWrite::Kind::Counter) { next_ = write.value; }
    else { ids_[write.index] = write.value; }
  }
  if (plan.initialCheck_) { initialCheck_ = false; }
  ++revision_;
  return {};
}
}
