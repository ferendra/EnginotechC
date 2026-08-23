# EC Tools — helper skrip JavaScript (Node ≥ 18, tanpa dependensi)

Kumpulan skrip pembantu untuk fungsi-fungsi dasar pengembangan
EnginotechC++. Semua berjalan dengan `node` polos — tanpa `npm install`.

## ec-build.js — build compiler

```bash
node tools/ec-build.js                 # -> build/engc
node tools/ec-build.js -o /tmp/engc    # lokasi lain
node tools/ec-build.js --quiet         # sembunyikan progres
```

Daftar source mengikuti CMakeLists.txt; warning compiler diteruskan ke stderr.

## ec-smoke.js — suite uji end-to-end

Menjalankan setiap `tests/smoke/*.ec` lewat `engc run`, membandingkan stdout
dengan `.expected` (mendukung `.stdin` dan kasus negatif `*_neg_*`).

```bash
node tools/ec-build.js && node tools/ec-smoke.js     # build + uji
node tools/ec-smoke.js --bin ./build/engc            # pakai binary lain
```

Exit code 0 = semua hijau; cocok untuk pre-commit hook / CI.

## ec-analyze.js — statistik proyek

```bash
node tools/ec-analyze.js               # LOC per modul + jumlah TODO/FIXME
node tools/ec-analyze.js --todo        # daftar lokasi marker
node tools/ec-analyze.js --ec          # ringkasan program .ec (fn/struct)
```

## Catatan backend

`engc run` butuh clang **atau** llvm-as+llc+cc di PATH. Di mesin tanpa keduanya,
set `ENGC_LLVM_BIN=<dir>` dan bila perlu `ENGC_LD_LIBRARY_PATH` sebelum menjalankan
ec-smoke.js (skrip mewarisi environment).

## ec-doctor.js — cek lingkungan

```bash
node tools/ec-doctor.js     # status g++, backend native, folder proyek
```

## ec-new.js — scaffold proyek baru

```bash
node tools/ec-new.js nama-app [dir-induk]   # buat src/main.ec + README
```

## ec-watch.js — rebuild + retest otomatis

```bash
export ENGC_LLVM_BIN=... ; export ENGC_LD_LIBRARY_PATH=...
node tools/ec-watch.js --src=all   # pantau compiler/std/runtime/gui
```

## LSP server — integrasi editor

`tools/lsp/ec-lsp.js`: Language Server LSP 3.x over stdio.
Diagnostics cepat sisi JS: kurung tak seimbang, string tak ditutup,
fungsi duplikat. VSCode: salin/symlink `tools/vscode-ec/` ke
`~/.vscode/extensions/` lalu reload.

## Uji LSP manual

printf 'Content-Length: N\r\n\r\n{...}' | node tools/lsp/ec-lsp.js
