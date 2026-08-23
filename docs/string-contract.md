# Kontrak String EngineC++ — Dokumentasi Perilaku Nyata

> Status: **SPESIFIKASI AKTUAL** (bersumber dari perilaku compiler v0.2.x, bukan aspirasi).
> Diperbarui: 2026-08-22.

## Ringkasan Eksekusi
- String direpresentasikan sebagai **NUL-terminated C-string** (`ptr` i8) dalam LLVM IR.
- Operan string di-pass sebagai pointer, TIDAK ada value semantics.
- Semua string literal disimpan di global constant pool, diakses via `@<name>` atau `.str.N`.

## Kontrak Operasional

### 1. String Literal
- Sintaks: `"..."` atau `"..."` (escape sequence didukung).
- Ditampung di global `@.str.N = private unnamed_addr constant [N x i8] c"..."`.
- Pointer literal adalah const, TIDAK boleh dimodifikasi.

### 2. String Concatenation (`+`)
- **Tidak mengubah operand**. Alih-alih, mengalokasikan buffer baru via `malloc(strlen(a)+strlen(b)+1)`, menyalin kedua input, dan mengembalikan pointer baru.
- **Buffer hasil TIDAK di-free secara otomatis** — lifecycle string hasil concat tunduk pada ownership yang tidak terdokumentasi dengan jelas. Saat ini, tidak ada mekanisme explicit free untuk string hasil operasi.
- **Leak potensial**: string hasil concat di variable `let` akan bocor sampai program berakhir (atau sampai engine menambahkan arena refcounting).

### 3. String Assignment
- `let s = other_string;` menyalin pointer, TIDAK mengalokasikan.
- `s = other_string;` di mut var sama — pointer copy.
- String literal assignment juga pointer copy.

### 4. String Indexing
- Operator `[]` pada string **TIDAK DIDUKUNG** — akan error typechecker.
- Access byte-wise harus menggunakan fungsi runtime: `str_at(str, idx)`.

### 5. String Length
- `len(str)` → mengembalikan `i64` panjang string (termasuk NUL terminator, tapi value yang dikembalikan adalah strlen tanpa NUL).
- Runtime: `strlen(ptr)` dipanggil setiap kali.

### 6. String Interpolation
- `"hello ${name}!"` → desugared ke `eng_str_concat` chain.
- Setiap sub-ekspresi dikonversi ke string dulu (via `str(int)`, dll).

## Batasan & Known Issues

### Memory Leak String Concat
- Setiap `a + b` mengaliokasikan buffer baru via `malloc` internal runtime.
- Buffer lama TIDAK di-free oleh runtime saat ini (kecuali literal).
- Rekomendasi: gunakan variable `let` immutable untuk hasil concat, hindari reassign berulang di loop.

### Tidak Ada Slice/Substring
- Tidak ada operasi substring native.
- Workaround: gunakan loop manual atau tunggu stdlib slice function.

### Tidak Ada Escape Sequence Unik
- Escape sequence didukung (`\n`, `\t`, `\"`, `\\`), tapi tidak ada Unicode/UTF-8 handling khusus.

## Rekomendasi Pengguna

1. **Hindari loop dengan string concatenation berulang** — setiap iterasi mengalokasikan buffer baru.
2. **Gunakan `let` untuk string hasil concat** — mutasi tidak relevan karena string immutable secara semantik.
3. **Untuk operasi kompleks**, tunggu library `std` yang menyediakan `str_replace`, `str_slice`, dll.

## Referensi Implementasi
- `compiler/codegen/irgen.cpp`: `emitStringConcat`, `emitStringLiteral`
- Runtime strings: `eng_str_concat` (define ptr @eng_str_concat(ptr %a, ptr %b))
