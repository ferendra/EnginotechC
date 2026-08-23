#!/usr/bin/env node
/**
 * ec-doctor.js — Environment checker untuk pengembangan EnginotechC++.
 * Usage:  node tools/ec-doctor.js
 */
'use strict';
const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const ROOT = path.resolve(__dirname, '..');

const ok = (m) => console.log(`  \x1b[32mOK\x1b[0m    ${m}`);
const bad = (m) => console.log(`  \x1b[31mMISS\x1b[0m  ${m}`);
const info = (m) => console.log(`  \x1b[33m--\x1b[0m    ${m}`);

function has(cmd) {
  try { execFileSync(cmd, ['--version'], { stdio: 'ignore' }); return true; }
  catch { return false; }
}

console.log('EnginotechC++ — Doctor\n');

process.stdout.write('Node.js            ');
console.log(process.version >= 'v18.' ? `\x1b[32m${process.version}\x1b[0m (cukup)` : `\x1b[31m${process.version}\x1b[0m (butuh >=18)`);

process.stdout.write('g++ (C++20)        ');
console.log(has('g++') ? '\x1b[32madalah\x1b[0m' : '\x1b[31mtidak ada\x1b[0m');

const backends = [];
if (has('clang')) backends.push('clang');
if (has('llvm-as') && has('llc') && (has('cc') || has('gcc')))
  backends.push('llvm-as+llc+cc');
if (backends.length) ok(`backend native: ${backends.join(', ')}`);
else bad('backend native (butuh clang ATAU llvm-as+llc+cc di PATH)');
if (process.env.ENGC_LLVM_BIN) info(`ENGC_LLVM_BIN=${process.env.ENGC_LLVM_BIN} akan dipakai child process`);

const bin = path.join(ROOT, 'build', 'engc');
if (fs.existsSync(bin)) ok(`compiler ter-build: ${path.relative(ROOT, bin)}`);
else info('compiler belum di-build -> node tools/ec-build.js');

for (const d of ['tests/smoke', 'examples', 'gui', 'tools']) {
  const p = path.join(ROOT, d);
  fs.existsSync(p) ? ok(`folder ${d}/`) : bad(`folder ${d}/ hilang`);
}
console.log('\nSiap! Alur cepat:  node tools/ec-build.js && node tools/ec-smoke.js');
