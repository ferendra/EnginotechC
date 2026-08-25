// Human-friendly syntax test (v0.4.0)
// say / set..to / repeat..times / give / is / and / not / function

function greet(name: string) {
    say "Hello, " + name + "!";
}

function main() {
    set counter = 3;

    repeat counter times {
        say "tick";
    }

    set total to 0;
    for i in 1..4 {
        total += i;   // 1+2+3 = 6
    }

    say "sum = " + str(total);

    if total is 6 and not false {
        say "HUMAN PASS";
    } else {
        say "HUMAN FAIL";
    }

    greet("EC");
}