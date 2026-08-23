# EnginotechC++ — Language Server Protocol (LSP)

Language server for EnginotechC++ providing IDE features via JSON-RPC 2.0.

## Usage

```bash
# Start LSP server
engc lsp

# Or from VS Code / Neovim / any LSP client
# Connect to stdio
```

## Features

- ✅ Completion (keywords, stdlib)
- ✅ Diagnostics (parse errors, type errors)
- ✅ Hover (keyword docs)
- ✅ Go-to-Definition
- ✅ Document Symbols

## JSON-RPC Methods

| Method | Description |
|--------|-------------|
| `initialize` | Server initialization |
| `initialized` | Notification that server is initialized |
| `textDocument/didOpen` | Document opened |
| `textDocument/didChange` | Document changed |
| `textDocument/didSave` | Document saved |
| `textDocument/didClose` | Document closed |
| `textDocument/completion` | Get completions |
| `textDocument/hover` | Get hover info |
| `textDocument/definition` | Go to definition |
| `textDocument/documentSymbol` | Get document symbols |
| `textDocument/diagnostic` | Get diagnostics |
| `shutdown` | Graceful shutdown |

## Protocol Format

```
Content-Length: <bytes>\r\n\r\n<json>
```

## Example

```bash
# Test server
echo -e 'Content-Length: 55\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' | engc lsp
```
