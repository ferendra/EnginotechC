// Smoke 11 — file I/O, substring, repeat
const PATH_ = "/tmp/engc_smoke_file.txt";

fn main() {
    write_file(PATH_, "baris satu\n");
    append_file(PATH_, "baris dua\n");
    print(file_exists(PATH_));

    let isi = read_file(PATH_);
    print(contains(isi, "dua"));

    let s = "enginotech";
    print(substring(s, 0, 7));
    print(repeat("ab", 3));
    print(len(read_file(PATH_)));

    delete_file(PATH_);
    print(file_exists(PATH_));
}
