"""Replay original ID lookup values, final state and exact ordered setter traces."""
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
base = pathlib.Path(__file__).resolve().parent
root = base / 'identity-fixtures'
if not (root / 'manifest.json').is_file():
    root = (base / '../../../migration/fixtures/rpc-identities').resolve()
manifest = json.loads((root / 'manifest.json').read_text())
commands, expected = (root / 'commands.jsonl').read_bytes(), (root / 'expected.jsonl').read_bytes()
assert hashlib.sha256(commands).hexdigest() == manifest['commands_sha256']
assert hashlib.sha256(expected).hexdigest() == manifest['expected_sha256']
work = pathlib.Path(tempfile.mkdtemp(prefix='biglybtc-rpc-identities-replay-'))
path = work / 'commands.jsonl'; path.write_bytes(commands); inode = path.stat().st_ino
result = subprocess.run([sys.argv[1], str(path)], check=True, capture_output=True, timeout=60)
(work / 'native.jsonl').write_bytes(result.stdout); (work / 'stderr.txt').write_bytes(result.stderr)
actual, original = result.stdout.splitlines(), expected.splitlines()
assert len(actual) == len(original)
for i, (a, b) in enumerate(zip(actual, original)):
    assert json.loads(a) == json.loads(b), (work, i, json.loads(a), json.loads(b))
assert path.read_bytes() == commands and path.stat().st_ino == inode
print('PASS original RPC identities:', manifest['observations'], 'cases,', manifest['lookups'], 'lookups;', work)
