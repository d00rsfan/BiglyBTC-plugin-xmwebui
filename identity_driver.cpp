// SPDX-License-Identifier: GPL-2.0-or-later
#include "rpc_identities.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace biglybtc::plugins::rpc;
int main(int argc, char** argv) {
  if (argc != 2) { return 2; }
  const auto path = std::filesystem::canonical(argv[1]);
  if (!path.native().starts_with("/tmp/biglybtc-") || std::filesystem::file_size(path) > 8 * 1024 * 1024) { return 2; }
  std::ifstream stream(path);
  if (!stream.good()) { return 3; }
  std::string line;
  while (std::getline(stream, line)) {
    QJsonParseError error;
    const auto input = QJsonDocument::fromJson(QByteArray(line.data(), static_cast<qsizetype>(line.size())), &error);
    if (error.error != QJsonParseError::NoError || !input.isObject()) { return 4; }
    const auto object = input.object();
    std::vector<std::int64_t> ids;
    for (const auto& value : object.value(QStringLiteral("ids")).toArray()) { ids.push_back(value.toInteger()); }
    auto ledger = IdentityLedger::restore(std::move(ids), object.value(QStringLiteral("next")).toInteger(),
        object.value(QStringLiteral("check")).toBool());
    if (!ledger) { return 5; }
    QJsonArray results, writes;
    for (const auto& item : object.value(QStringLiteral("requests")).toArray()) {
      const auto request = item.toArray();
      const auto before = std::vector(ledger->ids().begin(), ledger->ids().end());
      const auto counter = ledger->next(); const auto checked = ledger->needsInitialCheck();
      auto plan = ledger->preview(static_cast<std::size_t>(request[0].toInteger()), request[1].toBool());
      if (!plan || !std::equal(before.begin(), before.end(), ledger->ids().begin())
          || ledger->next() != counter || ledger->needsInitialCheck() != checked) { return 6; }
      results.append(static_cast<qint64>(plan->value()));
      for (const auto& write : plan->writes()) {
        if (write.kind == IdentityWrite::Kind::Counter) { writes.append(QJsonArray{QStringLiteral("counter"), static_cast<qint64>(write.value)}); }
        else { writes.append(QJsonArray{QStringLiteral("id"), static_cast<qint64>(write.index), static_cast<qint64>(write.value)}); }
      }
      if (!ledger->apply(*plan)) { return 7; }
    }
    QJsonArray finalIds;
    for (const auto id : ledger->ids()) { finalIds.append(static_cast<qint64>(id)); }
    QJsonObject observation{{QStringLiteral("results"), results}, {QStringLiteral("ids"), finalIds},
        {QStringLiteral("next"), static_cast<qint64>(ledger->next())}, {QStringLiteral("check"), ledger->needsInitialCheck()},
        {QStringLiteral("writes"), writes}};
    const auto output = QJsonDocument(observation).toJson(QJsonDocument::Compact);
    std::cout.write(output.constData(), output.size()); std::cout << '\n';
  }
}
