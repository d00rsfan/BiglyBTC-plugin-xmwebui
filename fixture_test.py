"""Compare JSON semantics of exact original RPC wrapper/session observations."""
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
base = pathlib.Path(__file__).resolve().parent
root = base / 'fixtures'
if not (root / 'manifest.json').is_file():
    root = (base / '../../../migration/fixtures/rpc-contract').resolve()
manifest = json.loads((root / 'manifest.json').read_text())
commands, expected = (root / 'commands.jsonl').read_bytes(), (root / 'expected.jsonl').read_bytes()
assert hashlib.sha256(commands).hexdigest() == manifest['commands_sha256']
assert hashlib.sha256(expected).hexdigest() == manifest['expected_sha256']
work = pathlib.Path(tempfile.mkdtemp(prefix='biglybtc-rpc-contract-replay-'))
path = work / 'commands.jsonl'; path.write_bytes(commands); inode = path.stat().st_ino
result = subprocess.run([sys.argv[1], str(path)], check=True, capture_output=True, timeout=60)
(work / 'native.jsonl').write_bytes(result.stdout); (work / 'stderr.txt').write_bytes(result.stderr)
actual, original = result.stdout.splitlines(), expected.splitlines()
assert len(actual) == len(original)
def equivalent(a, b):
    # JSON number formatting is not significant; booleans remain distinct from
    # integers, and Python compares large integer values without float coercion.
    if isinstance(a, bool) or isinstance(b, bool): return type(a) is type(b) and a == b
    if isinstance(a, dict) or isinstance(b, dict):
        return isinstance(a, dict) and isinstance(b, dict) and a.keys() == b.keys() and all(equivalent(a[k], b[k]) for k in a)
    if isinstance(a, list) or isinstance(b, list):
        return isinstance(a, list) and isinstance(b, list) and len(a) == len(b) and all(equivalent(x, y) for x, y in zip(a, b))
    return a == b
for i, (a, b) in enumerate(zip(actual, original)):
    assert equivalent(json.loads(a), json.loads(b)), (work, i, json.loads(a), json.loads(b))
assert path.read_bytes() == commands and path.stat().st_ino == inode
print('PASS original RPC contract:', manifest['observations'], 'observations;', work)
