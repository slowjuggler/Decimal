Description.

The implementation of a Decimal type (C#) on the C. The type consists of a 1-bit sign, a 96-bit integer number,
and a scaling factor used to divide the integer number and specify what portion of it is a decimal fraction.
The scaling factor is implicitly the number 10, raised to an exponent ranging from 0 to 28 (or 29 if the number less
than 1 and the mantisse fits in 96 bits).
Decimal number implemented as a four-element array of 32-bit unsigned integers (uint32_t bits[4]).
Bits[0], bits[1], and bits[2] contain the low, middle, and high 32 bits of the 96-bit integer number accordingly.
Bits[3] contains the scale factor and sign, and consists of following parts:
bits 0 to 15, the lower word, are unused;
bits 16 to 23 contain an exponent between 0 and 29, which indicates the power of 10 to divide the integer number;
bits 24 to 30 are unused;
bit 31 contains the sign: 0 meaning positive, and 1 meaning negative.
The project includes arithmetic operations as well as rounding and comparasion operations. Due to the fact that 96 bits must be
treated as a single bit string (i.e., to implement arithmetic and shifts, it is better to explicity refer to the flag register),
some procedures are implemented using inline Assembly (x86_64). For convinience and testing procedures, the program includes a small
calculator, with standart arithmetic and comparasion operations (+,-,/,*,%,==,!=,<=,>,<,>=). Type conversions can be seen in header file. 
