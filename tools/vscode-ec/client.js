// Client LSP untuk EnginotechC++ (.ec) - Full IDE support
'use strict';
const { spawn, execFile } = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');

const vscode = require('vscode');

// Check if compiler binary is available
function compilerAvailable(engc) {
  return new Promise((resolve) => {
    try {
      execFile(engc, ['version'], { timeout: 5000 }, (err) => resolve(!err));
    } catch (_) {
      resolve(false);
    }
  });
}

// Resolve compiler path from various locations
function resolveCompiler(cfgValue) {
  const candidates = [];
  if (cfgValue && cfgValue !== 'engc') candidates.push(cfgValue);
  candidates.push(path.join(os.homedir(), '.local', 'bin', 'engc'));
  candidates.push('/usr/local/bin/engc');
  candidates.push('/usr/bin/engc');
  for (const wf of vscode.workspace.workspaceFolders || []) {
    candidates.push(path.join(wf.uri.fsPath, 'engc'));
  }
  for (const c of candidates) {
    try { if (fs.existsSync(c)) return c; } catch (_) { /* ignore */ }
  }
  return null;
}

let server;
let client;

function activate(context) {
  // Bundled LSP server
  const bundled = path.join(__dirname, 'server', 'ec-lsp.js');
  const legacy = path.resolve(__dirname, '..', 'lsp', 'ec-lsp.js');
  const serverPath = fs.existsSync(bundled) ? bundled : legacy;
  const output = vscode.window.createOutputChannel('EnginotechC++');

  function startServer() {
    server = spawn(process.execPath, [serverPath]);
    const send = (msg) => {
      const body = JSON.stringify(msg);
      server.stdin.write(`Content-Length: ${Buffer.byteLength(body)}\r\n\r\n${body}`);
    };

    let buf = '';
    server.stdout.on('data', (d) => {
      buf += d.toString();
      let i;
      while ((i = buf.indexOf('\r\n\r\n')) >= 0) {
        const len = /Content-Length:\s*(\d+)/i.exec(buf.slice(0, i));
        const body = buf.slice(i + 4, i + 4 + Number(len[1]));
        buf = buf.slice(i + 4 + Number(len[1]));
        try { handle(JSON.parse(body)); } catch (_) {}
      }
    });

    function handle(msg) {
      if (msg.method === 'textDocument/publishDiagnostics') {
        const diags = msg.params.diagnostics.map((d) =>
          new vscode.Diagnostic(
            new vscode.Range(d.range.start.line, d.range.start.character,
                             d.range.end.line, d.range.end.character),
            d.message, d.severity === 2 ? vscode.DiagnosticSeverity.Warning
                                        : vscode.DiagnosticSeverity.Error));
        const uri = vscode.Uri.parse(msg.params.uri);
        client.diagnostics.set(uri, diags);
      } else if (msg.method === 'textDocument/completion') {
        // Forward completions
        if (msg.id !== undefined && client.pendingCompletions.has(msg.id)) {
          const resolve = client.pendingCompletions.get(msg.id);
          client.pendingCompletions.delete(msg.id);
          resolve(msg.result);
        }
      } else if (msg.method === 'textDocument/hover') {
        // Forward hover
        if (msg.id !== undefined && client.pendingHovers.has(msg.id)) {
          const resolve = client.pendingHovers.get(msg.id);
          client.pendingHovers.delete(msg.id);
          resolve(msg.result);
        }
      } else if (msg.method === 'textDocument/definition') {
        // Forward go-to-definition
        if (msg.id !== undefined && client.pendingDefinitions.has(msg.id)) {
          const resolve = client.pendingDefinitions.get(msg.id);
          client.pendingDefinitions.delete(msg.id);
          resolve(msg.result);
        }
      }
    }

    client = {
      diagnostics: vscode.languages.createDiagnosticCollection('ec'),
      send,
      pendingCompletions: new Map(),
      pendingHovers: new Map(),
      pendingDefinitions: new Map(),
    };
    context.subscriptions.push(client.diagnostics);

    // Initialize LSP
    send({ jsonrpc: '2.0', id: 1, method: 'initialize',
           params: { capabilities: {
             textDocument: {
               completion: { completionItem: { snippetSupport: true } },
               hover: { dynamicRegistration: true },
               definition: { dynamicRegistration: true }
             }
           } } });
    send({ jsonrpc: '2.0', method: 'initialized', params: {} });

    // Document sync
    for (const ev of ['didOpen', 'didChange']) {
      context.subscriptions.push(vscode.workspace.onDidSaveTextDocument((doc) => {
        if (!doc.fileName.endsWith('.ec')) return;
        send({ jsonrpc: '2.0', method: `textDocument/${ev}`,
               params: { textDocument: { uri: doc.uri.toString(), text: doc.getText() },
                         contentChanges: [{ text: doc.getText() }] } });
      }));
    }
    context.subscriptions.push(vscode.workspace.onDidCloseTextDocument((doc) => {
      if (!doc.fileName.endsWith('.ec')) return;
      send({ jsonrpc: '2.0', method: 'textDocument/didClose',
             params: { textDocument: { uri: doc.uri.toString() } } });
    }));
    vscode.workspace.textDocuments.forEach((doc) => {
      if (doc.fileName.endsWith('.ec'))
        send({ jsonrpc: '2.0', method: 'textDocument/didOpen',
               params: { textDocument: { uri: doc.uri.toString(), text: doc.getText() } } });
    });
  }

  startServer();

  // ▶ Run File - Ctrl+F5
  const runCmd = vscode.commands.registerCommand('ec.run', async () => {
    const ed = vscode.window.activeTextEditor;
    const doc = ed && ed.document;
    const isEc = !!doc && (doc.languageId === 'ec' || /\.ec$/i.test(doc.fileName));
    if (!isEc) {
      vscode.window.showErrorMessage('The active editor is not an EC (.ec) file.');
      return;
    }
    await doc.save();
    const cfg = vscode.workspace.getConfiguration('ec');
    const engc = resolveCompiler(cfg.get('compilerPath', 'engc'));

    if (!engc || !(await compilerAvailable(engc))) {
      const pick = await vscode.window.showErrorMessage(
        'EC compiler not found. Install to ~/.local/bin/engc (./scripts/build.sh then copy), or set "EC: Compiler Path" in Settings.',
        'Open Settings'
      );
      if (pick === 'Open Settings') {
        vscode.commands.executeCommand('workbench.action.openSettings', 'ec.compilerPath');
      }
      return;
    }

    const old = vscode.window.terminals.find((t) => t.name === 'EC Run');
    if (old) old.dispose();
    let term = vscode.window.createTerminal({ name: 'EC Run' });
    term.show(true);
    term.sendText(`${JSON.stringify(engc)} run ${JSON.stringify(doc.fileName)}`);
  });
  context.subscriptions.push(runCmd);

  // 🔨 Build Project - Ctrl+Shift+B
  const buildCmd = vscode.commands.registerCommand('ec.build', async () => {
    const cfg = vscode.workspace.getConfiguration('ec');
    const engc = resolveCompiler(cfg.get('compilerPath', 'engc'));
    
    if (!engc || !(await compilerAvailable(engc))) {
      vscode.window.showErrorMessage('EC compiler not found. Check "EC: Compiler Path" setting.');
      return;
    }

    const ws = vscode.workspace.workspaceFolders;
    if (!ws || ws.length === 0) {
      vscode.window.showErrorMessage('No workspace folder open');
      return;
    }

    const term = vscode.window.createTerminal({ name: 'EC Build' });
    term.show(true);
    term.sendText(`${JSON.stringify(engc)} build`);
  });
  context.subscriptions.push(buildCmd);

  // 🔄 Restart LSP - Ctrl+Shift+R
  const restartCmd = vscode.commands.registerCommand('ec.restartLsp', () => {
    if (server) server.kill();
    output.clear();
    output.appendLine('Restarting EnginotechC++ Language Server...');
    startServer();
    vscode.window.showInformationMessage('Language Server restarted');
  });
  context.subscriptions.push(restartCmd);

  // Completion Provider
  const completionProvider = vscode.languages.registerCompletionItemProvider(
    'ec',
    {
      async provideCompletionItems(document, position) {
        if (!client || !vscode.workspace.getConfiguration('ec').get('completionEnabled', true)) {
          return [];
        }
        const items = await requestCompletions(document, position);
        return items;
      }
    },
    '.', ':', '<', '(' // trigger characters
  );
  context.subscriptions.push(completionProvider);

  // Hover Provider
  const hoverProvider = vscode.languages.registerHoverProvider('ec', {
    async provideHover(document, position) {
      if (!client) return null;
      return await requestHover(document, position);
    }
  });
  context.subscriptions.push(hoverProvider);

  // Definition Provider
  const definitionProvider = vscode.languages.registerDefinitionProvider('ec', {
    async provideDefinition(document, position) {
      if (!client) return null;
      return await requestDefinition(document, position);
    }
  });
  context.subscriptions.push(definitionProvider);
}

async function requestCompletions(document, position) {
  return new Promise((resolve) => {
    const id = Date.now() + Math.random();
    client.pendingCompletions.set(id, (result) => {
      if (!result || !result.items) { resolve([]); return; }
      const items = result.items.map((item) => {
        const ci = new vscode.CompletionItem(item.label, item.kind || vscode.CompletionItemKind.Text);
        if (item.detail) ci.detail = item.detail;
        if (item.documentation) ci.documentation = item.documentation;
        if (item.insertText) ci.insertText = item.insertText;
        return ci;
      });
      resolve(items);
    });
    client.send({
      jsonrpc: '2.0',
      id,
      method: 'textDocument/completion',
      params: {
        textDocument: { uri: document.uri.toString() },
        position: { line: position.line, character: position.character }
      }
    });
  });
}

async function requestHover(document, position) {
  return new Promise((resolve) => {
    const id = Date.now() + Math.random();
    client.pendingHovers.set(id, (result) => {
      if (!result || !result.contents) { resolve(null); return; }
      const md = new vscode.MarkdownString(result.contents.value || result.contents);
      resolve(new vscode.Hover(md));
    });
    client.send({
      jsonrpc: '2.0',
      id,
      method: 'textDocument/hover',
      params: {
        textDocument: { uri: document.uri.toString() },
        position: { line: position.line, character: position.character }
      }
    });
  });
}

async function requestDefinition(document, position) {
  return new Promise((resolve) => {
    const id = Date.now() + Math.random();
    client.pendingDefinitions.set(id, (result) => {
      if (!result || result.length === 0) { resolve(null); return; }
      const loc = result[0] || result;
      resolve(new vscode.Location(
        vscode.Uri.parse(loc.uri),
        new vscode.Range(loc.range.start.line, loc.range.start.character,
                         loc.range.end.line, loc.range.end.character)
      ));
    });
    client.send({
      jsonrpc: '2.0',
      id,
      method: 'textDocument/definition',
      params: {
        textDocument: { uri: document.uri.toString() },
        position: { line: position.line, character: position.character }
      }
    });
  });
}

function deactivate() { 
  if (server) server.kill(); 
}

module.exports = { activate, deactivate };