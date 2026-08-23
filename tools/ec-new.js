#!/usr/bin/env node
/**
 * ec-new.js — Scaffold proyek .ec baru.
 * Usage:  node tools/ec-new.js <nama-proyek> [dir-induk]
 */
'use strict';
const fs = require('fs');
const path = require('path');

const name = process.argv[2];
if (!name || !/^[A-Za-z_][\w-]*$/.test(name)) {
  console.error('Usage: node tools/ec-new.js <nama-proyek> [dir-induk]');
  process.exit(1);
}
const parent = process.argv[3] || '.';
const dir = path.join(parent, name);
if (fs.existsSync(dir)) { console.error(`Sudah ada: ${dir}`); process.exit(1); }

fs.mkdirSync(path.join(dir, 'src'), { recursive: true });

fs.writeFileSync(path.join(dir, 'src', 'main.ec'), `// ${name} — EnginotechC++
fn sapa(nama: string) -> string {
    return "Halo, " + nama + "!";
}

fn main() {
    print(sapa("dunia"));

    let total = 0;
    for i in 1..11 {
        total += i;
    }
    print("1+..+10 = " + str(total));
}
`);

fs.writeFileSync(path.join(dir, 'README.md'),
  `# ${name}\n\nDibuat dengan ec-new.js.\n\n## Build & Run\n\n\`\`\`bash\nengc run src/main.ec\n\`\`\`\n`);

console.log(`Proyek dibuat: ${dir}`);
console.log(`Jalankan:      engc run ${path.join(name, 'src', 'main.ec')}`);
