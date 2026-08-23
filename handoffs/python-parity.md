# Python-parity: bahasa EC se-ergonomis Python
Status: DITUNDA (fokus fondasi 1–2 bulan — lihat handoffs/fondasi-roadmap.md) · Service: compiler · Diperbarui: 2026-08-22 12:00

## Sedang dikerjakan
Membuat EC "bersanding dengan Python": script tanpa boilerplate, koleksi dinamis,
operator kata, string methods. Target pengguna: pemula & scripting cepat, tapi hasil
kompilasi native cepat.

## Status terakhir (Fix #025 + #026 — 7/7 test PASS)
- Script mode: statement top-level = implicit main() tanpa `fn main()`.
- `elif`, `and`, `or`, `not` (alias token AND/OR/NOT).
- List: literal `[...]`, indexing stride-8 diperbaiki, `arr[i] = v`, `.append(v)`,
  `len(list)` & `len(str)`, `for x in list`.
- String interpolation `${expr}` bekerja di assignment global & lokal.
- **Percabangan lengkap**: if/elif/else, match-case (string/int/block/`_`), ternary
  Python `nilai if kondisi else nilai_lain` (nested-kanan ok, LLVM select, guard baris).

## Keputusan penting
- List = heap buffer malloc stride 8 byte; panjang dilacak compile-time di
  `arrayLens_` (irgen). Keterbatasan: list yang di-pass ke fungsi kehilangan
  pelacakan panjang → len/append/for-in di paramater function BELUM didukung.
- Script mode: lets top-level jadi global @gv.*; emitGlobalInits DINONAKTIFKAN untuk
  implicit main agar urutan sumber dihormati (flag scriptMode_).
- `tryParseAssignment()` menangani x=, obj.f=, arr[i]= di blok DAN top-level.

## Langkah berikutnya (urutan dampak tertinggi)
1. [ ] String methods: `"abc".upper()/.lower()/.contains()/.split()/.trim()`
       (runtime helper eng_* sudah ada eng_case_map — tinggal wiring method-call).
2. [ ] Dict literal `{"a": 1}`, indexing by key, `in` operator membership.
3. [ ] Tuple/multiple assignment `a, b = b, a+b`.
4. [ ] Default parameter + keyword arg di fn.
5. [ ] List lintas-fungsi: simpan panjang di header alokasi (8 byte pertama) agar
       len/append/for-in bekerja untuk parameter function.
6. [ ] `print` tanpa tipe-seperti-Python untuk float (5.0 bukan 5.000000)?
7. [ ] Update docs/language/reference.md + ROADMAP.md + contoh examples/pythonic/.

## Jangan lakukan (jebakan)
- JANGAN jalankan codegen dua kali (Fix #025: panggilan compileToIR dibuang sudah DIHAPUS).
- emitGlobalInits tidak boleh aktif di script main (urutan let rusak) — flag scriptMode_.
- memcpy/malloc/sprintf non-void call WAJIB dapat nomor register (%N = call ...) —
  bug penomoran LLVM sudah 3x kambuh (assert-print, append, dsb.).
- Slot field bernama `vt` BUKAN `type`; deklarasi method pakai Slot& harus di bawah
  definisi struct Slot di irgen.h.
- Ternary tanpa guard baris akan menelan statement `if` di baris berikutnya —
  cek tokens_[pos_-1].line == current().line (Fix #026).
- Sebelum lapor bug compiler: cek dulu ekspektasi tes sendiri (kasus tier "mid" vs "high").
