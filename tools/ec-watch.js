#!/usr/bin/env node
/**
 * ec-watch.js — Mode watcher: jalankan ulang smoke suite saat ada perubahan.
 * Usage:  node tools/ec-watch.js [--src compiler|gui|all] [--bin path]
 */
'use strict';
const { execFileSync, spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const args = new Set(process.argv.slice(2));
const scope = [...args].find((a) => a.startsWith('--src'))?.split('=')[1] || 'compiler';
const binIdx = process.argv.indexOf('--bin');
const BIN = binIdx >= 0 ? process.argv[binIdx + 1] : path.join(ROOT, 'build', 'engc');

let building = false, rerun = false;
function build() {
  if (building) { rerun = true; return; }
  building = true;
  console.log(`\x1b[36m[watch]\x1b[0m rebuild (${scope}) ...`);
  let r;
  try {
    r = spawnSync('node', [path.join(__dirname, 'ec-build.js')], { stdio: 'inherit' });
  } finally { building = false; }
  if (r && r.status === 0) runSmoke();
  if (rerun) { rerun = false; build(); }
}

function runSmoke() {
  console.log('\x1b[36m[watch]\x1b[0m smoke ...');
  try {
    const out = execFileSync('node', [path.join(__dirname, 'ec-smoke.js'), '--bin', BIN],
      { encoding: 'utf8', timeout: 300000 });
    console.log(out.trim().split('\n').pop());
  } catch (e) {
    console.log((e.stdout || '').trim().split('\n')
      .filter((l) => /FAIL/.test(l)).slice(0, 8).join('\n'));
  }
}

const roots = scope === 'all'
  ? ['compiler', 'std', 'runtime', 'gui']
  : [scope];
for (const root of roots) {
  fs.watch(path.join(ROOT, root), { recursive: true }, (_ev, f) => {
    if (f && /\.(cpp|h|c)$/.test(f)) {
      clearTimeout(build._t);
      build._t = setTimeout(build, 400);
    }
  });
}
console.log(`\x1b[36m[watch]\x1b[0m memantau ${roots.join(', ')} — Ctrl+C untuk berhenti`);
runSmoke();
