#!/usr/bin/env node
/**
 * ec-lsp.js — Language Server untuk EnginotechC++ (.ec).
 * Fitur:
 *   - Diagnostics dari compiler (engc build)
 *   - Syntax checking (fallback jika engc tidak tersedia)
 *   - Completions (basic)
 *   - Hover info
 *   - Go-to-definition
 */
'use strict';
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');

/* ==================== KEYWORDS & BUILTINS ==================== */

const KEYWORDS = [
  'fn', 'let', 'mut', 'const', 'if', 'else', 'elif', 'for', 'while', 'return',
  'break', 'continue', 'match', 'struct', 'enum', 'impl', 'interface', 'import',
  'pub', 'as', 'in', 'module', 'export', 'load', 'try', 'catch', 'throw',
  'except', 'finally', 'test', 'assert', 'expect', 'requires', 'ensures',
  'invariant', 'coroutine', 'yield', 'await', 'hot', 'reload', 'bytecode',
  'vm', 'op', 'true', 'false', 'none', 'function', 'say', 'ask', 'set',
  'repeat', 'give', 'print', 'output', 'input'
];

const TYPES = [
  'int', 'int8', 'int16', 'int32', 'int64',
  'uint', 'uint8', 'uint16', 'uint32', 'uint64',
  'float32', 'float64', 'double', 'bool', 'char', 'string', 'byte', 'void',
  'Option', 'Result'
];

const BUILTINS = [
  { name: 'print', detail: 'print(s: string) -> void', kind: 'Function' },
  { name: 'println', detail: 'println(s: string) -> void', kind: 'Function' },
  { name: 'str', detail: 'str(x: any) -> string', kind: 'Function' },
  { name: 'len', detail: 'len(s: string) -> int', kind: 'Function' },
  { name: 'input', detail: 'input() -> string', kind: 'Function' },
  { name: 'abs', detail: 'abs(x: int) -> int', kind: 'Function' },
  { name: 'min', detail: 'min(a: int, b: int) -> int', kind: 'Function' },
  { name: 'max', detail: 'max(a: int, b: int) -> int', kind: 'Function' },
  { name: 'sqrt', detail: 'sqrt(x: float64) -> float64', kind: 'Function' },
  { name: 'pow', detail: 'pow(x: float64, y: float64) -> float64', kind: 'Function' },
];

/* ==================== DIAGNOSTIC PARSER ==================== */

function parseCompilerOutput(output, filePath) {
  const diagnostics = [];
  const lines = output.split('\n');
  
  for (const line of lines) {
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
      resolve([]); // No compiler found
      return;
    }
    
    const tmpFile = path.join(os.tmpdir(), `ec_compile_${Date.now()}.ec`);
    fs.writeFileSync(tmpFile, fileContent);
    
    const args = ['build', tmpFile];
    const child = spawn(engcPath, args, { timeout: 10000 });
    
    let stderr = '';
    child.stderr.on('data', (data) => { stderr += data.toString(); });
    
    child.on('close', (code) => {
      try { fs.unlinkSync(tmpFile); } catch (_) {}
      
      if (code === 0 || !stderr) {
        resolve([]);
      } else {
        const diagnostics = parseCompilerOutput(stderr, filePath);
        resolve(diagnostics);
      }
    });
    
    child.on('error', () => {
      resolve([]);
    });
  });
}

/* ==================== COMPLETIONS ==================== */

function getCompletions(text, position) {
  const items = [];
  const wordAtPos = getWordAtPosition(text, position);
  
  // Add keywords
  for (const kw of KEYWORDS) {
    if (kw.startsWith(wordAtPos)) {
      items.push({
        label: kw,
        kind: 14, // Keyword
        detail: 'keyword',
        insertText: kw
      });
    }
  }
  
  // Add types
  for (const t of TYPES) {
    if (t.startsWith(wordAtPos)) {
      items.push({
        label: t,
        kind: 7, // Class/Type
        detail: 'type',
        insertText: t
      });
    }
  }
  
  // Add builtins
  for (const b of BUILTINS) {
    if (b.name.startsWith(wordAtPos)) {
      items.push({
        label: b.name,
        kind: 2, // Function
        detail: b.detail,
        documentation: b.detail,
        insertText: b.name + (b.name === 'print' ? '(' : '')
      });
    }
  }
  
  // Add user-defined functions from current file
  const funcRegex = /^\s*(?:pub\s+)?fn\s+([A-Za-z_]\w*)/gm;
  let match;
  while ((match = funcRegex.exec(text))) {
    const name = match[1];
    if (name.startsWith(wordAtPos)) {
      items.push({
        label: name,
        kind: 2, // Function
        detail: 'fn ' + name,
        insertText: name
      });
    }
  }
  
  // Add structs
  const structRegex = /^\s*struct\s+([A-Za-z_]\w*)/gm;
  while ((match = structRegex.exec(text))) {
    const name = match[1];
    if (name.startsWith(wordAtPos)) {
      items.push({
        label: name,
        kind: 7, // Class
        detail: 'struct ' + name,
        insertText: name
      });
    }
  }
  
  return items;
}

function getWordAtPosition(text, position) {
  const lines = text.split('\n');
  const line = lines[position.line] || '';
  let start = position.character;
  while (start > 0 && /[\w]/.test(line[start - 1])) start--;
  return line.slice(start, position.character);
}

/* ==================== HOVER ==================== */

function getHover(text, position) {
  const word = getWordAtPosition(text, position);
  
  // Check keywords
  if (KEYWORDS.includes(word)) {
    return { contents: { kind: 'markdown', value: `**Keyword:** \`${word}\`` } };
  }
  
  // Check types
  if (TYPES.includes(word)) {
    return { contents: { kind: 'markdown', value: `**Type:** \`${word}\`` } };
  }
  
  // Check builtins
  const builtin = BUILTINS.find(b => b.name === word);
  if (builtin) {
    return { contents: { kind: 'markdown', value: `**Function:** \`${builtin.detail}\`` } };
  }
  
  // Check user functions
  const funcRegex = /^\s*(?:pub\s+)?fn\s+([A-Za-z_]\w*)\s*\(([^)]*)\)\s*(?:->\s*([A-Za-z_]\w*))?/gm;
  let match;
  while ((match = funcRegex.exec(text))) {
    if (match[1] === word) {
      return { 
        contents: { 
          kind: 'markdown', 
          value: `**Function:** \`fn ${match[1]}(${match[2] || ''})${match[3] ? ' -> ' + match[3] : ''}\`` 
        } 
      };
    }
  }
  
  // Check structs
  const structRegex = /^\s*struct\s+([A-Za-z_]\w*)/gm;
  while ((match = structRegex.exec(text))) {
    if (match[1] === word) {
      return { contents: { kind: 'markdown', value: `**Struct:** \`struct ${match[1]}\`` } };
    }
  }
  
  return null;
}

/* ==================== GO-TO-DEFINITION ==================== */

function getDefinition(text, position) {
  const word = getWordAtPosition(text, position);
  const lines = text.split('\n');
  
  // Search for function definition
  const funcRegex = /^\s*(?:pub\s+)?fn\s+([A-Za-z_]\w*)/gm;
  let match;
  while ((match = funcRegex.exec(text))) {
    if (match[1] === word) {
      const lineNum = text.slice(0, match.index).split('\n').length - 1;
      return [{
        uri: 'file://' + path.resolve('.'), // placeholder, will be replaced by client
        range: {
          start: { line: lineNum, character: match.index - (lines[lineNum] ? lines[lineNum].indexOf('fn') : 0) },
          end: { line: lineNum, character: match.index + match[0].length }
        }
      }];
    }
  }
  
  // Search for struct definition
  const structRegex = /^\s*struct\s+([A-Za-z_]\w*)/gm;
  while ((match = structRegex.exec(text))) {
    if (match[1] === word) {
      const lineNum = text.slice(0, match.index).split('\n').length - 1;
      return [{
        uri: 'file://' + path.resolve('.'),
        range: {
          start: { line: lineNum, character: match.index - (lines[lineNum] ? lines[lineNum].indexOf('struct') : 0) },
          end: { line: lineNum, character: match.index + match[0].length }
        }
      }];
    }
  }
  
  return [];
}

/* ==================== SYNTAX ANALYSIS ==================== */

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
      reply(id, { capabilities: { 
        textDocumentSync: 1, // full
        completionProvider: { triggerCharacters: ['.', ':', '<', '('] },
        hoverProvider: true,
        definitionProvider: true
      } });
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
    case 'textDocument/completion':
      if (params.textDocument && docs.has(params.textDocument.uri)) {
        const text = docs.get(params.textDocument.uri).text;
        const items = getCompletions(text, params.position);
        reply(id, { items, isIncomplete: false });
      } else {
        reply(id, { items: [], isIncomplete: false });
      }
      break;
    case 'textDocument/hover':
      if (params.textDocument && docs.has(params.textDocument.uri)) {
        const text = docs.get(params.textDocument.uri).text;
        const hover = getHover(text, params.position);
        reply(id, hover);
      } else {
        reply(id, null);
      }
      break;
    case 'textDocument/definition':
      if (params.textDocument && docs.has(params.textDocument.uri)) {
        const text = docs.get(params.textDocument.uri).text;
        const defs = getDefinition(text, params.position);
        reply(id, defs);
      } else {
        reply(id, []);
      }
      break;
    case '$/cancelRequest': break;
    default:
      if (id !== undefined) reply(id, {});
  }
}