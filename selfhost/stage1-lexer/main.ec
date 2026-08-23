// ============================================================
// Self-hosting Stage 1: EC lexer written IN EC.
// Tokenizes an embedded sample program, counts token categories,
// and self-checks the results with asserts.
//
// Known limits (documented in ../ROADMAP.md):
//   - tokens are streamed (no list-of-tokens return yet)
//   - string escapes are kept raw
// ============================================================

fn isDigit(c: int) -> bool {
    return c >= 48 and c <= 57;
}

fn isAlpha(c: int) -> bool {
    return (c >= 65 and c <= 90) or (c >= 97 and c <= 122) or c == 95;
}

fn isAlnum(c: int) -> bool {
    return isDigit(c) or isAlpha(c);
}

fn classifyWord(w: string) -> int {
    // 1 = keyword, 0 = plain identifier
    if w == "fn" { return 1; }
    if w == "let" { return 1; }
    if w == "mut" { return 1; }
    if w == "if" { return 1; }
    if w == "elif" { return 1; }
    if w == "else" { return 1; }
    if w == "while" { return 1; }
    if w == "for" { return 1; }
    if w == "in" { return 1; }
    if w == "int" { return 1; }
    if w == "return" { return 1; }
    if w == "true" { return 1; }
    if w == "false" { return 1; }
    if w == "none" { return 1; }
    if w == "match" { return 1; }
    if w == "struct" { return 1; }
    if w == "enum" { return 1; }
    if w == "impl" { return 1; }
    if w == "import" { return 1; }
    if w == "pub" { return 1; }
    if w == "break" { return 1; }
    if w == "continue" { return 1; }
    return 0;
}

fn main() {
    let src = "fn add(a: int, b: int) -> int {\n    return a + b;\n}\nlet msg = \"hi\";\n";

    let n = len(src);
    let i = 0;

    let kwCount = 0;
    let idCount = 0;
    let numCount = 0;
    let strCount = 0;
    let opCount = 0;
    let commentCount = 0;
    let total = 0;
    let firstWord = "";

    while i < n {
        let c = src[i];

        // whitespace
        if c == 32 or c == 9 or c == 10 or c == 13 {
            i += 1;
        }
        // line comment
        elif c == 47 and i + 1 < n and src[i + 1] == 47 {
            commentCount += 1;
            while i < n and src[i] != 10 {
                i += 1;
            }
        }
        // string literal
        elif c == 34 {
            strCount += 1;
            total += 1;
            i += 1;
            while i < n and src[i] != 34 {
                if src[i] == 92 {
                    i += 1;   // skip escaped char
                }
                i += 1;
            }
            i += 1;           // closing quote
        }
        // number literal
        elif isDigit(c) {
            numCount += 1;
            total += 1;
            while i < n and (isDigit(src[i]) or src[i] == 46) {
                i += 1;
            }
        }
        // identifier / keyword
        elif isAlpha(c) {
            let mut w = "";
            while i < n and isAlnum(src[i]) {
                w += chr(src[i]);
                i += 1;
            }
            if firstWord == "" {
                firstWord = w;
            }
            total += 1;
            if classifyWord(w) == 1 {
                kwCount += 1;
            } else {
                idCount += 1;
            }
        }
        // two-char operators
        elif i + 1 < n and (
            (c == 61 and src[i + 1] == 61) or      // ==
            (c == 33 and src[i + 1] == 61) or      // !=
            (c == 60 and src[i + 1] == 61) or      // <=
            (c == 62 and src[i + 1] == 61) or      // >=
            (c == 45 and src[i + 1] == 62) or      // =>
            (c == 38 and src[i + 1] == 38) or      // &&
            (c == 124 and src[i + 1] == 124) or    // ||
            (c == 46 and src[i + 1] == 46)         // ..
        ) {
            opCount += 1;
            total += 1;
            i += 2;
        }
        // single-char operators / punctuation
        else {
            opCount += 1;
            total += 1;
            i += 1;
        }
    }

    // ---- self-test on the known sample ----
    // fn add ( a : int , b : int ) -> int { return a + b ; } let msg = "hi" ;
    assert(total == 25);
    assert(kwCount == 6);      // fn int int int return let
    assert(idCount == 6);      // add a b a b msg
    assert(strCount == 1);     // "hi"
    assert(numCount == 0);
    assert(opCount == 12);     // ( : , : ) -> { + ; } = ;
    assert(commentCount == 0);
    assert(firstWord == "fn");

    print("stage1-lexer OK");
    print("tokens:", total, "(kw", kwCount, ", id", idCount, ", str", strCount, ")");
}
