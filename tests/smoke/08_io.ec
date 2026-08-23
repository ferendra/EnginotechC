// Smoke 08 — input() from stdin + int(input())
fn main() {
    print("Siapa nama kamu?");
    let nama = input();
    print("Halo, " + nama + "!");
    let umur = int(input());
    print("Tahun depan: " + str(umur + 1));
}
