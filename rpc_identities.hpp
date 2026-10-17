// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <vector>
namespace biglybtc::plugins::rpc {
enum class IdentityError { Limit, UnknownTorrent, StalePlan, NoLedger };
struct IdentityWrite {
  enum class Kind { Torrent, Counter } kind;
  std::size_t index;
  std::int64_t value;
};
class IdentityLedger;
class IdentityPlan final {
public:
  std::int64_t value() const { return value_; }
  std::span<const IdentityWrite> writes() const { return writes_; }
  bool initialCheck() const { return initialCheck_; }
private:
  friend class IdentityLedger;
  struct Owner {};
  IdentityPlan() = default;
  std::weak_ptr<const Owner> owner_;
  std::uint64_t revision_ = 0;
  std::int64_t value_ = 0;
  bool initialCheck_ = false;
  std::vector<IdentityWrite> writes_;
};
// Owner-thread in-memory model. Preview never edits a file or the ledger. The
// future host must durably stage the original attribute/counter writes before
// applying the plan; this class is not a cross-file persistence transaction.
class IdentityLedger final {
public:
  static constexpr std::size_t maximumTorrents = 100000;
  static std::expected<IdentityLedger, IdentityError> restore(std::vector<std::int64_t> ids,
      std::optional<std::int64_t> next, bool initialCheck = true);
  IdentityLedger(const IdentityLedger&) = delete;
  IdentityLedger& operator=(const IdentityLedger&) = delete;
  IdentityLedger(IdentityLedger&&) noexcept = default;
  IdentityLedger& operator=(IdentityLedger&&) noexcept = default;
  std::expected<IdentityPlan, IdentityError> preview(std::size_t index, bool allocate) const;
  std::expected<void, IdentityError> apply(const IdentityPlan& plan);
  std::span<const std::int64_t> ids() const { return ids_; }
  std::int64_t next() const { return next_; }
  bool needsInitialCheck() const { return initialCheck_; }
private:
  IdentityLedger(std::vector<std::int64_t> ids, std::int64_t next, bool check)
      : ids_(std::move(ids)), next_(next), initialCheck_(check) {}
  std::vector<std::int64_t> ids_;
  std::int64_t next_;
  bool initialCheck_;
  std::shared_ptr<const IdentityPlan::Owner> owner_ = std::make_shared<const IdentityPlan::Owner>();
  std::uint64_t revision_ = 0;
};
}
