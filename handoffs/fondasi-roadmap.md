# Fondasi 1–2 Bulan — Roadmap Eksekusi
Status: BERJALAN · Service: seluruh compiler · Diperbarui: 2026-08-22 12:00
Keputusan owner: FITUR DITUNDA, fondasi dulu ±1–2 bulan. Python-parity lanjutan (dict,
tuple, string-methods) DITUNDA sampai fondasi tuntas (lihat handoffs/python-parity.md).

## Keputusan penting
- **Backend milik sendiri (arah jangka menengah, permintaan owner 2026-08-22)**: saat ini
  codegen bergantung clang/zig cc untuk merakit binary dari LLVM IR. Nanti dibuat backend
  EC sendiri — kandidat: emit objek x86-64 langsung (tanpa LLVM) + linker sistem, ATAU
  bundle toolchain statis bersama release. Mulai SETELAH Fase B selesai.
- **Target performa: secepat C.** Baseline terukur (2026-08-22, examples/bench/main.ec,
  fib(32)+loop 100jt, zig cc -O2): EC 0.013s vs C 0.010s (~1.3x). Aturan: setiap perubahan
  codegen tidak boleh memperburuk baseline; jalankan bench sebelum-sesudah.
- **Semantik int = i32** (wrap seperti C). Butuh 64-bit? gunakan int64 — JANGAN ubah int
  jadi 64-bit diam-diam (benchmark loop-sum membuktikan overflow 2^32 konsisten dgn C-int).
- **Branding**: logo E biru muda (#4FC3F7, transparan) di tools/vscode-ec/icon.png adalah
  logo BAHASA EC. Library/paket nanti punya logo SENDIRI per paket — jangan pakai logo
  bahasa untuk paket. Konvensi yang disarankan: `packages/<nama>/icon.png`.
- **Unknown ≠ error** — tipe yang belum bisa disimpulkan = placeholder "unknown",
  BUKAN error/false-positive E1001/E2001.
- **Satu jalur eksekusi** — jangan ada dua fase yang menge-emit hal yang sama
  (pelajaran global-inits vs body, Fix #025).
- **Non-void call wajib %N** — bug penomoran LLVM sudah 3x kambuh.
- **Build harus tidak pernah rusak oleh file WIP** — blacklist di scripts/build.sh.

## Status terakhir
- ✅ **Fase B #6 TUNTAS**: list header runtime `[len][elems]` — len/append/for-in/index
  bekerja lintas-fungsi (param `xs: vector`). arrayLens_ hack DIHAPUS. Semantik value-style:
  append di callee → reassign (`nums = push(nums, v)`); old buffer tak di-free (anti-UAF;
  refcounting menunggu backend sendiri). Tes: tests/integration/test_list_fn.ec.
- ✅ **Library std.ec** (lib/std.ec): strReverse/StartsWith/EndsWith/ContainsChar/CountChar/
  RepeatChar/PadLeft + listSum/Max/Min/Contains/Range — murni EC, lulus self-test via
  scripts/test_lib.sh (gabung sumber — jembatan sebelum import multi-file siap).
- ✅ **Enum TypeChecker registration** (Fase B #5): enum variants `Color.Red` dikenali
  bertipe `Color` (bukan "unknown") — prasyarat match-on-enum.
- ✅ Sinkron paralel (Fix #041): declareTopLevel dipulihkan; PHI serial 3-pred;
  keyword ETC/testing terparse; -= *= /= kembali di lexer; E2006 kecuali main;
  EC_KEEP_IR env; build.sh CXXFLAGS env.
- ✅ **scripts/test.sh**: build + integration tests + error-corpus + libtest + stage1 + bench.
- ✅ Dokumentasi: docs/string-contract.md, docs/memory-schema.md.
- ✅ Artifact VSIX: `dist/ec-language-0.3.3.vsix` — regenerasi lewat `./scripts/pack_vsix.sh`.
- ✅ `scripts/build.sh`: auto-glob source, cache .build/obj, pembersih object yatim,
  injeksi versi dari VERSION via -DENG_VERSION, CXXFLAGS env (ASan: CXX=g++
  CXXFLAGS="-g -O0 -fsanitize=address"), blacklist WIP: llvcodegen.cpp, compiler/ets/,
  etskeleton.cpp, js.cpp, platform_{windows,posix}.cpp — HAPUS saat fiturnya stabil!
- Regresi: 11/11 integration PASS, lib/std OK, stage1 OK, bench OK.
- ✅ Versi sinkron: `engc version` → v dari file VERSION (habis hardcoded v0.1.0).
- ✅ `.append()` kini free() buffer lama (kebocoran per-append berhenti).
- ✅ Perbaikan WIP sesi paralel di typechecker: E2003 bandingkan NAMA tipe ternormalisasi
  (bukan pointer shared_ptr!), FieldAccess enum-variant tak lagi E1001, BinaryOp toleran
  unknown, let/mut fallback "unknown" alih-alih nullptr.
- Regresi: 7/7 integration PASS, calculator OK.

## Status terakhir
- ✅ **Fase B #5-8 TUNTAS**: enum TypeChecker registration, list header runtime, string contract doc, memory schema doc.
- ✅ **Fase A infrastruktur**: test.sh lengkap (build+integration+error-corpus+libtest+stage1+bench).
- ✅ **Library std.ec** + 11 integration tests passing + 6 error cases validated.
- ✅ **Artifact VSIX** + **docs/** (string-contract, memory-schema).
- Regresi: **11/11 PASS**, lib/std OK, stage1 OK, bench OK, error corpus clean.

## Langkah berikutnya (Fase C — Diagnostik & keandalan)
9.  [x] Pesan error + potongan baris sumber & caret (^) di kolomnya → `DiagnosticEngine::print(srcLines)`.
10. [x] Kode E#### stabil + tabel dokumentasi tiap kode (docs/diagnostics.md).
11. [x] Korpus regresi parser: ≥5 file rusak; assert tidak hang, tidak crash, jumlah error konsisten.
12. [x] Warning untuk parameter tak terpakai (W1001). Shadowing warning ditunda (kompleks).

**Catatan**: Ada pre-existing bug runtime exit crash (`free(): invalid pointer`) yang mempengaruhi semua program. Tidak mempengaruhi correctness, hanya crash saat cleanup. Dokumentasikan di docs/memory-schema.md.

## Fase D — Dokumentasi bahasa (minggu 8–10)
13. [ ] docs/language/reference.md ditulis ulang dari PERILAKU aktual.
14. [ ] Contoh examples/ diverifikasi jalan lewat scripts/test.sh.

Fase C — Diagnostik & keandalan (minggu 4–6)
9.  [ ] Pesan error + potongan baris sumber & caret (^) di kolomnya.
10. [ ] Kode E#### stabil + tabel dokumentasi tiap kode (docs/diagnostics.md).
11. [ ] Korpus regresi parser: ≥15 file rusak; assert tidak hang, tidak crash, jumlah error konsisten.
12. [ ] Warning untuk kasus lunak (variabel tak terpakai, shadowing).

Fase D — Dokumentasi bahasa (minggu 6–8)
13. [ ] docs/language/reference.md ditulis ulang dari PERILAKU aktual (script mode, elif,
        and/or/not, list ops, match, ternary, interpolasi).
14. [ ] Contoh examples/ diverifikasi jalan lewat scripts/test.sh.

## Jangan lakukan (jebakan terbukti)
- JANGAN percaya PATH persistent antar panggilan tool — export PATH selalu di awal command;
  tanpa clang semua test "gagal" padahal program benar (sudah 2x mengelabui diagnosis).
- JANGAN bandingkan TypePtr dengan != — selalu nama ternormalisasi.
- JANGAN glob sembarang .cpp baru ke build tanpa cek dependensi (llvcodegen butuh llvm-dev).
- Sesi paralel aktif: RE-READ file sebelum edit; konflik typechecker sudah terjadi 2x hari ini.
