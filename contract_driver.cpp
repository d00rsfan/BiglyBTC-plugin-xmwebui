// SPDX-License-Identifier: GPL-2.0-or-later
#include "rpc_contract.hpp"
#include <QJsonDocument>
#include <QJsonParseError>
#include <filesystem>
#include <fstream>
#include <iostream>
namespace rpc = biglybtc::plugins::rpc;
rpc::DispatchResult fixtureDispatch(const rpc::Call& call) {
  using Kind = rpc::Failure::Kind;
  const auto& method = call.method;
  if (method == QStringLiteral("permission")) { return std::unexpected(rpc::Failure{Kind::Permission, {}, {}}); }
  if (method == QStringLiteral("textual")) { return std::unexpected(rpc::Failure{Kind::Textual, QStringLiteral("bad torrent"), {}}); }
  if (method == QStringLiteral("textual-null")) { return std::unexpected(rpc::Failure{Kind::Textual, {}, {}}); }
  if (method == QStringLiteral("textual-cause")) {
    return std::unexpected(rpc::Failure{Kind::Textual, QStringLiteral("bad torrent"),
        {{QStringLiteral("java.io.IOException"), QStringLiteral("disk\nline")}}});
  }
  if (method == QStringLiteral("general")) {
    return std::unexpected(rpc::Failure{Kind::General, {},
        {{QStringLiteral("java.lang.IllegalArgumentException"), QStringLiteral("bad argument")}}});
  }
  if (method == QStringLiteral("general-nested")) {
    return std::unexpected(rpc::Failure{Kind::General, {},
        {{QStringLiteral("java.io.IOException"), QStringLiteral("outer")},
         {QStringLiteral("java.lang.IllegalStateException"), QStringLiteral("inner")}}});
  }
  if (method == QStringLiteral("null-result")) { return std::optional<QJsonObject>{}; }
  QJsonObject result;
  if (method == QStringLiteral("echo")) {
    result = {{QStringLiteral("args"), call.arguments}, {QStringLiteral("method"), method},
        {QStringLiteral("jrpc2"), call.jsonrpc2}, {QStringLiteral("session"), QStringLiteral("fixture-session")}};
  }
  return std::optional<QJsonObject>{result};
}
int main(int argc, char** argv) {
  if (argc != 2) { return 2; }
  const auto path = std::filesystem::canonical(argv[1]);
  if (!path.native().starts_with("/tmp/biglybtc-") || std::filesystem::file_size(path) > 8 * 1024 * 1024) { return 2; }
  std::ifstream stream(path);
  if (!stream.good()) { return 3; }
  std::string line;
  while (std::getline(stream, line)) {
    if (line.size() > 1024 * 1024) { return 4; }
    QJsonParseError error;
    const auto input = QJsonDocument::fromJson(QByteArray(line.data(), static_cast<qsizetype>(line.size())), &error);
    if (error.error != QJsonParseError::NoError || !input.isObject()) { return 5; }
    const auto object = input.object();
    const auto before = input.toJson(QJsonDocument::Compact);
    QJsonObject observation;
    if (object.value(QStringLiteral("mode")) == QStringLiteral("session")) {
      rpc::Headers headers;
      const auto values = object.value(QStringLiteral("headers")).toObject();
      for (auto it = values.begin(); it != values.end(); ++it) { headers.insert(it.key(), it.value().toString()); }
      const auto valid = rpc::sessionValid(object.value(QStringLiteral("path")).toString(), headers,
          object.value(QStringLiteral("cookie")).toBool(), QStringLiteral("fixture-session"),
          object.value(QStringLiteral("trusted")).toBool());
      if (!valid) { observation.insert(QStringLiteral("status"), QStringLiteral("limit")); }
      else { observation.insert(QStringLiteral("valid"), *valid); }
    } else {
      int calls = 0;
      std::optional<QJsonObject> request;
      const auto value = object.value(QStringLiteral("request"));
      if (!value.isNull() && !value.isUndefined()) { request = value.toObject(); }
      const auto response = rpc::process(request, [&calls](const rpc::Call& call) {
        ++calls;
        return fixtureDispatch(call);
      });
      if (response) { observation.insert(QStringLiteral("status"), QStringLiteral("ok")); observation.insert(QStringLiteral("response"), *response); }
      else if (response.error() == rpc::OuterError::Permission) { observation.insert(QStringLiteral("status"), QStringLiteral("permission")); }
      else if (response.error() == rpc::OuterError::WrongType) { observation.insert(QStringLiteral("status"), QStringLiteral("cast")); }
      else if (response.error() == rpc::OuterError::MissingMethod) {
        observation.insert(QStringLiteral("status"), QStringLiteral("io")); observation.insert(QStringLiteral("message"), QStringLiteral("'method' missing"));
      } else { observation.insert(QStringLiteral("status"), QStringLiteral("limit")); }
      observation.insert(QStringLiteral("calls"), calls);
    }
    if (input.toJson(QJsonDocument::Compact) != before) { return 6; }
    const auto output = QJsonDocument(observation).toJson(QJsonDocument::Compact);
    std::cout.write(output.constData(), output.size()); std::cout << '\n';
  }
}
