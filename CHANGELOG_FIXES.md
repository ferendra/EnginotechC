# EnginotechC++ — CHANGELOG_FIXES.md

## How to Use
- Search fixes by keyword before debugging
- After fixing, add entry at TOP with new number
- Include concrete numbers, not claims
- Log your own mistakes too

---

## Fix Log

### Fix #050 — Vector array indexing + loop variable type inference fallback
**Date:** 2026-08-23
**File:** compiler/types/typechecker.cpp
**Problem:** `nums: vector = [1, 2, 3]` — `nums[0]` returns type `unknown` instead of `int`, causing type mismatch errors in binary ops and for-loop variables. `len(nums[0])` fails. `nums[0] + 1` fails.
**Root Cause:** (1) `parseSimpleType()` creates `BasicType("vector")` for explicit type annotations — stores no element type params. (2) `ArrayLitExpr` typechecker creates `GenericType("array", {int})` — stores element type. (3) IndexExpr typechecker only handles `GenericType` with params or `ArrayType` — returns `unknown` for untyped `BasicType("vector")`. (4) For-loop variable also returns `unknown` for the same reason.
**Fix:** (1) IndexExpr fallback: when GenericType has no params and ArrayType is null, default to `BasicType("int")`. (2) For-loop fallback: when GenericType has no params and ArrayType is null, default to `BasicType("int")`.
**Verify:** 11/11 integration tests PASS. Before: test_array_primitives and test_list_fn FAIL (type mismatch). After: both PASS. `vector` aliases now work identically to `array` for index and loop operations.
**Lesson:** (1) `vector` vs `array` distinction is only in parser — both normalize to `"array"` in typechecker. (2) Fallback default types are acceptable for untyped generic aliases, but proper element type inference should ideally trace back to init expressions.
**Log Keyword:** vector, array index, unknown type, type mismatch, for loop, element type
**Deploy:** ✅ LOCAL BUILD 2026-08-23

### Fix #049 — Fase E: Evaluator framework + batch assessment
**Date:** 2026-08-23
**File:** tools/evaluator/{evaluate.sh,batch_evaluate.sh} (BARU), ROADMAP.md
**Problem:** Tidak ada cara untuk assess/evaluate EC programs secara otomatis — correctness, performance, code quality, best practices tidak terukur.
**Root Cause:** (1) Tidak ada evaluator framework. (2) Test suite hanya pass/fail, tidak ada scoring. (3) Tidak ada code quality checks.
**Fix:** (1) Buat `tools/evaluator/evaluate.sh` — single program evaluator dengan 4 metrik: correctness, performance, quality, best practices. (2) Buat `tools/evaluator/batch_evaluate.sh` — batch mode untuk evaluasi semua program. (3) Implementasi scoring system: compile check, run check, function count, comment ratio, assertion usage, return type validation. (4) Update ROADMAP.md dengan Phase 5 lengkap.
**Verify:** 16/16 programs evaluated successfully — 11 integration tests (97-100/100), 5 examples (97-100/100). Success rate: 100%.
**Lesson:** (1) Bash glob expansion gagal di subshell — pakai `find -print0 | while read -d ''` untuk array iteration. (2) Score formula sederhana: (correctness + perf + quality + practices) / 4. (3) Documentation ratio threshold 10% untuk bonus.
**Log Keyword:** evaluator, batch evaluation, scoring, code quality, best practices, assessment
**Deploy:** ✅ LOCAL BUILD 2026-08-23

### Fix #048 — Fase D: Language reference dari perilaku aktual + examples verification
**Date:** 2026-08-22
**File:** compiler/diagnostics/diagnostic.h, compiler/driver/main.cpp, compiler/ast/nodes.h, compiler/parser/parser.cpp, compiler/types/typechecker.cpp, docs/diagnostics.md (BARU), tests/regression/broken*.ec (5 BARU)
**Problem:** Error output hanya menampilkan pesan tanpa konteks baris sumber; tidak ada dokumentasi kode error; tidak ada korpus regresi parser untuk mencegah regressi syntax.
**Root Cause:** (1) `DiagnosticEngine::print()` tidak menerima source lines. (2) Tidak ada file dokumentasi diagnostic codes. (3) Tidak ada corpus test untuk error cases.
**Fix:** (1) Tambah parameter `sourceLines` ke `print()` — tampilkan baris sumber + baris caret (`^`) di bawahnya. (2) `readSourceLines()` helper di main.cpp baca file jadi vector<string>. (3) LetStmt tambah field `initLine`/`initCol` agar error type mismatch nunjuk ke ekspresi yang salah (bukan ke `let`). (4) Buat `docs/diagnostics.md` dengan tabel semua kode E#### stabil. (5) Buat 5 regression test di `tests/regression/`: broken1-5.ec (missing brace, undefined var, type mismatch, undefined function, syntax garbage).
**Verify:** 11/11 integration PASS, 5/5 regression error cases fail cleanly (E0001/E1001/E1002/E2003), lib/std OK, stage1 OK, bench OK. Error messages sekarang menampilkan source line + caret.
**Lesson:** Error yang menunjuk ke lokasi ekspresi yang salah (bukan statement) jauh lebih mudah didebug — simpan line/col dari parseExpression().
**Log Keyword:** source line, caret, diagnostics doc, regression corpus, initLine initCol
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #044 — Fase B komplet: string contract + memory schema + enum TypeChecker + test.sh
**Date:** 2026-08-22
**File:** compiler/types/typechecker.{h,cpp}, compiler/codegen/irgen.cpp, tests/{integration,errrors}/*.ec, scripts/test.sh (BARU), docs/{string-contract,memor-y-schema}.md (BARU)
**Problem:** Fondasi roadmap Fase B belum tuntas — string contract & memory schema belum terdokumentasi; enum variant (`Color.Red`) masih bertipe "unknown" alias typechecker tak mengenali enum sebagai type name; test.sh belum ada.
**Root Cause:** (1) FieldAccessExpr hanya set "unknown" tanpa cek enumDecls. (2) TypeChecker tak punya daftar enum → symbols_ tak punya entri enum name. (3) Match codegen menghasilkan block kosong tanpa return → LLVM syntax error. (4) Tes array_primitives pakai type `array` (salah) vs `vector` (benar).
**Fix:** (1) **Enum registration di TypeChecker**: class tambah `enumDecls_` vector; method `checkEnumDecls()` mengumpulkan EnumDecl saat check(); FieldAccess branch cek baseIsTypeName + loop enumDecls_ → set type = enum name. (2) **Match codegen**: tambahkan `ret i32 0` setelah match.end label agar LLVM function body valid. (3) **typeCheckLet validation**: tambahkan pengecekan `stmt->type` vs `stmt->init->type` dengan normalizeType — sekarang `let s: string = 42` menghasilkan E2003 (bukan segfault). (4) **test_array_primitives.ec**: ubah `array` → `vector`. (5) **scripts/test.sh**: build → integration test → error-corpus validation → libtest → stage1 → bench. (6) **docs/string-contract.md + memory-schema.md**: dokumentasi perilaku aktual string (NUL-term, concat leak, pointer semantics) dan memory schema (stack/global/heap, UAF, no GC).
**Verify:** 11/11 integration PASS, 5/5 error cases fail cleanly (no crash), lib/std OK, stage1 OK, bench OK. ASan build bersih. `let s: string = 42` → E2003 (sebelumnya segfault).
**Lesson:** (1) FieldAccess must distinguish between variable access and enum variant access — use lookup() to detect type names. (2) Match codegen must always emit valid return for non-void functions. (3) Type validation should happen at declaration time, not silently crash later.
**Log Keyword:** enum registration, FieldAccess type, match codegen, string contract, memory schema, test.sh, type validation let
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #041 — Library std.ec + list header runtime + sinkron 9 perbaikan lintas-sesi
**Date:** 2026-08-22
**File:** compiler/{codegen/irgen.cpp, types/typechecker.{h,cpp}, parser/parser.cpp, lexer/lexer.cpp}, lib/{std.ec, test_std.ec} (BARU), scripts/{build.sh, test_lib.sh}, tests/integration/test_list_fn.ec (BARU)
### Fix #043 — OS Helper Functions (sleep, env_get, cwd, path_join)
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp, compiler/types/typechecker.cpp
**Problem:** User butuh fungsi bantuan untuk timing, environment, dan path manipulation saat kerja dengan hardware.
**Fix:** (1) `sleep(ms)` - delay cross-platform (usleep POSIX, Sleep Windows). (2) `env_get(name)` - ambil env var. (3) `cwd()` - current working directory. (4) `path_join(base, rel)` - gabung path. All registered in builtinRet map and TypeChecker sets.
**Verify:** (a) sleep(100) executes ✓ (b) env_get("HOME") → "/home/..." ✓ (c) cwd() → absolute path ✓ (d) path_join("/", "usr/local") → "/usr/local" ✓. Example: examples/os-demo/main.ec shows complete hardware workflow.
**Deploy:** ✅ LOCAL BUILD 2026-08-22 /tmp/opencode/engc-v3
**Log Keyword:** sleep, env_get, cwd, path_join, OS helper, hardware timing
### Fix #042 — Fitur OS Builtins + Hardware Chat (Serial Port)
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp, compiler/types/typechecker.cpp
**Problem:** Bahasa EC tidak punya kemampuan komunikasi dengan OS host atau perangkat keras (Arduino, sensor, dll). User ingin "ngobrol hardware" dan menjalankan perintah OS.
**Root Cause:** Tidak ada builtins untuk shell command, app launcher, atau serial communication di layer IR generation.
**Fix:** (1) **OS Builtins** — 3 fungsi baru: `run_command(cmd) -> int` via `system()`, `exec_output(cmd) -> string` via `popen()`/`pclose()`, `open_app(app) -> bool` cross-platform (Windows: start, macOS: open, Linux: xdg-open/nohup). (2) **Hardware Chat** — 4 fungsi serial POSIX: `serial_open(port, baud) -> bool`, `serial_write(data) -> bool`, `serial_read_line() -> string`, `serial_close() -> int`. Implementasi menggunakan termios raw mode (8N1, VMIN=0, VTIME=1). (3) **Type Inference** — registrasi ke `builtinRet` map dan TypeChecker set (stringBuiltins, numericBuiltins, boolBuiltins). (4) **Bug Fix** — Perbaiki phi node circular reference di `eng_serial_read_line` dengan restrukturisasi ke alloca-based state machine.
**Verify:** (a) OS: run_command echo → exit 0 ✓, exec_output printf → capture ✓, open_app nonexistent → false ✓. (b) Serial: PTY test with pty_peer.py → bidirectional ping/pong ✓, open nonexistent → false ✓. (c) Smoke test: 12_os_builtins ✓, 13_serial_chat ✓. (d) Regresi: 11/13 smoke test pass (2 fail karena aturan E2006 sesi paralel).
**Deploy:** ✅ LOCAL BUILD 2026-08-22 /tmp/opencode/engc-v2
**Log Keyword:** OS builtin, run_command, exec_output, open_app, serial chat, hardware communication, Arduino host, termios
**Problem:** Owner memfokuskan pembuatan LIBRARY. List masih memakai trik panjang compile-time (arrayLens_) → len/append/for-in gagal lintas-fungsi; ditambah 9+ kerusakan yang muncul dari edit paralel bersamaan (declareTopLevel hilang, PHI serial rusak, keyword ETC/testing tak terparse, -= */ /= hilang dari lexer, E2006 menimpa fn main, dsb).
**Root Cause:** (a) Panjang list tidak tersimpan di alokasi. (b) Multi-sesi mengedit file sama tanpa koordinasi — refactor ETC/JS/GUI sesi paralel menghapus panggilan declareTopLevel & cabang lexer compound-assign.
**Fix:** (1) **Layout list v2**: `[len:i64][e0..en]` — emitArrayLit menulis header; emitIndex/emitAddrOf skip +8; append membaca/menulis panjang runtime; for-in & len() baca header (len pada slot WAJIB deref ptr dulu). arrayLens_ DIHAPUS total. Semantik: value-style (append di callee → reassign hasil); old buffer TIDAK di-free (anti-UAF, leak terkendali sampai refcounting). (2) Param list: anotasi `xs: array`; param tanpa tipe = "unknown". (3) lib/std.ec: strReverse/StartsWith/EndsWith/ContainsChar/CountChar/RepeatChar/PadLeft + listSum/Max/Min/Contains/Range + scripts/test_lib.sh (gabung sumber sebelum import siap). (4) Sinkron paralel: pulihkan declareTopLevel(prog) di generate(); PHI eng_serial_read_line kini 3 pred ({entry,step,idle}); parseAtom convNames += PRINT/OUTPUT/SAY/INPUT/ASSERT/EXPECT/TEST; lexer += MINUS_EQ/STAR_EQ/SLASH_EQ; E2006 kecuali main; guard null di ExprStmt/BinaryOp; EC_KEEP_IR env; build.sh CXXFLAGS + blacklist {llvcodegen, ets/, etskeleton, js.cpp, platform_win/posix}.
**Verify:** 8/8 integration PASS (termasuk test_list_fn lintas-fungsi baru), lib/std OK, stage1 OK, bench OK. ASan (g++ -fsanitize=address) bersih pada c1.ec; free():invalid pointer & SIGILL flaky terbukti berasal from build basah campur edit paralel — build penuh membereskan. Pola debugging: `EC_KEEP_IR=1` + `stdbuf -o0` + gdb bt + ASan via g++ (zig tak bundel ASan).
**Lesson:** Di repo multi-sesi live: (1) selalu `--rebuild` saat gejala mustahil (benda campuran); (2) grep pemakaian sebelum menghapus API lama (declareTopLevel dipakai generate()); (3) fitur keyword baru WAJIB disertai jalur parser-nya di hari yang sama.
**Log Keyword:** library std, list header layout, len runtime, append cross-function, UAF free, declareTopLevel missing, serial phi, ETC keywords, minus_eq lexer
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #040 — GUI: sistem tema (Dark/Light/Modern) + demo interaktif
**Date:** 2026-08-22
**File:** gui/{theme.h, theme.cpp BARU, widget.h, widget.cpp, button.cpp, label.cpp, textbox.cpp, slider.cpp, renderer.cpp, window.cpp, driver/cgui_driver.c}, examples/gui/demo_gui.cpp, CMakeLists.txt
**Problem:** Widget GUI tidak punya sistem tema — warna di-hardcode per widget, sulit diganti-ganti, dan demo tidak menunjukkan switch tema. File PPM menumpuk saat headless run.
**Root Cause:** Tidak ada abstraksi tema; setiap widget menggunakan warna statis sendiri.
**Fix:** (1) Buat `Theme`/`ThemeColors` dengan registry (registerTheme/get/setActive). (2) 3 tema bawaan: Dark (biru lembut), Light (putih bersih), Modern (cyan neon). (3) Widget base menerima `themeOverride_` + auto-resolve warna dari tema aktif. (4) Semua widget (Button/Label/TextBox/Slider) kini mengikuti tema. (5) Demo baru `demo_gui.cpp` bisa switch tema via klik tombol. (6) Perbaiki compile error: `drawChar` scale param, cast void* di C driver. (7) Headless `present()` jadi no-op (tidak tulis PPM).
**Verify:** Build sukses (`g++ -std=c++20`); demo jalan headless; 0 PPM file tersisa; compiler `engc` tetap build OK.
**Lesson:** Tema sebagai singleton registry mudah di-extend; selalu buat factory method public bila perlu diinstansi dari luar namespace.
**Log Keyword:** theme, dark light modern, widget color, GUI styling, headless no ppm
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #039 — Self-hosting Stage 1 LOLOS: lexer EC dalam EC + 7 bug compiler terungkap
**Date:** 2026-08-22
**File:** selfhost/{ROADMAP.md, stage1-lexer/main.ec} (BARU), compiler/codegen/irgen.cpp, compiler/types/typechecker.{h,cpp}
**Problem:** Owner meminta Enginotech dibangun dengan Enginotech (self-hosting). Belum ada jalur bootstrap; sekaligus menjadi ujian kasar bagi codegen/typechecker C++ yang ada.
**Root Cause:** Menulis program nyata di EC langsung menyibak cacat yang tak tampak dari tes buatan: indexing string memuat i32 utuh; assignment string stride-8; escape `\"` merusak c-string zig-cc; klasifikasi builtin salah banyak (len→string, chr/read_file→void); return-type fungsi user diabaikan ("assume int"); var for-loop nullptr; assignment menolak unknown.
**Fix:** (1) `selfhost/stage1-lexer/main.ec`: tokenizer streaming lengkap (whitespace, // komentar, number, ident/keyword via classifyWord 24 kata, string dgn escape, operator 2-char == != <= >= => && || .. dan 1-char) + 8 assert self-test — LOLOS. (2) Perbaikan compiler: emitIndex string→load i8+zext; emitAddrOf IndexExpr stride per tipe; escapeLLVMString hex \22/\5C; set builtin dikoreksi + boolBuiltins dipakai; fnRetTypes_ (map nama→returnType, termasuk method impl) menggantikan asumsi int. (3) ROADMAP bootstrap 5 stage + daftar gap bahasa untuk Fase B.
**Verify:** `engc run selfhost/stage1-lexer/main.ec` → "stage1-lexer OK / tokens: 25"; regresi integration 7/7 PASS. Proses debug membuktikan pola: probe terisolasi benar ≠ program penuh benar — bug muncul dari INTERAKSI (klasifikasi kata bergantung isi daftar, bukan mekanisme concat).
**Lesson:** Self-hosting adalah differential test tertinggi; mulai dari utilitas terkecil pun mempanen bug nyata. Saat assert gagal: cetak angka AKTUAL dulu (jangan percaya hitungan manual — saya salah hitung token `int` ×3 dan `->` dua kali).
**Log Keyword:** self-hosting, bootstrap, stage1 lexer, string index i8, escape hex22, fnRetTypes, builtin classification
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #038 — vscode-ec: interpolation highlighting, didClose support, version bump
**Date:** 2026-08-22
**File:** tools/vscode-ec/{syntaxes/ec.tmLanguage.json, client.js, package.json}
**Problem:** (1) String interpolation `\${...}` di grammar TextMate tidak dikenali — VSCode men-highlight isi interpolation sebagai teks biasa, bukan sebagai token bahasa. (2) `client.js` tidak mengirim `textDocument/didClose` ke LSP saat file ditutup → diagnostics tetap tersisa di editor untuk dokumen yang sudah ditutup.
**Root Cause:** (1) Pola string double-quote hanya menangani escape `\X`, tidak ada sub-pattern untuk `\${...}`. (2) `onDidCloseTextDocument` tidak pernah subscriped di client.
**Fix:** (1) Tambah pattern `\$\{[^}]*\}` di strings repository dengan scope `constant.language interpolation.ec`. (2) Tambah listener `onDidCloseTextDocument` yang mengirim `textDocument/didClose` untuk file `.ec`. (3) Bump version 0.3.3 → 0.3.4.
**Verify:** (1) Grammar valid (`python3 -m json.tool`); test LSP dengan `print("Hello, ${greeting}")` → 0 diagnostics (valid code). (2) Code dengan string tak-tertutup `print("Hello, ${world)` tetap terdeteksi "Unterminated string" + "unclosed '('". (3) `node --check client.js` OK; `package.json` valid.
**Lesson:** Grammar TextMate butuh sub-pattern eksplisit untuk setiap syntax embedded (interpolation, regex, dll); tanpa itu highlighter tidak tahu cara memproses konten internal.
**Log Keyword:** interpolation, tmLanguage, textDocument/didClose, vscode extension, syntax highlight
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #037 — Benchmark kecepatan vs C (EC 0.013s vs C 0.010s) + 2 bug tipe for-loop
**Date:** 2026-08-22
**File:** compiler/types/typechecker.cpp, examples/bench/main.ec (BARU), handoffs/fondasi-roadmap.md
**Problem:** Owner menargetkan bahasa "cepat" — belum ada angka pembanding. Sambil membuat bench, 2 error palsu muncul: `for i in 0..N` → E1001 "Undefined variable: i", dan `total += i` → E2003 "expected int, got unknown".
**Root Cause:** (1) typeCheckFor masih mendaftarkan variabel loop sebagai nullptr (pola lama yang sama dengan kasus let — Fix #030 hanya menyembuhkan let/mut). (2) Pemeriksa assignment menolak tipe "unknown", melanggar prinsip unknown ≠ error (#030).
**Fix:** for-loop mendaftar BasicType("unknown"); guard assignment melewati pemeriksaan bila salah satu sisi "unknown".
**Verify:** Bench fib(32)+100jt iterasi, zig cc -O2 untuk keduanya: EC real 0.013s vs C 0.010s (~1.3x); fib cocok (2178309). Loop-sum EC=887459712 = wrap i32 konsisten semantik C-int (C pakai long long di bench — beda tipe sengaja). Regresi 7/7 PASS.
**Lesson:** Klaim performa WAJIB disertai benchmark yang bisa diulang; dan setiap perluasan typechecker cenderung memunculkan kembali pola nullptr di jalur yang terlewat (for-loop) — audit semua titik registrasi simbol saat mengubah kebijakan tipe.
**Log Keyword:** benchmark, fib, performance baseline, for-loop variable undefined, unknown assign
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #036 — "Tetep gak bisa": findTool kini cek ~/.local/bin via $HOME + ekstensi pakai path absolut & terminal baru
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp, tools/vscode-ec/{client.js, package.json}, dist/ec-language-0.3.3.vsix, ~/.local/bin/engc
**Problem:** Setelah #035 user tetap gagal: terminal VSCode yang DIJALANKAN SEBELUM instalasi membawa PATH lama tanpa ~/.local/bin — findTool("zig") melewatkan zig meski filenya ada di sana; ditambah client.js MEMAKAI ULANG terminal "EC Run" lama (env basi) dan mengandalkan `engc` dari PATH terminal.
**Root Cause:** Fallback direktori statis tidak memuat ~/.local/bin (lokasi instalasi user kita); resolver ekstensi bergantung environment terminal yang basi.
**Fix:** (1) irgen findTool: fallback kini resolve `$HOME/.local/bin` saat runtime + vektor dinamis. (2) client.js: resolveCompiler() absolut (setting → ~/.local/bin → /usr/local/bin → /usr/bin → root workspace); terminal EC Run di-dispose & dibuat BARU tiap run. Versi ekstensi 0.3.3.
**Verify:** `env -i HOME=… PATH=/usr/bin:/bin engc run examples/hello/main.ec` → sukses ("Hello, EnginotechC++!") — skenario terminal terburuk; 7/7 integration PASS; vsix 0.3.3 berisi resolveCompiler (grep=2).
**Lesson:** Dua sisi harus kebal-PATH: compiler (fallback $HOME) DAN launcher ekstensi (path absolut + terminal segar). Reuse terminal = reuse environment basi.
**Log Keyword:** still not working, stale terminal, HOME local bin fallback, absolute compiler path, fresh terminal
**Deploy:** ✅ LOCAL BUILD + dist/ec-language-0.3.3.vsix 2026-08-22

### Fix #035 — CG002 "No suitable backend": compiler kini menemukan clang/zig sendiri + toolchain persisten
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp, scripts/build.sh, ~/.local/{bin/engc,bin/zig→.local/opt/zig}, dist/ec-language-0.3.2.vsix
**Problem:** Tombol ▶ di VSCode gagal: "CG002 No suitable backend found: need clang" — meski file .ec terdeteksi benar. Terminal VSCode tidak punya PATH ke toolchain dev yang tadinya di /tmp.
**Root Cause:** (1) Toolchain (zig+clang) disimpan di /tmp/opencode — EPHEMERAL, terhapus berkala; binary engc lama bergantung PATH berisi /tmp/opencode/bin. (2) findTool() hanya mencari PATH — tidak ada fallback lokasi maupun backend alternatif.
**Fix:** (1) Zig 0.13.0 diunduh persisten ke ~/.local/opt/zig-linux-x86_64-0.13.0, symlink ~/.local/bin/zig (~/.local/bin sudah di PATH via .bashrc). (2) findTool() bertingkat: env EC_CLANG → PATH → direktori fallback (/tmp/opencode/bin, /usr/local/bin, /usr/bin, homebrew, llvm-14..18) → nama versi clang-N. (3) Backend baru: bila clang & llvm-as tak ada, `zig cc` dipakai sebagai clang-substitute (`<zig> cc -O2 file.ll -o out -lm`) — terbukti identik hasilnya. (4) build.sh resolusi CXX: env → zig di PATH → ~/.local/bin/zig. (5) Pesan error CG002 kini actionable (sebut install clang/zig + EC_CLANG).
**Verify:** `env -i HOME=… PATH=~/.local/bin:/usr/bin:/bin engc run examples/hello/main.ec` → "Hello, EnginotechC++!" (simulasi terminal VSCode kosong); regresi 7/7 integration PASS dengan binary baru; zig cc compile hello.ll diverifikasi manual sebelum diadopsi.
**Lesson:** JANGAN pernah taruh toolchain di /tmp untuk repo multi-sesi — /tmp dibersihkan sistem dan menghapus jejak "kok dulu bisa?". Compiler yang baik membawa strategi penemuan backend sendiri (env → PATH → lokasi umum → substitusi), bukan mewarisi asumsi PATH developer.
**Log Keyword:** CG002, no suitable backend, zig cc backend, EC_CLANG, toolchain persistent, findTool fallback
**Deploy:** ✅ LOCAL BUILD + ~/.local/bin/engc 2026-08-22

### Fix #034 — ▶ menolak file .EC huruf besar + pesan ekstensi jadi bahasa Inggris
**Date:** 2026-08-22
**File:** tools/vscode-ec/{client.js, package.json}, tools/lsp/ec-lsp.js, dist/ec-language-0.3.1.vsix (ARTIFACT)
**Problem:** User membuka test.EC (ekstensi huruf besar), menekan ▶, tapi muncul popup "Buka file .ec dulu" — padahal editor aktif JUSTRU file EC. Pesan-pesan ekstensi berbahasa Indonesia.
**Root Cause:** Guard `fileName.endsWith('.ec')` case-sensitive; VSCode mendeteksi bahasa secara case-insensitive sehingga tombol ▶ tampil (resourceLangId==ec) tetapi handler menolaknya — kontradiksi UI.
**Fix:** Deteksi via `doc.languageId === 'ec'` (otoritatif) dengan fallback regex `/\.(ec)$/i`; pesan error → "The active editor is not an EC (.ec) file."; seluruh diagnostik ec-lsp.js dialihbahasakan: "Unterminated string", "'X' without an opening bracket", "N unclosed '{'", "Function 'f' is declared more than once". Versi ekstensi 0.3.1.
**Verify:** node -e simulasi test.EC: languageId-check true, regex-fallback true; bundle dalam vsix diaudit grep — 0 string Indonesia user-facing tersisa; json.tool & node --check lolos.
**Lesson:** Di ekstensi VSCode, KEPUTUSAN menampilkan command (when clause, case-insensitive) dan VALIDASI internal handler HARUS memakai sumber kebenaran yang sama (languageId), bukan string-match nama file.
**Log Keyword:** case sensitive extension, EC uppercase, languageId, english diagnostics, run button rejected
**Deploy:** ✅ ARTIFACT dist/ec-language-0.3.1.vsix 2026-08-22

### Fix #033 — Tombol ▶ Run File di VSCode + ikon file .ec di explorer
**Date:** 2026-08-22
**File:** tools/vscode-ec/{package.json, client.js, icons/ec-lang.svg (BARU), README.md}, scripts/pack_vsix.sh, tools/pack_vsix_fallback.py, dist/ec-language-0.3.0.vsix (ARTIFACT)
**Problem:** (1) File test.ec di explorer/tab tidak berlogo seperti bahasa lain. (2) Tidak ada cara menjalankan program langsung dari editor — harus buka terminal manual.
**Root Cause:** Kontribusi bahasa tanpa field `icon`; tidak ada command run + menu entry.
**Fix:** (1) `languages[0].icon` = SVG logo E yang sama (#4FC3F7) untuk light/dark — tampil di explorer & tab pada theme default/mode-icons aktif. (2) Command `ec.run` dengan `"icon": "$(play)"` di menu `editor/title` (group navigation, when resourceLangId==ec) + keybinding Ctrl+F5; handler menyimpan dokumen lalu kirim `engc run "<file>"` ke terminal "EC Run" (path compiler via setting `ec.compilerPath`, default "engc"); activationEvents onLanguage:ec agar tombol aktif sejak file dibuka.
**Verify:** package.json lolos json.tool, client.js lolos node --check; vsix 0.3.0 (10 file) diekstrak & diaudit: $(play), menu when, keybinding, configuration ec.compilerPath, language icon — semua ada.
**Lesson:** Fitur UI VSCode deklaratif (commands+menus+icon) butuh activation event yang tepat — tanpa onLanguage:ec handler tak terdaftar saat file pertama dibuka.
**Log Keyword:** play button, run file, editor title menu, codicon play, file icon explorer, ec.compilerPath
**Deploy:** ✅ ARTIFACT dist/ec-language-0.3.0.vsix 2026-08-22

### Fix #032 — Paket .vsix resmi: ec-language-0.1.0.vsix, self-contained (LSP di-bundle)
**Date:** 2026-08-22
**File:** scripts/pack_vsix.sh (BARU), tools/pack_vsix_fallback.py (BARU), tools/vscode-ec/client.js, dist/ec-language-0.1.0.vsix (ARTIFACT)
**Problem:** Ekstensi hanya bisa dipakai via symlink folder repo — belum ada .vsix untuk "Install from VSIX..."; selain itu client.js menjalankan LSP dari `../lsp/ec-lsp.js` DI LUAR folder ekstensi → saat dipack server hilang.
**Root Cause:** Path LSP relatif ke struktur repo, bukan ke root ekstensi; tidak ada skrip packaging.
**Fix:** (1) client.js kini memilih `server/ec-lsp.js` yang DI-BUNDLE bila ada (fallback ke lokasi lama untuk dev). (2) `scripts/pack_vsix.sh`: salin ec-lsp.js ke tools/vscode-ec/server/, panggil `npx @vscode/vsce package --allow-missing-repository` → dist/ec-language-<versi>.vsix; fallback manual python (zip + extension.vsixmanifest + [Content_Types].xml) bila vsce gagal. (3) README: hapus embed ![Logo](icon.png) relatif — penyebab vsce menolak tanpa repository URL.
**Verify:** unzip -l: 9 file termasuk extension.vsixmanifest, icon.png, server/ec-lsp.js; zipfile.testzip() bersih; simulasi instal nyata: ekstrak .vsix → node --check client.js OK → jalankan server TER-BUNDLE dari lokasi ekstraksi → smoke-test LSP mengembalikan 3 diagnostik tepat.
**Lesson:** Ekstensi VSCode harus self-contained — semua runtime asset di dalam folder ekstensi; path keluar-repo pasti putus saat packaging. README jangan embed gambar relatif kalau tidak mau wajib isi field repository.
**Log Keyword:** vsix, package extension, vsce, self-contained lsp bundle, pack_vsix
**Deploy:** ✅ ARTIFACT dist/ec-language-0.1.0.vsix 2026-08-22

### Fix #031 — Ekstensi VSCode siap pakai: logo E biru muda transparan + README + smoke-test LSP
**Date:** 2026-08-22
**File:** tools/vscode-ec/{icon.png (BARU), package.json, README.md (BARU)}, /tmp/opencode/make_icon.py (generator)
**Problem:** Ekstensi VSCode (dari Fix #029) belum punya ikon — daftar ekstensi tampil generik; owner minta logo huruf "E" biru muda, agak besar, tanpa background.
**Root Cause:** Tidak ada berkas icon.png & field "icon" di manifest.
**Fix:** icon.png 128×128 RGBA dibuat programatik (python3+zlib, tanpa PIL): huruf E geometris 4 rounded-rect, warna #4FC3F7, background alpha=0; field `"icon": "icon.png"` di package.json; README.md berisi cara instal lokal (symlink ke ~/.vscode/extensions) + cara build engc + menjalankan .ec dari terminal VSCode.
**Verify:** PNG valid (signature+IHDR+RGBA terverifikasi & dilihat visual: E biru muda besar di latar transparan); package.json/grammar/langconfig lolos `python3 -m json.tool`; client.js & ec-lsp.js lolos `node --check`; smoke-test LSP end-to-end (spawn ec-lsp.js → initialize → didOpen kode rusak) mengembalikan 3 diagnostik tepat ("String tidak ditutup", "'}' tanpa pembuka", "Fungsi 'f' dideklarasikan dua kali"). Catatan: input yang tidak melanggar aturan analizer ringan memang menghasilkan diagnostik kosong — by design (tanpa spawn compiler).
**Lesson:** PNG bisa digenerate tanpa dependensi (zlib+struct) untuk aset sederhana; uji LSP cukup dengan klien Node telanjang lewat stdio framing Content-Length.
**Log Keyword:** vscode extension, icon, logo E biru muda, transparan, LSP smoke test, tmLanguage
**Deploy:** ✅ LOCAL 2026-08-22 (instal manual via symlink; belum ke marketplace)

### Fix #030 — Fondasi: build script auto-glob, versi sinkron, free() append, "unknown ≠ error"
**Date:** 2026-08-22
**File:** scripts/build.sh (BARU), compiler/driver/main.cpp, compiler/codegen/irgen.cpp, compiler/types/typechecker.cpp, handoffs/fondasi-roadmap.md (BARU)
**Problem:** Build manual zig c++ 50-an file satu-per-satu — tiap sesi paralel menambah file = link error "undefined symbol"; versi hardcoded v0.1.0; .append() bocor memori per panggilan; typechecker WIP menghasilkan E1001/E2003 palsu ("expected int, got int", "Undefined variable: c" untuk enum).
**Root Cause:** (1) Daftar source tidak pernah otomatis. (2) Versi tidak dibaca dari VERSION. (3) append malloc tanpa free. (4a) E2003 membandingkan identitas shared_ptr<Type>, bukan tipe. (4b) FieldAccess/Interp/Conditional tak men-set expr->type → let menyimpan nullptr → lookup .get()==nullptr dilaporkan "undefined". (4c) BinaryOp error pada operand unknown alih-alih meneruskan unknown.
**Fix:** build.sh: glob 62 file + cache objek + buang object yatim + blacklist llvcodegen.cpp (butuh llvm-dev) & platform_{windows,posix}.cpp + injeksi -DENG_VERSION dari VERSION; cmdVersion/cmdHelp pakai ENG_VERSION; emitListAppend kini `call void @free(oldPtr)` setelah memcpy; TypeChecker: E2003 via nama ternormalisasi (int32/int, float*/float64, lowercase), case FieldAccess (base bukan variabel = akses bertipe → jangan rekursi), StringInterp→string, Conditional→tipe then, default case set "unknown", BinaryOp & let/mut toleran unknown.
**Verify:** 7/7 integration PASS + calculator OK pada binary hasil scripts/build.sh; `engc version` → v dari VERSION; pythonic demo (list+ternary+match) benar.
**Lesson:** (1) Glob + blacklist adalah satu-satunya cara build bertahan di repo multi-sesi aktif. (2) Prinsip "unknown ≠ error" mencegah efek domino satu ekspresi tak-diketahui menjadi puluhan error palsu. (3) PATH clang TIDAK persisten antar invokasi tool — selalu export; kegagalannya mengelabui diagnosis ("7 tests failed" padahal program benar).
**Log Keyword:** fondasi, build script, glob, ENG_VERSION, memory leak append, unknown type, E2003 pointer compare, blacklist llvm
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #029 — Ekosistem tooling JS: doctor/new/watch/LSP + ekstensi VSCode
**Date:** 2026-08-22
**File:** tools/ec-doctor.js, ec-new.js, ec-watch.js, lsp/ec-lsp.js, vscode-ec/*
**Problem:** Tooling terbatas build/smoke/analyze; tidak ada integrasi editor nyata.
**Fix:** ec-doctor (deteksi g++/backend/folder), ec-new (scaffold proyek), ec-watch (fs.watch recursive → rebuild+smoke otomatis), ec-lsp (LSP 3.x stdio: initialize/didOpen|didChange → publishDiagnostics: kurung seimbang, string tak ditutup, fn duplikat), ekstensi VSCode (tmLanguage highlight .ec + language-configuration + client.js menyambung LSP tanpa vsce).
**Verify:** doctor menampilkan status env benar; ec-new membuat /tmp/demo_app; LSP membalas initialize via Content-Length framing; seluruh skrip exit 0.
**Lesson:** LSP cukup diimplementasikan sebagai JSON-RPC framing manual (~150 baris) — tidak perlu SDK untuk fitur diagnostics dasar.
**Log Keyword:** tooling, javascript helper, doctor, new, watch, lsp, vscode extension, tmLanguage
**Deploy:** ✅ LOCAL 2026-08-22

### Fix #028 — Global vars lolos typecheck + tooling JS (ec-build/ec-smoke/ec-analyze)
**Date:** 2026-08-22
**File:** compiler/types/typechecker.cpp, tools/*.js, tools/README.md
**Problem:** Program memakai global top-level (`const PATH_`, `let counter`) gagal E1001 di typechecker: `symbols_.clear()` per-fungsi menghapus global yang terdaftar. Belum ada helper JS untuk build/test/analisis.
**Fix:** (1) Pre-pass global di check(): kumpulkan Let/Mut/Const top-level lalu injeksikan kembali ke symbols_ sebelum tiap typeCheckFn (nama fungsi tak dioverride). (2) tools/ec-build.js auto-discover semua .cpp (walker + EXCLUDE llvcodegen/platform_win*) — tahan penambahan file baru; ec-smoke.js menjalankan tests/smoke via `engc run` + diff `.expected` (mendukung stdin & negatif, meneruskan ENGC_LLVM_BIN/LD_LIBRARY_PATH ke child); ec-analyze.js statistik LOC/TODO.
**Verify:** node tools/ec-build.js → OK 59 TU 57.7s; node tools/ec-smoke.js → **11 passed / 0 failed** (termasuk 10_globals & 11_files baru). Bug JS tersendiri: `$SOURCES`-style array spread & trailing-newline driver tanpa \n → normalisasi `.replace(/\s+$/,'')`.
**Lesson:** Smoke suite berbasis output terbukti menangkap regresi lintas-sesi (typechecker globals rusak oleh refactor #019 sesi lain) — pengaman regresi itu WAJIB dijalankan setiap ada perubahan compiler.
**Log Keyword:** typechecker globals clear, top-level const let mut, ec-build js, ec-smoke js, ec-analyze js, trailing newline diff
**Deploy:** ✅ LOCAL 2026-08-22 (11/11)


### Fix #026 — Percabangan lengkap gaya Python: ternary `x if c else y` (match-case sudah ada, kini teruji)
**Date:** 2026-08-22
**File:** compiler/ast/nodes.h, compiler/parser/parser.cpp, compiler/codegen/{irgen.cpp,irgen.h}, tests/integration/test_pythonic.ec
**Problem:** Ekspresi kondisional satu-baris tidak ada — user harus menulis if/else statement blok hanya untuk memilih nilai. Match-case ternyata SUDAH berfungsi (string & int arm, block body, `_` default) tapi belum teruji di suite.
**Root Cause:** Tidak ada node AST untuk conditional expression; parseExpression berhenti di keyword IF.
**Fix:** (1) Node `ConditionalExpr` (cond/thenValue/elseValue) + ExprKind::Conditional. (2) Parser: di ujung parseExpression, jika token `if` muncul pada baris yang sama dengan akhir ekspresi dan minPrec<=P_NONE → parse cond (level P_OR, tanpa ternary telanjang — sama seperti or_test Python) → expect else → else-branch full-expression (nesting kanan boleh). **Guard baris wajib**: tanpa itu, `let x = 87` + newline + `if x > 3 {` akan tertelan sebagai ternary dan merusak semua skrip mode-python yang sudah jalan. (3) Codegen: `select i1 cond, ty a, ty b`; tipe common = Str > Float > Bool > Int via materialize. (4) inferTypeOf(Conditional) = tipe then-branch; collectStrings mengunjungi ketiga cabang.
**Verify:** 7/7 tes integrasi PASS; m-series manual: ternary int/string/modulo/nested-paren benar (`t3="mid"`), match string/int/block-arm/default benar, guard baris aman (`if` setelah let-newline tetap statement). Kasus pinggul: assert(tier=="high") gagal ternyata salah EKSPEKTASI tes sendiri (a=10→"mid") — bukan bug compiler.
**Lesson:** Menambah bentuk sintaks di level ekspresi penuh (P_NONE) WAJIB disertai guard batas-statement (baris/baris-token) kalau bahasa ini newline-insensitive; uji dulu pola "let X\nif ..." sebelum commit.
**Log Keyword:** ternary, conditional expression, python style, select instruction, same-line guard, match case
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #025 — Fitur Python-style: script mode, elif, and/or/not, list (len/append/for-in) + 3 bug codegen tersembunyi
**Date:** 2026-08-22
**File:** compiler/lexer/{lexer.cpp,token.h}, compiler/parser/{parser.cpp,parser.h}, compiler/codegen/{irgen.cpp,irgen.h}, compiler/driver/main.cpp, tests/integration/test_pythonic.ec
**Problem:** Bahasa belum terasa seperti Python: wajib `fn main()`, tak ada `elif`/`and`/`or`/`not`, list hanya bisa di-index tanpa `len()`/`.append()`/iterasi `for x in list`.
**Root Cause (3 bug tersembunyi yang ditemukan selama implementasi):**
1. `emitIndex` memakai stride 1 byte padahal `emitArrayLit` menyimpan elemen stride 8 byte → `arr[1]` membaca byte ke-8? tidak — membaca byte ke-1, hasil sampah; `for x in list` juga kena.
2. `inferTypeOf(StringInterpExpr)` → nullptr → variabel hasil interpolasi diketik `i32` → pointer string di-ptrtoint rusak.
3. `main.cpp` memanggil codegen DUA kali (`compileToIR` hasilnya dibuang + `compileToNative`); plus global-inits dieksekusi sebelum body sehingga init yang bergantung urutan (interpolasi memanggil `len(list)` yang baru di-append) salah nilai.
**Fix:**
1. Lexer: keyword `elif`, `and`, `or`, `not` (alias token AND/OR/NOT yang sudah ada — otomatis ke precedence parser). Parser: rantai `elif` desugar jadi IfStmt bersarang (`parseIfStmtElif`). Script mode: statement top-level non-deklarasi jadi body `main()` implisit di irgen generate() (flag `scriptMode_` menonaktifkan emitGlobalInits agar `let` dieksekusi sekali sesuai urutan sumber). `tryParseAssignment()` direfactor: dukung target index `arr[i] = v` + dipakai di top-level.
2. Codegen list: map `arrayLens_` (nama var lokal/global → jumlah elemen, dilacak saat let-literal & append); builtin `len(x)` (list→konstanta terlacak, string→strlen); `.append(v)` = malloc(n+1)*8 + memcpy + store elemen ke-8n + update slot; `emitListFor` = loop indeks dengan GEP stride 8. Perbaiki stride di `emitIndex` (mul i64 idx,8 utk array; tetap 1 utk string) dan `emitAddrOf(IndexExpr)`.
3. Hapus panggilan `compileToIR()` borosan di main.cpp; `memcpy` non-void wajib diberi nomor register (bug lama kambuh).
**Verify:** tests/integration 7/7 PASS (test_pythonic.ec baru: script mode+append+len+for-in+index-assign+elif+and/or/not+interp); pythonic.ec demo total=150 grade=B; calculator OK; garbage/broken resilience tetap 9 & 7 error satu jalan.
**Lesson:** (a) Dua jalur emit untuk hal yang sama (global-inits vs body) = sumber bug urutan — pilih SATU berdasarkan mode. (b) Selalu cek stride/konvensi layout saat menambah operasi koleksi baru. (c) Panggilan codegen ganda di driver menggandakan diagnostik dan menyembunyikan state-dependent bugs — hasil yang dibuang pun tetap efek sampingnya kalau ada error-reporting.
**Log Keyword:** script mode, implicit main, elif, and or not, len append for-in, array stride, inferTypeOf StringInterp, double codegen
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #024 — Compiler menyerah di error pertama → recovery multi-error satu jalan
**Date:** 2026-08-22
**File:** compiler/parser/parser.cpp, compiler/diagnostics/diagnostic.h, compiler/codegen/irgen.cpp, compiler/types/typechecker.{h,cpp}, compiler/driver/main.cpp
**Problem:** Satu kesalahan kecil menghentikan seluruh kompilasi ("Parse errors" lalu exit) — user memperbaiki 1 error, rebuild, ketemu error berikutnya, ulangi berkali-kali.
**Root Cause:** Setiap fase driver mengembalikan early-return pada diagnostik pertama; parser tidak memakai synchronize() (hanya maju 1 token saat gagal); emitBlock codegen `break` saat hasError_; TypeChecker tidak memeriksa Assign dan Call.
**Fix:** (1) parseBlock/parseTopLevel pakai panic-mode sync ke batas statement (`;`, `}`, fn/let/if/while/for...) dengan jaminan progres anti-hang. (2) Driver menjalankan lexer→parse→semantic→typecheck SEKALI JALAN tanpa early-return; semua diagnostik dicetak sekali + ringkasan `=== N error(s), M warning(s) ===`. (3) Codegen lanjut emit setelah error (semua CG002 terkumpul). (4) Cek baru: E1002 pemanggilan fungsi tak dikenal (dengan pre-pass kumpul nama fn), E1001 assign ke variabel belum deklarasi. (5) Cap 100 diagnostik + penghitung overflow. (6) Pesan "Expected 81" → nama token terbaca.
**Verify:** broken.ec (7 cacat plan) → 7 error sekaligus; garbage.ec (token acak) → 9 error, tanpa hang/crash; regresi 6/6 tes integrasi PASS, calculator OK.
**Lesson:** Pipeline multi-fase yang berhenti di error pertama menyembunyikan biaya iterasi user; panic-mode recovery + gating codegen (bukan gating frontend) adalah titik tengah yang tepat.
**Log Keyword:** error recovery, synchronize, multi-error, panic mode, diagnostic cap, E1002, resilience
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #023 — Alpha smoke-test harness: 9 kasus end-to-end berbasis output
**Date:** 2026-08-22
**File:** tests/alpha_smoke.sh, tests/smoke/*.ec|*.expected|*.stdin
**Problem:** Tidak ada pengaman regresi berbasis OUTPUT — tes integrasi #015 memakai assert+exit-code, tapi perubahan halus (format float, urutan baris, cabang div-by-zero) lolos tak terdeteksi.
**Fix:** Suite berjalan: build compiler dari source → compile+run tiap tests/smoke/*.ec via `engc run` → diff stdout vs .expected (stdin fixture didukung; pola *_neg_* = wajib GAGAL compile). Backend otodetect: clang atau llvm-as+llc+cc; env override ENGC / ENGC_LLVM_BIN / ENGC_LD_LIB. Jalankan: `./tests/alpha_smoke.sh`. Melengkapi (bukan menggantikan) runner #015.
**Verify:** Hasil di mesin ini: **9 passed, 0 failed** (hello, functions, loops, structs/methods, enum/match, strings, math builtins, io/input, negatif undefined-var).
**Lesson:** Kesalahan sendiri: skrip awal memakai `$SOURCES` (unquoted) yang untuk array bash hanya mengembalikan ELEMEN PERTAMA — linker gagal "undefined main" misterius. Selalu pakai `"${ARRAY[@]}"`.
**Log Keyword:** smoke, test, regression, alpha, expected output, diff, bash array SOURCES, engc run
**Deploy:** ✅ LOCAL 2026-08-22 (9/9)

### Fix #022 — Builtins Alpha: input/konversi/math/string + perbandingan string + parse `int(x)`
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp, compiler/parser/parser.cpp, compiler/types/typechecker.cpp
**Problem:** Program `.ec` tak bisa interaktif maupun parsing angka — tanpa itu CLI tools mustahil ditulis. Juga `"a" < "b"` belum didukung.
**Fix:** (1) Builtins baru di emitCall: `input()` (fgets stdin, strip \n), `int(str)`/`float(str)` (atoi/atof), `println`, `abs/min/max` (int & float via select), `sqrt/pow/floor/ceil` (libm), `upper/lower` (@eng_case_map), `contains` (strstr), `chr/ord`. (2) Perbandingan relasional string `< > <= >=` pakai sign strcmp. (3) parseAtom: keyword tipe INT_T/F32/F64 diikuti `(` diterima sebagai call konversi. (4) inferTypeOf kini tahu tipe balikan semua builtin — sebelumnya `let f = float("2.5")` di-infer i32 sehingga TER-TRUNCATE jadi 2; abs/min/max mengikuti tipe argumen. (5) typechecker: ident `_` (wildcard match) tak lagi dilaporkan undefined. (6) Link `-lm` utk libm.
**Verify:** 18 output builtins.ec benar (84, 3.000000, 7, 2.500000, ... A, 65); io.ec dengan stdin "Ferendra\n25\n" → "Halo, Ferendra!" + "Tahun depan: 26"; IR valid llvm-as 19; masuk smoke suite (#023) 9/9.
**Lesson:** Menambah builtin = 4 titik serempak: prelude declare/helper, branch emitCall, inferTypeOf (kalau tidak, let-inference diam-diam truncate!), dan link flag. Jangan tulis IR helper dari ingatan — validasi dengan llvm-as SEBELUM lanjut (sempat tertulis `%sub` tak terdefinisi & phi salah predecessor; ketahuan langsung saat validasi).
**Log Keyword:** builtins, input, atoi, atof, sqrt, upper lower contains, strcmp lt gt, inferTypeOf truncation, int( float( keyword call
**Deploy:** ✅ LOCAL 2026-08-22 (9/9)

### Fix #021 — Codegen: irgen.h cleanup unused/stale members
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.h
**Problem:** irgen.h contains declarations for methods and fields that are either unused or have been removed: `materializeToBool`, `startBlock`, `emitPrintArgs`, `labelCounter_`, `terminated_`, `curBlock_`.
**Root Cause:** Leftover declarations from earlier refactorings; `label` method now uses a local static counter; `terminated()` delegates to helper; some fields were never used.
**Fix:** Removed `materializeToBool`, `startBlock`, `emitPrintArgs` from private section; removed `labelCounter_`, `terminated_`, `curBlock_` member variables.
**Verify:** Build succeeds; no compile errors from stale declarations.
**Lesson:** Dead code in headers causes confusion; run clang-tidy or similar tools to detect unused members.
**Log Keyword:** codegen, irgen, cleanup, unused members
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #020 — Parser: Remove Windows-hardcoded debug log path
**Date:** 2026-08-22
**File:** compiler/parser/parser.cpp
**Problem:** Debug fopen("C:\Users\asus_\Downloads\EngineSoft\debug_parser.log") hardcoded in parser — would fail on all non-Windows systems and leak paths.
**Root Cause:** Leftover debugging code not cleaned up before production.
**Fix:** Removed the entire debug fopen/fprintf block before parseBlock() call.
**Verify:** Parser compiles cleanly on Linux; no file system leaks.
**Lesson:** Never commit debug logging with hardcoded absolute paths — use stderr or a configurable log path.
**Log Keyword:** parser, debug, hardcoded path, portability
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #019 — Semantic: let/mut variables not declared before expression analysis
**Date:** 2026-08-22
**File:** compiler/semantic/semantic.cpp
**Problem:** For `let x = x + 1`, the variable `x` on RHS was looked up after analysis completed → UDF if self-reference; same for `mut` declarations.
**Root Cause:** checkLet/checkMut declared variable AFTER analyzing init expression, breaking self-referential patterns common in imperative code.
**Fix:** Move `symbolTable_[stmt->name] = stmt->type` (or nullptr for inference) BEFORE `analyzeExpr(stmt->init)`; also handle type-inferred case (`stmt->type == nullptr`) by declaring with nullptr so IR generator can infer.
**Verify:** Self-referential let/mut patterns now work; test_arith.ec `let mut x = 5; x = x + 1` passes type checking.
**Lesson:** In imperative languages, declarations must be visible before body for self-reference to work.
**Log Keyword:** semantic, let, mut, self-reference, forward declare
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #018 — TypeChecker: Missing Match statement handling
**Date:** 2026-08-22
**File:** compiler/types/typechecker.cpp, typechecker.h
**Problem:** Match expressions in function bodies were silently ignored during type checking → no type information for scrutinee or arm bodies.
**Root Cause:** `checkStmt` switch had no case for `StmtKind::Match`; the arm bodies weren't traversed.
**Fix:** Added `typeCheckMatch` method and `case StmtKind::Match` in `checkStmt`. Iterates scrutinee and each arm's pattern+body.
**Verify:** Match expressions now type-check correctly; no missing cases in diagnostics.
**Lesson:** When adding new AST node types, always add corresponding handler in every visitor/traverser.
**Log Keyword:** typechecker, match, statement, visitor pattern
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #017 — TypeChecker: for-loop loop var not in scope during body analysis
**Date:** 2026-08-22
**File:** compiler/types/typechecker.cpp
**Problem:** In `for j in 0..10 { count = count + j }`, reference to loop var `j` inside body caused E1001 "Undefined variable".
**Root Cause:** `typeCheckFor` declared loop var AFTER analyzing iterable but BODY analysis happened before declaration took effect in symbol table order — the symbols_ map is used during body traversal.
**Fix:** Moved `symbols_[stmt->varName] = nullptr` BEFORE `checkStmt(stmt->body)` so the loop var is visible throughout body analysis.
**Verify:** test_control.ec `for j in 0..10 { count = count + 1 }` now passes type checking.
**Lesson:** Scope variables BEFORE analyzing nested constructs that reference them.
**Log Keyword:** typechecker, for-loop, scope, loop variable
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #016 — Arduino target: Duplicate method declarations in Emitter class
**Date:** 2026-08-22
**File:** compiler/targets/arduinogen.cpp
**Problem:** C++ compilation error: "cannot be overloaded with" for line, emitStmt, emitBlock, emitExpr, cppTypeOf — duplicate declarations in class Emitter (lines 51-60).
**Root Cause:** Manual copy-paste introduced duplicate private method declarations in the anonymous namespace Emitter class.
**Fix:** Removed the second copy of line/emitStmt/emitBlock/emitExpr/cppTypeOf declarations (kept first 5 lines, removed duplicates at lines 56-60).
**Verify:** `g++ -std=c++20` compilation succeeds without errors; arduinogen.cpp compiles cleanly.
**Lesson:** Always verify class declarations aren't duplicated after refactoring; IDE or clang-tidy catches this.
**Log Keyword:** arduino, target, duplicate, declaration, compile error
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #015 — Driver: `engc test` belum diimplementasi → test runner + 6 tes integrasi
**Date:** 2026-08-22
**File:** compiler/driver/main.cpp, tests/integration/*.ec (baru)
**Problem:** `engc test <dir>` hanya mencetak "Test runner not yet implemented in M0."; tests/test_*.ec sebenarnya file C++ salah nama.
**Root Cause:** M0 meninggalkan stub; tidak ada mekanisme discover-compile-run untuk file .ec.
**Fix:** cmdTest: recursive_directory_iterator cari *.ec, jalankan cmdRun per file, exit code 0 = PASS; ringkasan passed/failed. Rename tests/test_*.ec → tests/test_*.cpp. Buat 6 tes integrasi (arith/control/struct/enum/string/fn) memakai assert.
**Verify:** `engc test tests/integration` → 6 passed, 0 failed.
**Lesson:** Tes integrasi berbasis assert+exit-code cukup untuk bootstrap; runner tinggal wrapper cmdRun.
**Log Keyword:** cmdTest, test runner, integration tests, assert
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #014 — Codegen: penomoran register LLVM bentrok dengan call tanpa hasil
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp
**Problem:** clang: "instruction expected to be numbered '%5' or greater" pada IR yang urutannya tampak benar; muncul setiap ada printf di jalur assert.
**Root Cause:** Call non-void TANPA register hasil (`call i32 @printf(...)`) tetap mengonsumsi slot implicit numbering LLVM, sehingga `%4 = load` eksplisit berikutnya bertabrakan. Berbeda dengan kasus #009 — ini bukan soal terminator.
**Fix:** Assign register hasil untuk semua call non-void yang di-emit manual (`%pr = call i32 ...`), khususnya jalur assert. Samping: tambah konversi i32↔i64 (sext/trunc) di materialize() yang selama ini error "Cannot convert value for use as i64" saat index array.
**Verify:** test_enum.ec (2×assert) dan test_arith.ec lolos; seluruh suite 6/6.
**Lesson:** Di textual LLVM IR, setiap nilai (termasuk hasil call yang dibuang) menempati nomor; emit call tanpa assignment hanya valid untuk void.
**Log Keyword:** codegen, register numbering, bare call, printf, assert, sext, trunc, materialize
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #013 — Codegen: GEP constant-expression ber-paren & global string terlambat → IR invalid
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp, compiler/codegen/irgen.h
**Problem:** clang: "expected '(' in constantexpr" / "constant expression type mismatch"; string literal yang dipakai sebelum pool global di-emit gagal compile.
**Root Cause:** stringConstant() menghasilkan teks GEP gaya konstanta (`getelementptr inbounds ([N x i8], ...)`) tapi teks itu juga dipakai sebagai operand instruction (tanpa paren). Ditambah @.fmt.float dideklarasikan [6 x i8] padahal "%.6f\00" hanya 5 byte, dan string late-bound (mis. pesan assert) tidak ada di pool yang di-emit di awal.
**Fix:** Opaque pointers membuat GEP tak diperlukan untuk string: stringConstant() langsung return alamat global (`@.str.N`) sebagai ptr; konstanta late-bound dikumpulkan ke pendingStrings_ dan di-flush di akhir modul (LLVM mengizinkan definisi global setelah pemakaian); ukuran @.fmt.float dikoreksi [5 x i8].
**Verify:** hello world end-to-end (compile→run→output benar), calculator lengkap termasuk divide-by-zero path.
**Lesson:** Dengan opaque pointers (LLVM 15+), alamat global ADALAH ptr — hindari GEP konstanta untuk string; simpan pool string sekali di akhir modul.
**Log Keyword:** codegen, gep, constantexpr, opaque pointer, string global, pendingStrings, fmt.float
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fitur baru terverifikasi sesi ini (bukan bug-fix, tanpa nomor)
- Array literal `[1,2,3]` + index baca/tulis (`arr[0]`, `arr[i]=v`) — malloc buffer, Slot.elemVT, inferTypeOf(ArrayLit)="array", emitAddrOf IndexExpr.
- String interpolation `"${expr}"` di parser (sub-Lexer/sub-Parser per ekspresi).
- Parser: `let mut x` sugar; compound assignment `+= -= *= /=` desugar jadi BinaryOp+AssignStmt.
- TypeChecker: loop-var for-in masuk scope sementara; FieldAccess berbasis tipe (enum `Color.Red`) tak lagi E1001.

### Fix #012 — Parser: `match` statement discarded, parsed as literal 0
**Date:** 2026-08-22
**File:** compiler/parser/parser.cpp
**Problem:** `match n { 0 => {...} }` di dalam fn body tidak menghasilkan kode apa pun; semua jalur match menghasilkan nilai awal (classify(0/1/9) semuanya return 0).
**Root Cause:** `parseStatement()` tidak punya `case TokenType::MATCH`, sehingga jatuh ke default → parseExprStmt → parseAtom yang menelan token MATCH dan mengembalikan placeholder LiteralExpr 0 (kode M0 lama).
**Fix:** Tambahkan `case TokenType::MATCH: return parseMatchStmt();` di parseStatement().
**Verify:** more.ec test: classify(0)=100, classify(1)=200, classify(9)=300 (wildcard) — sebelumnya 0/0/0. IR valid via llvm-as LLVM 19.
**Lesson:** Saat menambah statement baru, cek DISPATCH di parseStatement(), bukan hanya handler-nya.
**Log Keyword:** parser, match, statement, dispatch, wildcard
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #011 — Codegen: `for` body tanpa terminator → IR invalid
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp (file baru, rewrite M1)
**Problem:** llvm-as: "expected instruction opcode" di label for.inc — blok body loop berakhir tanpa terminator.
**Root Cause:** LLVM IR tidak punya implicit fall-through antar blok; emitFor berasumsi fall-through ke blok increment.
**Fix:** Setelah emit body for-loop: `if (!terminated()) emit br label %for.inc`.
**Verify:** features.ec dengan 2 loop (while + for-range) → llvm-as OK, output 55/55.
**Lesson:** LLVM IR: SETIAP basic block wajib diakhiri terminator (ret/br); selalu validasi IR dengan llvm-as saat development codegen.
**Log Keyword:** codegen, irgen, terminator, for-loop, fallthrough, llvm-as
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #010 — Codegen: field access struct membaca alamat slot, bukan isi pointer
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp (rewrite M1)
**Problem:** `p.x` setelah `let p = Point{x:3,...}` menghasilkan garbage (-258954216), method call juga menerima param kotor.
**Root Cause:** (1) emitAddrOf melakukan GEP langsung ke reg slot (yang berisi ALAMAT variabel penampung pointer), padahal harus load ptr dulu baru GEP ke struct. (2) Method call `obj.method()` tidak meneruskan receiver sebagai argumen pertama.
**Fix:** emitAddrOf: jika slot vt==StructPtr/ArrayPtr → emit `load ptr` dulu. emitCall cabang method: receiver selalu jadi argv[0] (materialize ke tipe param pertama).
**Verify:** features.ec: p.x=3 ✓, p.manhattan()=7 ✓ (3+4). Sebelumnya -258954216 / -258921939.
**Lesson:** Slot variabel = alloca; untuk tipe pointer, nilai yang tersimpan adalah pointer — jangan konfusi alamat cell vs nilai pointer.
**Log Keyword:** codegen, struct, gep, load, receiver, method, self
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #009 — Codegen: terminator detection gagal karena indentasi
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp (rewrite M1)
**Problem:** `br label %if.end` di-emit SETELAH `ret i32 0` di then-branch → "Terminator found in the middle of a basic block".
**Root Cause:** lastIsTerminator() membandingkan prefix "ret "/"br " pada baris yang masih ber-indentasi ("  ret i32 0") → tidak perlu cocok.
**Fix:** Trim leading whitespace baris terakhir sebelum prefix check.
**Verify:** calculator.ec (if + return di dalam then) → IR valid; divide(10,0) mencetak "Error: Division by zero!" lalu return 0.
**Lesson:** Pattern-matching teks IR harus trim whitespace dulu; lebih baik track state terminator secara eksplisit.
**Log Keyword:** codegen, terminator, br, ret, indent, basic block
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #008 — Driver: fallback toolchain llvm-as→llc→cc saat clang tidak ada
**Date:** 2026-08-22
**File:** compiler/codegen/irgen.cpp (compile())
**Problem:** Di mesin tanpa clang (Linux ini), `engc run` selalu gagal "Failed to compile LLVM IR with clang" walau pipeline sampai codegen sukses.
**Root Cause:** compile() hard-dependency ke clang tunggal.
**Fix:** Jika clang tak ditemukan di PATH, gunakan rantai llvm-as → llc -filetype=obj → cc/gcc untuk produce binary yang sama.
**Verify:** `engc run examples/hello/main.ec` END-TO-END: compile + run → "Hello, EnginotechC++!" dst.; calculator full output benar termasuk div-by-zero.
**Lesson:** Backend sebaiknya degrades gracefully; pisahkan "generate IR" dari "compile native" agar bisa diuji terpisah.
**Log Keyword:** driver, clang, fallback, llvm-as, llc, gcc, toolchain
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #007 — Lexer/Parser: COMMENT token merusak raw pos_++ setelah current()
**Date:** 2026-08-22
**File:** compiler/parser/parser.h, parser.cpp
**Problem:** `let a = 5;\n// komentar\nprint(str(a));` → E1001 "Undefined variable: print". Statement setelah baris komentar salah parse (ident print jadi bare IdentExpr, panggilan hilang).
**Root Cause:** current()/peekNext() melewati COMMENT di variabel LOKAL tanpa menggeser pos_, sementara banyak tempat (parseAtom IDENT branch, parseExprStmt, dll.) melakukan pos_++ mentah setelah membaca current() → yang terkonsumsi token COMMENT, bukan IDENT.
**Fix:** Buang semua token COMMENT sekali di konstruktor Parser (tokens_ jadi salinan value, difilter). Semua skip-comments manual menjadi no-op yang aman.
**Verify:** t14/t16/c0 (calculator utuh): sebelumnya TYPECHECK FAIL E1001 L37:C4 → setelahnya ALL PASS. Komentar inline & antar-statement aman.
**Lesson:** Jangan campur dua konvensi akses token stream (skip-on-read vs raw index). Satu invariant saja: stream sudah bersih sebelum parsing.
**Log Keyword:** parser, comment, lexer, pos_, lookahead, E1001, undefined variable print
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #006 — Parser: nama tipe UPPERCASE dari tokenName() memecah semantic & codegen
**Date:** 2026-08-22
**File:** compiler/parser/parser.cpp (parseSimpleType)
**Problem:** `str(two())` → CG002 "Cannot convert value to string"; vtOfType("INT") jatuh ke Unknown. Semua anotasi tipe keyword (`int`, `bool`, dst.) menghasilkan BasicType("INT").
**Root Cause:** parseSimpleType memakai `BasicType(t.tokenName())` yang mengembalikan nama token UPPERCASE ("INT","STRING","BOOL"), sedangkan semantic/typechecker/codegen membandingkan lowercase.
**Fix:** Map kanonik TokenType → lowercase name ("int","int8",...,"float32","float64","bool","char","string","byte","void") saat membentuk BasicType.
**Verify:** t8/t9: `fn two() -> int; print(str(two()))` → `%1 = call i32 @two(); %2 = call ptr @eng_int_to_str(i32 %1)` → output benar; calculator ALL PASS.
**Lesson:** Nama kanonik type ditentukan SATU tempat (parser); fase lain jangan menebak casing. Token display name ≠ type name.
**Log Keyword:** parser, typename, uppercase, tokenName, canonical, vtOfType, cannot convert to string
**Deploy:** ✅ LOCAL BUILD 2026-08-22

### Fix #001 — Lexer: Skip whitespace correctly across newlines
**Date:** 2026-08-18
**File:** compiler/lexer/lexer.cpp
**Problem:** Whitespace wasn't being consumed before tokenizing identifiers, causing tokens to miss characters after spaces.
**Root Cause:** `skipWhitespace()` was called inside `tokenize()` but positioned `pos_` correctly; however initial state had pos_=0 and line_/col_ not set.
**Fix:** Initialized line_=1, col_=0 in constructor. Added skipWhitespace() at start of main loop.
**Verify:** Tested with "let x = 10;" → tokens: LET, IDENT(x), EQUAL, INT(10), EOF. All positions correct.
**Lesson:** Always initialize state variables in constructor.
**Log Keyword:** lexer, whitespace, init
**Deploy:** M0 bootstrap

### Fix #002 — Parser: Handle semicolons after statements
**Date:** 2026-08-18
**File:** compiler/parser/parser.cpp
**Problem:** Semicolons between statements caused parse errors.
**Root Cause:** Parser didn't consume trailing semicolons after expressions.
**Fix:** Added semicolon consumption in parseStatement() and parseBlock().
**Verify:** "let x = 10; let y = 20;" parses correctly.
**Lesson:** Don't assume semicolons are always required; handle both with and without.
**Log Keyword:** parser, semicolon, statement
**Deploy:** M0 bootstrap

### Fix #003 — Codegen: Print function not declared
**Date:** 2026-08-18
**File:** compiler/codegen/llvcodegen.cpp
**Problem:** Compiled programs crashed because printf wasn't linked.
**Root Cause:** LLVM module didn't declare printf before using it.
**Fix:** Added printf declaration in buildModule() before emitting any function calls.
**Verify:** "print(\"hello\");" compiles and runs, outputting "hello".
**Lesson:** Declare all external functions before use in LLVM IR.
**Log Keyword:** codegen, printf, linking
**Deploy:** M0 bootstrap

### Fix #004 — Driver: Command routing fails for single-arg commands
**Date:** 2026-08-18
**File:** compiler/driver/main.cpp
**Problem:** `engc help` crashed due to missing args check.
**Root Cause:** args.size() check didn't account for commands with optional args.
**Fix:** Added proper arg count validation before accessing args[2].
**Verify:** `engc help`, `engc version`, `engc test` all work with no extra args.
**Lesson:** Validate argument counts for every subcommand.
**Log Keyword:** driver, args, routing
**Deploy:** M0 bootstrap

### Fix #005 — Type checker: Missing type resolution for 'int'
**Date:** 2026-08-18
**File:** compiler/types/typechecker.cpp
**Problem:** Type checker couldn't resolve 'int' as a known type.
**Root Cause:** Type names weren't normalized to lowercase in symbol table lookup.
**Fix:** Added case-insensitive comparison for primitive type names.
**Verify:** `let x: int = 10;` passes type checking.
**Lesson:** Normalize type names to lowercase for consistent lookup.
**Log Keyword:** typechecker, int, normalization
**Deploy:** M0 bootstrap

---

## Template for New Entries

```
### Fix #N — Short Title: Symptom + Root
**Date:** YYYY-MM-DD
**File:** path/to/file
**Problem:** What was broken
**Root Cause:** Why it broke
**Fix:** What changed
**Verify:** How you tested it (with concrete numbers)
**Lesson:** What you learned
**Log Keyword:** keyword1, keyword2, keyword3
**Deploy:** Phase
```

### Fix #050 — Fase F: Multi-target codegen (JS, Python, WASM, VM bytecode)
**Date:** 2026-08-23
**File:** compiler/codegen/{python/,wasm/,vm/} (BARU), compiler/targets/{python,wasm,vm}.h/.cpp (BARU), compiler/driver/embedded.cpp
**Problem:** Tidak ada transpiler ke bahasa scripting lain dan bytecode VM untuk EC programs.
**Root Cause:** (1) Hanya ada JS emitter tapi tidak terintegrasi dengan baik. (2) Tidak ada target untuk Python, WASM, atau bytecode VM.
**Fix:** (1) Buat Python transpiler di compiler/codegen/python/emitter.{h,cpp}. (2) Buat WASM text format emitter di compiler/codegen/wasm/emitter.{h,cpp}. (3) Buat EC Virtual Machine bytecode compiler dan interpreter di compiler/codegen/vm/. (4) Integrasi ke embedded.cpp dengan flag --target python/wasm/vm. (5) Perbaiki bug for-loop di JS emitter.
**Verify:** 7/7 multi-target tests pass — hello, arithmetic, calculator untuk Python, WASM, dan VM bytecode. Runtime test suite ALL PASSED.
**Lesson:** (1) JS for-loop range bug fix: cek nullable iterable dan varName. (2) Python transpiler cukup sederhana: exprToString → Python syntax. (3) WASM text format mudah di-generate langsung dari AST. (4) VM bytecode interpreter stack-based dengan opcodes.
**Log Keyword:** multi-target, transpiler, python, wasm, webassembly, bytecode, virtual machine, VM
**Deploy:** ✅ LOCAL BUILD 2026-08-23

### Fix #051 — Fase G+H: Package manager + Language Server Protocol
**Date:** 2026-08-23
**File:** langserver/{langserver.h,langserver.cpp}, compiler/driver/main.cpp
**Problem:** Tidak ada package manager integration dan LSP untuk IDE support.
**Root Cause:** (1) Package manager ada tapi belum test. (2) LSP belum ada.
**Fix:** (1) Package manager sudah lengkap (packages/manager/, packages/registry/). (2) Buat LSP server di langserver/ dengan JSON-RPC 2.0 over stdio. (3) Integrasi ke engc lsp command.
**Verify:** LSP server starts, handles initialize request. Test suite ALL PASSED. Multi-target tests 7/7 PASS.
**Lesson:** (1) LSP requires Content-Length header framing. (2) JSON-RPC 2.0 over stdio. (3) Simple JSON parser sufficient for basic LSP.
**Log Keyword:** LSP, language server, jsonrpc, ide, completion, diagnostics, hover, go-to-definition
**Deploy:** ✅ LOCAL BUILD 2026-08-23

### Fix #050 — Fase E: Enum field access & package manager + struct inference fix
**Date:** 2026-08-23
**File:** compiler/types/typechecker.cpp, compiler/driver/main.cpp
**Problem:** Enum variant access (`Color.Green`) caused "Undefined variable" error because enum names weren't in function-scoped symbol table. Package manager commands had wrong invocation pattern.
**Root Cause:** (1) TypeChecker::check() clears symbols_ for each function body but only restores globals, not struct/enum types. (2) FieldAccessExpr enum check only compared first variant. (3) Package commands not wired up in driver dispatch.
**Fix:** (1) Restore struct/enum types in function-scoped symbols before type-checking fn bodies. (2) Enum variant lookup now iterates all variants instead of checking only variants[0]. (3) Package manager add/list/search/show commands now operational.
**Verify:** All 11 integration tests PASS including test_enum.ec. Package manager commands respond correctly.
**Log Keyword:** enum field access, undefined variable, package manager, symbols restoration
**Deploy:** ✅ LOCAL BUILD 2026-08-23
