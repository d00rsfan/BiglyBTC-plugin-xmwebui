// SPDX-License-Identifier: GPL-2.0-or-later
#include "rpc_contract.hpp"
#include <QJsonArray>
#include <cstdlib>
#include <iostream>
using namespace biglybtc::plugins::rpc;
void require(bool value) { if (!value) { std::abort(); } }
int main() {
  int calls = 0;
  Dispatcher dispatcher = [&calls](const Call&) -> DispatchResult {
    ++calls;
    return std::optional<QJsonObject>{QJsonObject{}};
  };
  QJsonObject request{{QStringLiteral("method"), QStringLiteral("echo")}};
  auto deep = QJsonValue(0);
  for (unsigned i = 0; i < 40; ++i) { deep = QJsonArray{deep}; }
  request.insert(QStringLiteral("tag"), deep);
  require(process(request, dispatcher).error() == OuterError::Limit && calls == 0);
  QJsonArray wide;
  for (int i = 0; i < 10001; ++i) { wide.append(i); }
  request.insert(QStringLiteral("tag"), wide);
  require(process(request, dispatcher).error() == OuterError::Limit && calls == 0);
  request.insert(QStringLiteral("tag"), QString(1024 * 1024 + 1, 'x'));
  require(process(request, dispatcher).error() == OuterError::Limit && calls == 0);
  request.remove(QStringLiteral("tag"));
  require(process(request, {}).error() == OuterError::MissingDispatcher && calls == 0);
  request.insert(QStringLiteral("arguments"), QJsonArray{1});
  require(process(request, dispatcher).error() == OuterError::WrongType && calls == 0);
  Headers headers{{QStringLiteral("x-vuze-is-tunnel"), QStringLiteral("true")}};
  require(sessionValid(QStringLiteral("/transmission/rpc"), headers, true, QStringLiteral("s")) == false);
  require(sessionValid(QStringLiteral("/transmission/rpc"), headers, true, QStringLiteral("s"), true) == true);
  headers.insert(QStringLiteral("cookie"), QString(65537, 'x'));
  require(sessionValid(QStringLiteral("/transmission/rpc"), headers, true, QStringLiteral("s")).error() == OuterError::Limit);
  require(sessionValid(QString(16385, 'x'), {}, true, QStringLiteral("s")).error() == OuterError::Limit);
  require(sessionValid(QStringLiteral("/rpc"), {}, false, QString{}).error() == OuterError::Limit);
  std::cout << "PASS bounded RPC inputs, no dispatch on rejected data and explicit trusted tunnel boundary\n";
}
