// Demo of using input/output in Cupcake
// First compile interpreter with:
// > bash compile.sh install

// Run this file with:
// ck -f res/IODemo.ck

stdio.print('Type your name: ');
var name = stdio.readLn();

stdio.println('Hello, ' + name);

stdio.print('Type your age: ');
var age = stdio.readInt();

stdio.println('You are ' + age + ' yrs old');

stdio.println('Goodbye, lol');
