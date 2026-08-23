#!/usr/bin/env node
/**
 * ec-analyze.js — Quick project stats for the EnginotechC++ repo.
 * Usage:  node tools/ec-analyze.js [--todo] [--ec]
 *   --todo   scan sources for TODO/FIXME/HACK markers
 *   --ec     summarize .ec example programs instead of C++ sources
 */
'use strict';
const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const args = new Set(process.argv.slice(2));

function* walk(dir) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) yield* walk(p);
    else yield p;
  }
}

if (args.has('--ec')) {
  let files = 0, loc = 0;
  const fns = [];
  for (const root of [path.join(ROOT, 'examples'), path.join(ROOT, 'tests')]) {
    if (!fs.existsSync(root)) continue;
    for (const f of walk(root)) {
      if (!f.endsWith('.ec')) continue;
      const src = fs.readFileSync(f, 'utf8');
      if (/^#include/m.test(src)) continue;          // mislabeled C++ files
      files++; loc += src.split('\n').length;
      for (const m of src.matchAll(/^\s*(?:pub\s+)?fn\s+([A-Za-z_]\w*)/gm)) {
        fns.push(`${path.relative(ROOT, f)}: ${m[1]}()`);
      }
    }
  }
  console.log(`EC examples/tests : ${files} files, ${loc} lines`);
  console.log(`functions defined : ${fns.length}`);
} else {
  const dirs = ['compiler', 'runtime', 'std', 'gui', 'formatter', 'linter',
                'build-system', 'packages'];
  const markers = /\b(TODO|FIXME|HACK|XXX)\b/;
  const rows = [];
  let totalLoc = 0, totalTodo = 0;
  for (const d of dirs) {
    const full = path.join(ROOT, d);
    if (!fs.existsSync(full)) continue;
    let loc = 0, todo = 0, nfiles = 0;
    for (const f of walk(full)) {
      if (!/\.(cpp|h|c)$/.test(f)) continue;
      nfiles++;
      const src = fs.readFileSync(f, 'utf8');
      loc += src.split('\n').length;
      todo += (src.match(markers) || []).length;
      if (args.has('--todo') && todo > totalTodo) { /* listed below */ }
    }
    rows.push({ d, nfiles, loc, todo });
    totalLoc += loc; totalTodo += todo;
  }
  console.log('module            files    lines    TODO/FIXME');
  for (const r of rows)
    console.log(`${r.d.padEnd(17)} ${String(r.nfiles).padStart(5)}  ${String(r.loc).padStart(7)}  ${String(r.todo).padStart(6)}`);
  console.log(`${'TOTAL'.padEnd(17)} ${rows.reduce((a, r) => a + r.nfiles, 0)}  ${totalLoc}  ${totalTodo}`);

  if (args.has('--todo')) {
    console.log('\nMarkers:');
    for (const d of dirs) {
      const full = path.join(ROOT, d);
      if (!fs.existsSync(full)) continue;
      for (const f of walk(full)) {
        if (!/\.(cpp|h|c)$/.test(f)) continue;
        fs.readFileSync(f, 'utf8').split('\n').forEach((line, i) => {
          const m = line.match(markers);
          if (m) console.log(`  ${path.relative(ROOT, f)}:${i + 1}: ${line.trim().slice(0, 80)}`);
        });
      }
    }
  }
}
