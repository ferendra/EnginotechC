# Fitur Arduino: transpiler EC → sketch .ino
Status: BERJALAN · Service: compiler/targets + driver embedded · Diperbarui: 2026-08-22 11:00

## Sedang dikerjakan
Transpiler `engc build --target arduino` — SUDAH di-wire oleh sesi paralel:
main.cpp:148 memanggil `arduinogen::generateSketch(prog, board)` untuk target embedded.
Implementasi terpecah 3 file: `compiler/targets/{sketch_builder,stmt_emitter,expr_emitter}.cpp`
(arduinogen.cpp kini stub; JANGAN hapus headernya — main.cpp include itu).
Build command kini HARUS menyertakan ketiga file .cpp tersebut.

## Status terakhir
- Fix #024 & #025 selesai (recovery multi-error; fitur Python-style) — CHANGELOG_FIXES.md.
- Build lengkap sukses dengan sketch_builder+stmt_emitter+expr_emitter.
- Belum diverifikasi end-to-end: apakah `engc build --target arduino` benar menghasilkan .ino valid.

## Keputusan penting
- Verifikasi TANPA hardware: compile .ino hasil generate untuk HOST dengan -I misc/arduino-sim,
  jalankan dengan timeout. avr-gcc tidak ada di mesin ini.

## Langkah berikutnya
1. [ ] Uji `engc build examples/embedded/arduino-blink/main.ec --target arduino` → periksa .ino.
2. [ ] Skrip tes `tests/embedded/test_host_sim.sh`: generate blink → zig c++ -I misc/arduino-sim
       + sim_main.cpp → jalankan → assert "digitalWrite(13," muncul ≥2x dan exit 0.
3. [ ] Pastikan emitter tahan fitur baru Fix #025 (elif/and/or/not/len/append/for-in-list) bila
       dipakai di sketch — expr_emitter kemungkinan belum kenal AND/OR word alias & list ops.
4. [ ] Update docs/language/reference.md bagian embedded.

## Jangan lakukan (jebakan)
- JANGAN jadikan "loop" keyword lexer — cukup special-case parser.
- Emitter: CallExpr callee FieldAccess = method call; Ident polos = fungsi bebas.
- RangeExpr.end EXCLUSIVE kecuali flag inclusive=true.
- Array EC = buffer heap stride 8 byte (lihat Fix #025) — emitter C++ Arduino bebas pakai
  std::vector, tidak perlu meniru layout LLVM.
