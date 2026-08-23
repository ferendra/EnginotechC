#!/usr/bin/env node
/**
 * ec-build.js — Build the EnginotechC++ compiler from source.
 * Usage:  node tools/ec-build.js [-o outPath] [--quiet]
 */
'use strict';
const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const args = process.argv.slice(2);
const quiet = args.includes('--quiet');
const outIdx = args.indexOf('-o');
const OUT = outIdx >= 0 ? args[outIdx + 1] : path.join(ROOT, 'build', 'engc');

// Auto-discover all C++ sources; mirrors the CMake target's contents.
// Excluded: legacy LLVM-C++ backend + Windows-only platform files.
const EXCLUDE = new Set([
  'compiler/codegen/llvcodegen.cpp',
  'compiler/codegen/asmcodegen.cpp',
  'compiler/platform/platform_posix.cpp',
  'compiler/platform/platform_windows.cpp',
]);

const SOURCES = [];
function* walk(rel) {
  const full = path.join(ROOT, rel);
  if (!fs.existsSync(full)) return;
  for (const e of fs.readdirSync(full, { withFileTypes: true })) {
    const child = `${rel}/${e.name}`;
    if (e.isDirectory()) yield* walk(child);
    else if (child.endsWith('.cpp') && !EXCLUDE.has(child)) yield child;
  }
}
for (const d of ['compiler', 'runtime', 'std', 'formatter', 'linter',
                 'build-system', 'packages']) {
  SOURCES.push(...walk(d));
}
SOURCES.sort();

fs.mkdirSync(path.dirname(OUT), { recursive: true });
if (!quiet) console.log(`[ec-build] compiling ${SOURCES.length} translation units ...`);
const t0 = Date.now();
try {
  const err = execFileSync('g++',
    ['-std=c++20', '-Wall', '-Wextra', '-Icompiler', '-I.', `-o${OUT}`, ...SOURCES],
    { cwd: ROOT, stdio: ['ignore', 'pipe', 'pipe'] }).toString();
  if (err.trim()) process.stderr.write(err);   // warnings
} catch (e) {
  process.stderr.write(e.stderr ? e.stderr.toString() : String(e));
  console.error(`[ec-build] FAILED`);
  process.exit(1);
}
console.log(`[ec-build] OK -> ${path.relative(ROOT, OUT)} (${((Date.now() - t0) / 1000).toFixed(1)}s)`);
