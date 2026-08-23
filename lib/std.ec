// ============================================================
// Enginotech Standard Library — strings & lists (pure EC)
// Pemakaian: salin fungsi yang dibutuhkan ke proyek, atau gabung
// lewat build multi-file (import system menyusul).
// Semua fungsi murni — tanpa efek samping global.
// ============================================================

// ---------- string ----------

fn strReverse(s: string) -> string {
    let mut out = "";
    let n = len(s);
    let mut i = n - 1;
    while i >= 0 {
        out += chr(s[i]);
        i -= 1;
    }
    return out;
}

fn strStartsWith(s: string, prefix: string) -> bool {
    let plen = len(prefix);
    if len(s) < plen {
        return false;
    }
    let mut i = 0;
    while i < plen {
        if s[i] != prefix[i] {
            return false;
        }
        i += 1;
    }
    return true;
}

fn strEndsWith(s: string, suffix: string) -> bool {
    let slen = len(suffix);
    let n = len(s);
    if n < slen {
        return false;
    }
    let mut i = 0;
    while i < slen {
        if s[n - slen + i] != suffix[i] {
            return false;
        }
        i += 1;
    }
    return true;
}

fn strContainsChar(s: string, target: int) -> bool {
    let n = len(s);
    let mut i = 0;
    while i < n {
        if s[i] == target {
            return true;
        }
        i += 1;
    }
    return false;
}

fn strCountChar(s: string, target: int) -> int {
    let n = len(s);
    let mut hits = 0;
    let mut i = 0;
    while i < n {
        if s[i] == target {
            hits += 1;
        }
        i += 1;
    }
    return hits;
}

fn strRepeatChar(ch: int, times: int) -> string {
    let mut out = "";
    let mut i = 0;
    while i < times {
        out += chr(ch);
        i += 1;
    }
    return out;
}

fn strPadLeft(s: string, ch: int, width: int) -> string {
    let pad = width - len(s);
    if pad <= 0 {
        return s;
    }
    return strRepeatChar(ch, pad) + s;
}

// ---------- list ----------

fn listSum(xs: array) -> int {
    let mut acc = 0;
    for v in xs {
        acc += v;
    }
    return acc;
}

fn listMax(xs: array) -> int {
    let mut best = xs[0];
    for v in xs {
        if v > best {
            best = v;
        }
    }
    return best;
}

fn listMin(xs: array) -> int {
    let mut best = xs[0];
    for v in xs {
        if v < best {
            best = v;
        }
    }
    return best;
}

fn listContains(xs: array, target: int) -> bool {
    for v in xs {
        if v == target {
            return true;
        }
    }
    return false;
}

fn listRange(from: int, to: int) -> array {
    // inclusive-exclusive, seperti 0..N tapi sebagai nilai list
    let mut xs = [0];
    xs[0] = from;
    let mut cur = from + 1;
    while cur < to {
        xs.append(cur);
        cur += 1;
    }
    return xs;
}
