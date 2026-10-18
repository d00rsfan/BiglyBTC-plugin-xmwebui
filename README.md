# BiglyBTC-plugin-xmwebui

Bounded original RPC/session contract with Qt 6.12 Core and C++23. Original
baseline: `c9ab8f7cd62b2874f7385772992c0288a224e85d`. This static host library
does not listen on HTTP, read a profile or implement download/session methods.
The caller supplies a real dispatcher and maps host errors to the original
structured failure contract. Fixture echo/error dispatchers are test code only.

Transmission arguments/result/tag and JSON-RPC 2 params/result/id, null/default
handling, underscore method names, permission propagation and original textual/
general error shapes are retained. JSON values are compared semantically;
field order/escape style is not a persisted-file format. Numeric identifiers in
the signed 64-bit range, nested values and Unicode are characterized against
exact original method bodies with a supplied host dispatcher and frozen JSON.

Session checks retain the original /transmission/ path predicate, header and
cookie precedence, exact cookie names, ASCII trim and path exemptions. The host
owns expected session IDs. Raw incoming x-vuze-is-tunnel claims never establish
trust here; an authenticated internal tunnel adapter must supply explicit trust
metadata. This session/CSRF check does not replace HTTP authentication or method
authorization. The reference host applies that same incoming-header boundary.

Bounds: 10,000 request value nodes, 32 levels, 1 Mi UTF-16 units per string/key;
128 headers, 64 Ki header values and 16 Ki paths. Rejections occur before dispatch.
No operational server or full remote-control parity is claimed. HTTP framing/
authentication, host methods/view permissions, pairing/tunnels/search and web
assets remain required. GPL-2.0-or-later; original Copyright 2009 Vuze, Inc.


The identity ledger separately models original RPC torrent IDs. Preview returns
ordered duplicate-repair/allocation write intentions without modifying its state;
apply accepts only a plan for that ledger and generation. It preserves negative
IDs, zero-only allocation, signed-long wrap and the original first-lookup counter
reset. Inputs must include the ordered active and archived stub snapshot. The
model holds at most 100,000 torrents and uses a bounded ordered set for duplicate
checking. It opens no files. Durable attribute/counter binding and a cross-file
journal must precede real host activation; preview/apply alone is not that journal.

## Continuous integration

The workflow is [`.github/workflows/compatibility.yml`](.github/workflows/compatibility.yml). [View runs](https://github.com/d00rsfan/BiglyBTC-plugin-xmwebui/actions).

Start it manually from **Actions → Run workflow** (or `gh workflow run compatibility.yml -R d00rsfan/BiglyBTC-plugin-xmwebui`). It requires a separately provisioned Linux x64 runner with labels `self-hosted`, `Linux`, `X64`, and `biglybtc-linux`, plus the pinned toolchain and dependencies used by the recipe. Registering the workflow does not provision that runner; a run stays queued until a matching runner is online. It does not run on pull requests.

For future workflow changes, push using this repository's SSH URL (`git@github.com:d00rsfan/BiglyBTC-plugin-xmwebui.git`). HTTPS pushes authenticated with an OAuth token require the additional `workflow` scope to add or update workflow files. Viewing public Actions runs does not require that scope.
