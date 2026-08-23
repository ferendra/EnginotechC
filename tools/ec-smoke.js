#!/usr/bin/env node
/**
 * ec-smoke.js — Run the end-to-end smoke suite (tests/smoke).
 * Compiles+runs each *.ec via `engc run` and diffs stdout vs .expected.
 *
 * Usage:
 *   node tools/ec-build.js && node tools/ec-smoke.js            # build first
 *   node tools/ec-smoke.js --bin /path/to/engc                  # existing bin
 */
'use strict';
const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const SMOKE = path.join(ROOT, 'tests', 'smoke');
const args = process.argv.slice(2);
const binIdx = args.indexOf('--bin');
const BIN = binIdx >= 0 ? args[binIdx + 1] : path.join(ROOT, 'build', 'engc');
const WORK = fs.mkdtempSync('/tmp/engc_smoke_js_');

if (!fs.existsSync(BIN)) {
  console.error(`[ec-smoke] binary not found: ${BIN} (run tools/ec-build.js first)`);
  process.exit(1);
}

// Propagate backend overrides into the child environment.
const childEnv = { ...process.env };
if (process.env.ENGC_LLVM_BIN) {
  childEnv.PATH = `${process.env.ENGC_LLVM_BIN}:${childEnv.PATH || ''}`;
  if (process.env.ENGC_LD_LIBRARY_PATH)
    childEnv.LD_LIBRARY_PATH =
      `${process.env.ENGC_LD_LIBRARY_PATH}:${childEnv.LD_LIBRARY_PATH || ''}`;
}

let pass = 0, fail = 0, skip = 0;
for (const ec of fs.readdirSync(SMOKE).filter((f) => f.endsWith('.ec')).sort()) {
  const name = path.basename(ec, '.ec');
  const file = path.join(SMOKE, ec);

  // negative tests must FAIL to compile
  if (/_neg_|^neg_/.test(name)) {
    let failed = false;
    try {
      execFileSync(BIN, ['run', file, path.join(WORK, 'neg')],
                   { stdio: 'ignore', env: childEnv });
    } catch (_) { failed = true; }
    if (failed) { console.log(`\x1b[32mPASS\x1b[0m ${name}`); pass++; }
    else { console.log(`\x1b[31mFAIL\x1b[0m ${name} — expected compile failure`); fail++; }
    continue;
  }

  const expectedFile = path.join(SMOKE, `${name}.expected`);
  if (!fs.existsSync(expectedFile)) {
    console.log(`\x1b[33mSKIP\x1b[0m ${name} — no .expected`); skip++; continue;
  }
  const stdinFile = path.join(SMOKE, `${name}.stdin`);

  let out;
  try {
    out = execFileSync(
      BIN,
      ['run', file, path.join(WORK, name)],
      {
        input: fs.existsSync(stdinFile) ? fs.readFileSync(stdinFile) : '',
        encoding: 'utf8',
        timeout: 30000,
        env: childEnv,
      });
  } catch (e) {
    console.log(`\x1b[31mFAIL\x1b[0m ${name} — engc exit ${e.status}`);
    fail++;
    continue;
  }
  const filtered = out
    .split('\n')
    .filter((l) => !/^(Compiled: |Ran: )/.test(l))
    .join('\n')
    .replace(/\s+$/, '');   // driver's last line may lack a trailing newline
  const expected = fs.readFileSync(expectedFile, 'utf8').replace(/\s+$/, '');
  if (filtered === expected) {
    console.log(`\x1b[32mPASS\x1b[0m ${name}`); pass++;
  } else {
    console.log(`\x1b[31mFAIL\x1b[0m ${name} — output mismatch:`);
    for (const d of diff(expected.split('\n'), filtered.split('\n'))) console.log(`      ${d}`);
    fail++;
  }
}

fs.rmSync(WORK, { recursive: true, force: true });
console.log(`\nResults: ${pass} passed, ${fail} failed, ${skip} skipped`);
process.exit(fail ? 1 : 0);

// tiny line diff for readable failures
function diff(a, b) {
  const lines = [];
  const n = Math.max(a.length, b.length);
  for (let i = 0; i < n; i++) {
    if (a[i] !== b[i]) {
      if (a[i] !== undefined) lines.push(`- expected: "${a[i]}"`);
      if (b[i] !== undefined) lines.push(`+ actual:   "${b[i]}"`);
    }
  }
  return lines.slice(0, 12);
}
