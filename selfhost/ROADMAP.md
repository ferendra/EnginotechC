# Self-Hosting EnginotechC++ — Tangga Bootstrap

Tujuan: **Enginotech ditulis dengan Enginotech** (seperti rustc di Rust).
Prinsip: setiap stage HARUS lolos differential test vs implementasi C++ sebelum lanjut.

## Tangga

| Stage | Isi | Status |
|---|---|---|
| 0 | engc C++ mampu compile EC | ✅ selesai |
| **1** | **Lexer EC dalam EC** (`stage1-lexer/main.ec`) — tokenizer streaming + 8 self-test assert | ✅ **LOLOS 2026-08-22** |
| 2 | Parser EC dalam EC → dump token-stream ke file; diff vs output C++ | belum |
| 3 | AST + Typechecker port; bandingkan jumlah error pada corpus rusak | belum |
| 4 | IRGen port (emit LLVM IR identik untuk subset aman) | belum |
| 5 | Driver `engc-ec` mengompilasi dirinya sendiri; C++ hanya bootstrap terakhir | belum |

## Hasil nyata Stage 1 — bug COMPILER yang ditemukan karena menulis di EC
Menulis ulang di EC terbukti jadi ujian paling kasar bagi engc C++. Ditemukan & diperbaiki:
1. `s[i]` pada string memuat kata i32 utuh, bukan 1 byte → kini load i8+zext.
2. `s[i] = c` memakai stride 8 seperti array → string kini stride 1.
3. `escapeLLVMString` memancarkan `\"` yang merusak parsing c-string di zig-cc → hex `\22`/`\5C`.
4. Klasifikasi return-type builtin salah banyak: len→string, chr/read_file/input→void,
   contains/file_exists/write_file→void (kini string/int/bool benar).
5. Return-type fungsi user DIABAIKAN ("assume int") → kini dilacak via fnRetTypes_.
6. Variabel for-loop terdaftar nullptr → E1001 palsu (kini "unknown").
7. Assignment menolak tipe "unknown" (kini diloloskan — unknown ≠ error).

## Kebutuhan bahasa yang ditemukan selama Stage 1 (umpan balik ke fondasi)
- [x] `len()`, indexing byte `s[i]`, `chr`, `substring`, concat `+` (sudah ada)
- [ ] List dinamis lintas-fungsi (header panjang di alokasi) — Fase B fondasi #6;
      tanpa ini lexer tak bisa MENGEMBALIKAN daftar token dari fungsi
      (workaround Stage 1: scan streaming di satu fungsi + hitung kategori)
- [ ] Dict/map untuk tabel keyword & simbol (Fase B #2)
- [ ] Unit-test framework di std (std/testing.cpp ada — butuh binding .ec yang enak)

## Aturan main
1. Tidak boleh menambah fitur C++; semua gap diselesaikan lewat fitur EC baru.
2. Setiap stage punya self-test `assert` — exit code 0 = lulus.
3. Differential check tiap stage: output EC vs C++ harus identik byte-per-byte.
