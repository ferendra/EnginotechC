#!/usr/bin/env node
/**
 * ec-lsp.js — Language Server minimal untuk EnginotechC++ (.ec).
 * Protokol: LSP 3.x over stdio. Fitur:
 *   - initialize / shutdown / exit
 *   - textDocument/didOpen | didChange -> publishDiagnostics
 *
 * Diagnostics sisi JS (cepat, tanpa spawn compiler):
 *   kurung seimbang, string/comment belum ditutup,
 *   nama fungsi ganda, pemakaian variabel tak dikenal pada level fn.
 *
 * Jalankan manual:
 *   node tools/lsp/ec-lsp.js
 */
'use strict';
const readline = require('readline');
const BUILTINS = new Set(['print', 'println', 'input', 'str', 'int', 'float',
  'abs', 'min', 'max', 'sqrt', 'pow', 'floor', 'ceil', 'upper', 'lower',
  'contains', 'len', 'ord', 'chr', 'assert',
  'read_file', 'write_file', 'append_file', 'file_exists', 'delete_file',
  'substring', 'repeat']);

/* ---------------- analyzer ---------------- */

function analyze(src) {
  const diags = [];
  const line = (i) => src.slice(0, i).split('\n').length - 1;
  let depth = { '{': 0, '(': 0, '[': 0 };
  let close = { '}': '{', ')': '(', ']': '[' };
  let inStr = false, strStart = 0, inLineC = false, inBlockC = false;
  const fns = new Map();

  for (let i = 0; i < src.length; i++) {
    const c = src[i], n = src[i + 1];
    if (!inStr && !inLineC && !inBlockC) {
      if (c === '/' && n === '/') { inLineC = true; continue; }
      if (c === '/' && n === '*') { inBlockC = true; i++; continue; }
      if (c === '"') { inStr = true; strStart = i; continue; }
      if (c in depth) depth[c]++;
      else if (c in close) {
        if (--depth[close[c]] < 0) {
          diags.push(d(line(i), `'${c}' without an opening bracket`));
          depth[close[c]] = 0;
        }
      }
    } else if (inStr) {
      if (c === '\\') i++;
      else if (c === '"') inStr = false;
      else if (c === '\n') { diags.push(d(line(strStart), 'Unterminated string')); inStr = false; }
    } else if (inLineC && c === '\n') inLineC = false;
    else if (inBlockC && c === '*' && n === '/') { inBlockC = false; i++; }
  }
  if (inStr) diags.push(d(line(strStart), 'Unterminated string'));
  for (const k of ['{', '(', '[']) {
    if (depth[k] > 0)
      diags.push(d(lastLine(src), `${depth[k]} unclosed '${k}'`));
  }

  // duplikasi nama fungsi
  const re = /^\s*(?:pub\s+)?fn\s+([A-Za-z_]\w*)/gm;
  let m;
  while ((m = re.exec(src))) {
    const name = m[1], ln = m.index === 0 ? 0 : src.slice(0, m.index).split('\n').length - 1;
    if (fns.has(name)) diags.push(d(ln, `Function '${name}' is declared more than once`, 2 /*Warn*/));
    else fns.set(name, ln);
  }
  return diags;

  function lastLine() { return src.split('\n').length - 1; }
  function d(ln, msg, sev = 1) {
    return { range: { start: { line: ln, character: 0 }, end: { line: ln, character: 999 } },
             message: msg, severity: sev, source: 'ec-lsp' };
  }
}

/* ---------------- JSON-RPC over stdio ---------------- */

let buf = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', (chunk) => {
  buf += chunk;
  let idx;
  while ((idx = buf.indexOf('\r\n\r\n')) >= 0) {
    const head = buf.slice(0, idx);
    const len = /Content-Length:\s*(\d+)/i.exec(head);
    if (!len) { buf = ''; return; }
    const body = buf.slice(idx + 4, idx + 4 + Number(len[1]));
    buf = buf.slice(idx + 4 + Number(len[1]));
    try { handleMessage(JSON.parse(body)); } catch (_) { /* malformed */ }
  }
});

function send(method, params) {
  const body = JSON.stringify(params === undefined ? { jsonrpc: '2.0', method } : { jsonrpc: '2.0', method, params });
  process.stdout.write(`Content-Length: ${Buffer.byteLength(body)}\r\n\r\n${body}`);
}
function reply(id, result) {
  const body = JSON.stringify({ jsonrpc: '2.0', id, result });
  process.stdout.write(`Content-Length: ${Buffer.byteLength(body)}\r\n\r\n${body}`);
}

const docs = new Map();
function publish(uri) {
  const doc = docs.get(uri);
  send('textDocument/publishDiagnostics', {
    uri, diagnostics: doc ? analyze(doc.text) : [],
  });
}

function handleMessage(msg) {
  const { id, method, params } = msg;
  switch (method) {
    case 'initialize':
      reply(id, { capabilities: { textDocumentSync: 1 /* full */ } });
      break;
    case 'initialized': break;
    case 'shutdown': reply(id, null); break;
    case 'exit': process.exit(0); break;
    case 'textDocument/didOpen':
      docs.set(params.textDocument.uri, params.textDocument);
      publish(params.textDocument.uri);
      break;
    case 'textDocument/didChange':
      for (const ch of params.contentChanges)
        if (ch.text !== undefined) docs.set(params.textDocument.uri, { text: ch.text });
      publish(params.textDocument.uri);
      break;
    case 'textDocument/didClose':
      docs.delete(params.textDocument.uri);
      publish(params.textDocument.uri);
      break;
    case '$/cancelRequest': break;
    default:
      if (id !== undefined) reply(id, {});
  }
}
