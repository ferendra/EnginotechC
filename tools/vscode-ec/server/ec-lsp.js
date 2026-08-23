#!/usr/bin/env node
/**
 * ec-lsp.js — Language Server untuk EnginotechC++ (.ec).
 * Fitur:
 *   - Diagnostics dari compiler (engc build)
 *   - Syntax checking (fallback jika engc tidak tersedia)
 */
'use strict';
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');

/* ==================== DIAGNOSTIC PARSER ==================== */

// Parse compiler output to extract diagnostics
function parseCompilerOutput(output, filePath) {
  const diagnostics = [];
  const lines = output.split('\n');
  
  for (const line of lines) {
    // Match pattern: [error] (CODE) message or [error] file:line:col -> message
    const errorMatch = line.match(/\[error\]\s*\((E?\d+)\)\s*(.+)$/);
    if (errorMatch) {
      diagnostics.push({
        severity: 1, // Error
        code: errorMatch[1],
        message: errorMatch[2].trim(),
        range: { start: { line: 0, character: 0 }, end: { line: 0, character: 999 } }
      });
      continue;
    }
    
    // Match pattern: file:line:col -> message
    const fileMatch = line.match(/^(.+?):(\d+):(\d+)\s*->\s*(.+)$/);
    if (fileMatch) {
      const lineNum = parseInt(fileMatch[2]) - 1;
      const colNum = parseInt(fileMatch[3]) - 1;
      diagnostics.push({
        severity: 1,
        code: '',
        message: fileMatch[4].trim(),
        range: { 
          start: { line: lineNum, character: colNum }, 
          end: { line: lineNum, character: colNum + 1 } 
        }
      });
      continue;
    }
    
    // Match pattern: line:col message (for notes/other)
    const noteMatch = line.match(/^(\d+):(\d+)\s+(.+)$/);
    if (noteMatch && !line.includes('===')) {
      const lineNum = parseInt(noteMatch[1]) - 1;
      diagnostics.push({
        severity: 3, // Hint
        code: '',
        message: noteMatch[3].trim(),
        range: { start: { line: lineNum, character: 0 }, end: { line: lineNum, character: 999 } }
      });
    }
  }
  
  return diagnostics;
}

// Run engc compiler to get real diagnostics
function runCompiler(fileContent, filePath) {
  return new Promise((resolve) => {
    // Find engc binary
    const engcPaths = [
      path.join(os.homedir(), '.local', 'bin', 'engc'),
      '/usr/local/bin/engc',
      '/usr/bin/engc',
      path.join(__dirname, '..', '..', 'engc'),
      'engc'
    ];
    
    let engcPath = null;
    for (const p of engcPaths) {
      if (fs.existsSync(p) || p === 'engc') {
        engcPath = p;
        break;
      }
    }
    
    if (!engcPath) {
      resolve([]); // No compiler found, return empty
      return;
    }
    
    // Create temp file for compilation
    const tmpFile = path.join(os.tmpdir(), `ec_compile_${Date.now()}.ec`);
    fs.writeFileSync(tmpFile, fileContent);
    
    const args = ['build', tmpFile];
    const child = spawn(engcPath, args, { timeout: 10000 });
    
    let stderr = '';
    child.stderr.on('data', (data) => { stderr += data.toString(); });
    
    child.on('close', (code) => {
      // Clean up temp file
      try { fs.unlinkSync(tmpFile); } catch (_) {}
      
      if (code === 0 || !stderr) {
        resolve([]); // No errors
      } else {
        const diagnostics = parseCompilerOutput(stderr, filePath);
        resolve(diagnostics);
      }
    });
    
    child.on('error', () => {
      resolve([]); // Spawn error, return empty
    });
  });
}

// Fallback: basic syntax analysis
function analyzeSyntax(src) {
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
          diags.push({
            severity: 1,
            message: `'${c}' without an opening bracket`,
            range: { start: { line: line(i), character: 0 }, end: { line: line(i), character: 999 } }
          });
          depth[close[c]] = 0;
        }
      }
    } else if (inStr) {
      if (c === '\\') i++;
      else if (c === '"') inStr = false;
      else if (c === '\n') { 
        diags.push({
          severity: 1,
          message: 'Unterminated string',
          range: { start: { line: line(strStart), character: 0 }, end: { line: line(strStart), character: 999 } }
        });
        inStr = false; 
      }
    } else if (inLineC && c === '\n') inLineC = false;
    else if (inBlockC && c === '*' && n === '/') { inBlockC = false; i++; }
  }
  
  if (inStr) {
    diags.push({
      severity: 1,
      message: 'Unterminated string',
      range: { start: { line: line(strStart), character: 0 }, end: { line: line(strStart), character: 999 } }
    });
  }
  
  for (const k of ['{', '(', '[']) {
    if (depth[k] > 0)
      diags.push({
        severity: 1,
        message: `${depth[k]} unclosed '${k}'`,
        range: { start: { line: src.split('\n').length - 1, character: 0 }, end: { line: src.split('\n').length - 1, character: 999 } }
      });
  }
  
  // Check for duplicate function names
  const re = /^\s*(?:pub\s+)?fn\s+([A-Za-z_]\w*)/gm;
  let m;
  while ((m = re.exec(src))) {
    const name = m[1];
    const ln = m.index === 0 ? 0 : src.slice(0, m.index).split('\n').length - 1;
    if (fns.has(name)) {
      diags.push({
        severity: 2, // Warning
        message: `Function '${name}' is declared more than once`,
        range: { start: { line: ln, character: 0 }, end: { line: ln, character: 999 } }
      });
    } else {
      fns.set(name, ln);
    }
  }
  
  return diags;
}

/* ==================== JSON-RPC SERVER ==================== */

const docs = new Map();
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
    try { handleMessage(JSON.parse(body)); } catch (_) {}
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

async function publishDiagnostics(uri, text) {
  // Try compiler first, fallback to syntax analysis
  let diagnostics = await runCompiler(text, uri);
  
  // If no compiler errors, do basic syntax check
  if (diagnostics.length === 0) {
    diagnostics = analyzeSyntax(text);
  }
  
  send('textDocument/publishDiagnostics', { uri, diagnostics });
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
      publishDiagnostics(params.textDocument.uri, params.textDocument.text);
      break;
    case 'textDocument/didChange':
      for (const ch of params.contentChanges)
        if (ch.text !== undefined) docs.set(params.textDocument.uri, { text: ch.text });
      publishDiagnostics(params.textDocument.uri, ch.text);
      break;
    case 'textDocument/didClose':
      docs.delete(params.textDocument.uri);
      break;
    case '$/cancelRequest': break;
    default:
      if (id !== undefined) reply(id, {});
  }
}
