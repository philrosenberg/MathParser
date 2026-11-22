# MathParser

MathParser is a library and executable that will parse math expressions and calulate the results.

For example it will take the following input

x=2
y=10
x+y

and will output 12

you can assign a variable to be an expression which uses other variables

z=x+y

Whenever you give it an expression which doesn't include an assignment, it calculates the result. That includes just giving it a variable name.

z

gives the output 12

MathParser works more like a spreadsheet than most programming languages, in that if you change a variable, the change propogates through. Whereas if you were programming, once you assing a variable, the varibale remains fixed until it is assigned again

x=5

z

gives the output 15 - x has changed, so z zhanges.

You can make arrays by enclosing in curly braces and separating by commas

x={2, 4, 8, 3}

x + y

gives the output { 12, 14, 18, 13 }

Everything works elementwise. As you can see we added y to every element of x. But y can be an array too

y={10, 20, 30, 40}

x + y

gives output { 12, 24, 38, 43 }

To operate on elements the sizes must match or they must have size 1.

You have seen that this works for scalars. An array with size 1 is treated as a scalar.

y={10}

x + y

gives output { 12, 14, 18, 13 }, whereas

y={10, 20}

x + y

gives an error

We can do 2D arrays as well

x = {{1, 2}{3, 4}}

y = {{5, 6}{7, 8}}

x + y

gives { { 6, 8 } { 10, 12 } }

A 2D array that has a size of 1 in all dimensions is treated as a scalar

y = {{10}}

x + y

gives { { 11, 12 } { 13, 14 } }

To be compatible, dimensions must be either 1 or the same. So

y = {{10, 20}}

x + y

gives { { 11, 22 } { 13, 24 } }


1D arrays are treated as having their first dimension as 1. So {10, 20} is equivalent to {{10,20}}

To get a 1d array in the other dimensions we can do

y = {{10}{20}}

x + y

gives { { 11, 12 } { 23, 24 } }

And we can combine 1d Arrays in the two directions to give a 2D array

x = {1, 2}

y = {{10}{20}}

x + y

gives { { 11, 12 }{ 21, 22 } }

You can of course use brackets and all the standard math operators

(10+2)*(1+2)/4+33

gives output 42

the caret symbol is power. 

2^3

gives output 8

For power towers, we work from the top down

2^3^4

gives output 2.41785e+24, i.e. 2^3^4 = 2^(3^4) = 2^81. This is the correct methematical way to calculate power towers, but you should be aware that some sofware could incorrectly give the answer 4096 here.


