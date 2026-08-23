# EnginotechC++ (EC) — Ekstensi VSCode

Dukungan bahasa EC di Visual Studio Code: syntax highlighting, bracket matching,
dan diagnostics real-time via LSP ringan (tanpa dependensi npm).

Logo bahasa: huruf **E** biru muda (#4FC3F7) di atas background transparan
(`icon.png`, tampil otomatis di daftar ekstensi).

## Fitur
- **Syntax highlighting** untuk `.ec` — keyword, tipe, string + interpolasi `${...}`,
  komentar, angka, builtin (`print`, `len`, `str`, `input`, ...).
- **Diagnostics** — error compiler muncul inline saat file disimpan (LSP `ec-lsp.js`).
- **Language configuration** — auto-indent blok `{}`, komentar `//` & `/* */`, bracket pair.

## Instalasi (lokal, tanpa marketplace)
1. Salin/symlink folder ini ke direktori ekstensi VSCode:
   ```bash
   ln -s "$(pwd)/tools/vscode-ec" ~/.vscode/extensions/ec-language
   ```
   (Windows: salin folder ke `%USERPROFILE%\.vscode\extensions\ec-language`)
2. Restart VSCode.
3. Buka file `.ec` — status bar kanan-bawah harus menampilkan "EnginotechC++".

## Mencoba bahasa EC
Butuh binary compiler. Build sekali dari root repo:
```bash
./scripts/build.sh /usr/local/bin/engc     # atau path lain di PATH
```

### ▶ Tombol Play
Setelah ekstensi terpasang, buka file `.ec` → ikon **▶** muncul di pojok kanan atas
editor (dan `Ctrl+F5`). File disimpan otomatis lalu `engc run <file>` dieksekusi di
terminal "EC Run". Jika `engc` tidak ada di PATH, atur:
```
Settings → Extensions → EnginotechC++ → Ec: Compiler Path
```

## Struktur
| Berkas | Fungsi |
|---|---|
| `package.json` | Manifest: bahasa `.ec`, grammar, ikon |
| `icon.png` | **Logo BAHASA EC** — huruf E biru muda (#4FC3F7), transparan |
| `client.js` | Klien LSP mini (spawn `../lsp/ec-lsp.js`) |
| `syntaxes/ec.tmLanguage.json` | Grammar TextMate |
| `language-configuration.json` | Bracket/komentar/indent |

> **Keputusan branding:** ikon ini mewakili *bahasa* EnginotechC++ saja.
> Library/paket (registry) NANTI memakai logo berbeda per paket — jangan
> memakai logo bahasa untuk paket mana pun.

## Catatan LSP
`client.js` menjalankan `tools/lsp/ec-lsp.js` dengan `process.execPath`
(Node bawaan host VSCode) — pastikan folder `tools/lsp/` ikut ada.
Diagnostics terbitkan pada save (`didOpen`/`didChange`).
