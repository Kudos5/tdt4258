Exercise 2 - Notes
==================

New knowledge
-------------

### Declarations and definitions
- **Declaration** of an identifier (variable or function) describes its type.
  This is *needed by the compiler* to accept references to that identifier. 
  Each time you reference an identifier, make sure the place you're referencing
  from has knowledge about that identifier (usually by including a header file
  that declares it). 
- **Definition** of an identifier actually instantiates / implements it, i.e.
  defines the functionality of a function or the value of a variable.
- The header file (`.h`) should be mostly used for declarations, as a way of
  telling the compiler that *these functions (and global variables) exist somewhere*.
  Try to avoid defining anything in header files, and stick to declaring.
- Usually you would avoid global variables across files, but if you have to use
  one, you should use the **extern** keyword. This *declares* a variable, 
  without defining it.
- [Source with examples](https://stackoverflow.com/questions/1410563/what-is-the-difference-between-a-definition-and-a-declaration#1410632)

### Static keyword
- A **static variable** (variable allocated statically) in general means a variable 
  that is kept in memory for the entire run of the program.
- A static **global** variable or function is only seen by the file it's declared
  in. This works as encapsulation / access control, which is good coding style.
- A static variable **inside a function** will only be accessible from inside that
  function and will *keep its value after the function returns*.

- [Source with examples](https://stackoverflow.com/questions/572547/what-does-static-mean-in-c)



