// Client LSP ringan: menyambungkan ec-lsp.js ke VSCode tanpa vsce build.
'use strict';
const { spawn, execFile } = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');

const vscode = require('vscode');

// Resolves quickly whether the compiler binary is usable.
function compilerAvailable(engc) {
  return new Promise((resolve) => {
    try {
      execFile(engc, ['version'], { timeout: 5000 }, (err) => resolve(!err));
    } catch (_) {
      resolve(false);
    }
  });
}

// Find engc WITHOUT relying on the terminal's PATH (VSCode may have been
// launched before ~/.local/bin existed). Returns an absolute path or null.
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

function activate(context) {
  // Server di-bundle ke dalam ekstensi (server/ec-lsp.js) agar .vsix
  // self-contained; fallback ke lokasi lama untuk mode dev dari repo.
  const bundled = path.join(__dirname, 'server', 'ec-lsp.js');
  const legacy = path.resolve(__dirname, '..', 'lsp', 'ec-lsp.js');
  const serverPath = fs.existsSync(bundled) ? bundled : legacy;
  const output = vscode.window.createOutputChannel('EnginotechC++');

  function start() {
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
        // Map .ec -> editor document URI langsung.
        client.diagnostics.set(uri, diags);
      }
    }

    const client = {
      diagnostics: vscode.languages.createDiagnosticCollection('ec'),
      send,
    };
    context.subscriptions.push(client.diagnostics);

    send({ jsonrpc: '2.0', id: 1, method: 'initialize',
           params: { capabilities: {} } });
    send({ jsonrpc: '2.0', method: 'initialized', params: {} });

    for (const ev of ['didOpen', 'didChange']) {
      context.subscriptions.push(vscode.workspace.onDidSaveTextDocument((doc) => {
        if (!doc.fileName.endsWith('.ec')) return;
        send({ jsonrpc: '2.0', method: `textDocument/${ev}`,
               params: { textDocument: { uri: doc.uri.toString(), text: doc.getText() },
                         contentChanges: [{ text: doc.getText() }] } });
      }));
    }
    // didClose fires when a document is closed, not on save — handle separately.
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

  start();

  // ▶ Run File — save, then run `engc run <file>` in the integrated terminal.
  const runCmd = vscode.commands.registerCommand('ec.run', async () => {
    const ed = vscode.window.activeTextEditor;
    // Language id is authoritative (case-insensitive); extension regex as fallback.
    const doc = ed && ed.document;
    const isEc = !!doc && (doc.languageId === 'ec' || /\.ec$/i.test(doc.fileName));
    if (!isEc) {
      vscode.window.showErrorMessage('The active editor is not an EC (.ec) file.');
      return;
    }
    await doc.save();
    const cfg = vscode.workspace.getConfiguration('ec');
    const engc = resolveCompiler(cfg.get('compilerPath', 'engc'));

    // Pre-flight: fail with actionable guidance instead of a raw shell error.
    if (!engc || !(await compilerAvailable(engc))) {
      const pick = await vscode.window.showErrorMessage(
        'EC compiler not found. Install it to ~/.local/bin/engc ' +
        '(./scripts/build.sh then copy), or set "Ec: Compiler Path" in Settings.',
        'Open Settings'
      );
      if (pick === 'Open Settings') {
        vscode.commands.executeCommand('workbench.action.openSettings',
                                       'ec.compilerPath');
      }
      return;
    }

    // Fresh terminal per run: stale terminals keep an outdated environment
    // (e.g. missing ~/.local/bin from before the compiler was installed).
    const old = vscode.window.terminals.find((t) => t.name === 'EC Run');
    if (old) old.dispose();
    let term = vscode.window.createTerminal({ name: 'EC Run' });
    term.show(true);
    term.sendText(`${JSON.stringify(engc)} run ${JSON.stringify(doc.fileName)}`);
  });
  context.subscriptions.push(runCmd);
}

function deactivate() { if (server) server.kill(); }

module.exports = { activate, deactivate };
