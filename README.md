# MathParser

MathParser is a library and executable that will parse math expressions and calulate the results.

For example it will take the following input

x=2
y=10
x+y

and will output 12

you can assign a variable to be an expression which uses other variables

z=x+y

Whenever you give it an expression which doesn't include an assignment, it calculates the result

z

gives the output 12

MathParser works more like a spreadsheet than most programming languages, in that if you change a variable, the change propogates through. Whereas if you were programming, once you assing a variable, the varibale remains fixed until it is assigned again

x=5
z

gives the output 15 - x has changed, so z zhanges.

You can make arrays by enclosing in curly braces and separating by commas

x={2, 4, 8, 3}
z

gives the output { 12, 14, 18, 13 }

Everything works elementwise. As you can see we added y to every element of x. But what y can be an array too

y={10, 20, 30, 40}
z

gives output { 12, 24, 38, 43 }

If the sizes don't match, then the smaller array is tiled to make it big enough

y={10, 20}
z

gives output { 12, 24, 18, 23 }

The arrays don't have to nest perfectly

y = { 10, 20, 30 }
z

gives output { 12, 24, 38, 13 }

You can of course use brackets and all the standard math operators

(10+2)*(1+2)/4+33

gives output 42

the caret symbol is power. 

2^3

gives output 8

For power towers, we work from the top down

2^3^4

gives output 2.41785e+24, i.e. 2^3^4 = 2^(3^4) = 2^81. This is the correct methematical way to calculate power towers, but you should be aware that some sofware could incorrectly give the answer 4096 here.
