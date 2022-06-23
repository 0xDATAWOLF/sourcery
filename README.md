# Magikhats - A Macro Engine for C++ Source Files

**CURRENTLY A WIP, NOT READY FOR USE!**

Magikhats is a macro engine for C++ source files that allows the user to quickly
expand redundant information out across multiple lines. Magikhats supports numerical,
alphabetical, and comma delimited sources.

With Magikhats, you can turn this inline text macro:

```C++
@@SetInputState(&prevInput, &currInput, (GetKeyState('@A:Z') & 0x8000));
```

Into this expanded form:

```C++
SetInputState(&prevInput, &currInput, (GetKeyState('A') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('B') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('C') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('D') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('E') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('F') & 0x8000));
...
SetInputState(&prevInput, &currInput, (GetKeyState('Z') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('Y') & 0x8000));
SetInputState(&prevInput, &currInput, (GetKeyState('Z') & 0x8000));
```

# Using Magikhats

### The Line Header Token and the Escape Token

In order for Magikhats to find a line to parse, it must first be headed by `@@`.
The example below shows how to designate a particular line of code to be parsed
by Magikhats.

```C++
std::vector<int> myVector;
@@myVector.push_back(@1:4);
```

When Magikhats encounters the line header token, it will begin evaluating the line
for any sources to evaluate. In the example above, it will expand to:

```C++
std::vector<int> myVector;
myVector.push_back(1);
myVector.push_back(2);
myVector.push_back(3);
myVector.push_back(4);
```

Additionally, you can escape any `@` symbols with `@@` after the line header
token. When Magikhats encounters any additional pairs of `@@`, it will convert it
to `@` for you. Below is an example of how this escape sequence works:

```C++
@@std::string myStringEscaped = "This @@1:4 is escaped, this @1:4 is not.\n";
```

And Magikhats will parse it as:

```C++
std::string myStringEscape = "This @1:4 is escaped, this 1 is not.\n";
std::string myStringEscape = "This @1:4 is escaped, this 2 is not.\n";
std::string myStringEscape = "This @1:4 is escaped, this 3 is not.\n";
std::string myStringEscape = "This @1:4 is escaped, this 4 is not.\n";
```

### Alpha Sequences

Magikhats can generate a series of alpha characters for you. In order for Magikhats
to know where to stop, you must provide a range. The case of the range determines the
output case, such as A-Z or a-z. You can set the start of the range to any character.
In order for the input to be valid, the range must be supplied correctly and share
the case-ness of the start.

```C++
@@std::string charString = "This is the character: @C:F.\n";
```

```C++
std::string charString = "This is the character: C.\n";
std::string charString = "This is the character: D.\n";
std::string charString = "This is the character: E.\n";
std::string charString = "This is the character: F.\n";
```
