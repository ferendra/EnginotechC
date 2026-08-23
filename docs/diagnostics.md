# EngineC++ Diagnostic Codes

Stabil codes yang digunakan oleh compiler. Setiap code memiliki format:
- **Kode**: `E####` (Error), `W####` (Warning), `L####` (Lexer)
- **Ringkasan**: Penyebab + contoh situasi
- **Fix**: Langkah perbaikan

---

## Lexer Errors (L0001–L0099)

### L0001 — Invalid character
**Penyebab**: Karakter tidak dikenal di source (biasanya typo atau encoding issue).
**Contoh**: `let x = 4€;`
**Fix**: Hapus karakter invalid atau ganti dengan Unicode yang valid.

---

## Parser Errors (E0001–E0999)

### E0001 — Expected token / Unexpected token
**Penyebab**: Parser menemukan token yang tidak diharapkan pada posisi tersebut.
**Contoh**:
```ec
let x = ;    // Expected expression after =
if foo {     // Expected '(' after 'if'
```
**Fix**: Periksa syntax — biasanya ada token yang hilang (`,`, `(`, `)`, `;`).

### E0002 — Missing closing delimiter
**Penyebab**: Kurung kurawal `}`, parenthesis `)`, atau bracket `]` tidak ditutup.
**Contoh**:
```ec
fn main() {
    print("hello")   // Missing }
```
**Fix**: Tambahkan closing delimiter yang sesuai.

---

## Semantic Errors (E1001–E1999)

### E1001 — Undefined variable
**Penyebab**: Variabel digunakan sebelum dideklarasikan atau typo nama.
**Contoh**:
```ec
fn main() {
    print(foo);  // foo belum didefinisikan
}
```
**Fix**: Pastikan variabel dideklarasikan sebelum digunakan.

### E1002 — Undefined function
**Penyebab**: Fungsi dipanggil tapi belum didefinisikan.
**Contoh**:
```ec
fn main() {
    let x = undefined_func();
}
```
**Fix**: Definisi fungsi harus ada sebelum panggilan, atau gunakan forward declaration.

---

## Type Checker Errors (E2001–E2999)

### E2001 — Type mismatch in arithmetic
**Penyebab**: Operasi arithmetic dilakukan pada tipe yang tidak kompatibel.
**Contoh**:
```ec
let result = "hello" + 42;  // string + int
```
**Fix**: Konversi tipe dulu: `str(42)` atau gunakan variable bertipe sesuai.

### E2003 — Type mismatch in assignment
**Penyebab**: Tipe nilai tidak cocok dengan deklarasi variable.
**Contoh**:
```ec
let s: string = 42;  // int tidak cocok dengan string
```
**Fix**: Sesuaikan tipe variable dengan nilai yang diassign.

### E2006 — Missing explicit return type
**Penyebab**: Fungsi non-main tidak memiliki anotasi return type eksplisit.
**Contoh**:
```ec
fn compute() {  // Missing -> int
    return 42;
}
```
**Fix**: Tambahkan return type: `fn compute() -> int`.

---

## Code Generation Errors (CG001–CG099)

### CG001 — LLVM IR generation failed
**Penyebab**: Internal compiler error saat generate LLVM IR.
**Fix**: Laporkan bug dengan contoh kode minimal yang memicu error.

### CG002 — Failed to compile LLVM IR
**Penyebab**: Compiler clang gagal mengompilasi IR yang dihasilkan.
**Fix**: Periksa error output clang untuk detail lebih lanjut.

---

## Notes

- Error codes bersifat **stabil** — tidak akan berubah tanpa versi mayor.
- Untuk debugging lanjutan, gunakan flag `EC_KEEP_IR=1` untuk menyimpan IR intermediate.
- Source line + caret akan ditampilkan jika file sumber tersedia.
