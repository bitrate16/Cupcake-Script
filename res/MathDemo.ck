// Demo of using Math in Cupcake
// First compile interpreter with:
// > bash compile.sh install

// Second compile StreamApi & Files modules with:
// > bash buildso.sh src/modules/Math.cpp bin/Math.so
// >> this will generate file: Math.so

// Run this file with:
// ck -f res/MathDemo.ck

// For list of functions see reference http://www.cplusplus.com/reference/cmath

stdio.println('Math load response: ' + NativeLoader.load('../bin/Math.so'));

// Convert 45 degrees to radians
var angleDeg = 45;
var angleRad = Math.DEG2RAD * angleDeg;

stdio.println("sin(45) = " + Math.sin(angleDeg));
stdio.println("cos(45) = " + Math.cos(angleDeg));
// e.t.c...
