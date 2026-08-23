# Skema Memori EngineC++ — Dokumentasi Perilaku Nyata

> Status: **SPESIFIKASI AKTUAL** (bersumber dari perilaku compiler v0.2.x).
> Diperbarui: 2026-08-22.

## Ringkasan Arsitektur Memori
Compiler saat ini **TIDAK memiliki garbage collector**. Memory management bersifat:
1. **Stack allocation** untuk variabel lokal sederhana
2. **Global allocation** untuk string literal dan data konstan
3. **Heap allocation manual** untuk array/list dan hasil string concatenation

## Skema Alokasi

### 1. Stack Variables (`%var = alloca i32`)
- Variabel lokal (`let`, `mut`) dialokasikan di stack via `alloca`.
- Lifetime: sepanjang function scope.
- TIDAK ada automatic cleanup — nilai di-stack tetap ada hingga function return.
- **Kelemahan**: variabel yang tidak di-init bisa berisi garbage value.

### 2. Global Constants
- String literal disimpan di global `@.str.N = constant [N x i8]`.
- Enum constants disimpan sebagai global `@enum_...`.
- Lifetime: seluruh program.
- Read-only, TIDAK bisa dimodifikasi.

### 3. Heap Allocation
#### A. Array/List
- `let arr: vector = [...]` → mengalokasikan buffer heap dengan header `[len][elems]`.
- Header 8 byte pertama menyimpan length (i64).
- **Ownership**: pemilik buffer adalah variable yang mengacu padanya.
- **Tidak ada free otomatis** — leak jika variable tidak di-reassign ke buffer lain.

#### B. String Concatenation
- `a + b` → `malloc(strlen(a)+strlen(b)+1)` + `memcpy`.
- **Tidak ada free otomatis** — buffer baru bocor kecuali di-reassign ke variable yang baru.

### 4. Memory Safety

#### Use-After-Free (UAF)
- **TIDAK TERLINDUNGI** — compiler TIDAK melacak ownership.
- Jika pointer string/array dipakai setelah buffer di-free (misalvia manual free), behavior undefined.
- **Rekomendasi**: hindari manual memory management untuk saat ini.

#### Buffer Overflow
- **TIDAK TERLINDUNGI** — tidak ada bounds checking pada array indexing atau string operation.
- `arr[i]` dengan `i >= len(arr)` akan read/write sembarangan di memory.

## Rekomendasi Pengguna

1. **Hindari manual memory management** — belum ada RAII atau destructor.
2. **Gunakan value semantics** — assign ulang variable alih-alih mutate in-place.
3. **Batasi lifetime array/string panjang** — gunakan scope block `{ ... }` untuk membatasi lifetime conceptual.
4. **Jangan andalkan stack memory untuk data jangka panjang** — stack bisa reused setelah function return.

## Rencana Jangka Panjang

### Short-term (Fase berikutnya)
- Implementasi **arena allocator** per function scope untuk mengurangi fragmentation.
- **Reference counting** untuk string dan array (saat ini ada rencana tapi belum implementasi).

### Long-term
- **Garbage collector** mark-sweep atau reference counting otomatis.
- **RAII** untuk resource management (file handles, memory buffers).
- **Ownership type system** seperti Rust (saat ini hanya type checking statis).

## Referensi Implementasi
- `compiler/codegen/irgen.cpp`: `emitArrayLit`, `emitListAppend`, `emitStringConcat`
- Runtime: `eng_malloc`, `eng_free` (jika ada), `eng_str_concat`

## Known Issues

### Runtime Exit Cleanup Crash
- **Gejala**: Program berjalan benar, tapi crash dengan `free(): invalid pointer` saat exit
- **Root cause**: Ada buffer yang di-free secara ganda atau invalid pointer di cleanup phase
- **Status**: Pre-existing, belum diperbaiki — tidak mempengaruhi correctness
- **Workaround**: Ignore stderr output crash, program tetap valid
