# Sourcery - An Intuitive C++ Preprocessor

**CURRENTLY A WIP, NOT READY FOR USE!**

Sourcery is a preprocessor engine for C++ source files that allows the user to quickly
expand redundant information out across multiple lines. Sourcery supports numerical,
alphabetical, and comma delimited sources for macros.

With Sourcery, you can turn this inline text macro:

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

# Quickstart Guide

There are currently no available binaries for Sourcery, so you will need to
compile Sourcery from source:

1. Clone the repository to any directory of your choice.

	``` git clone https://github.com/0xDATAWOLF/sourcery.git ```

2. Configure the project using CMake.

	``` cmake . -B ./build ```

3. Build the project.

	``` cmake --build ./build ```

4. Copy the executable from the build binary directory. The relative directory is shown:

	``` ./build/bin/Debug/sourcery.exe ```

5. Paste the executable somewhere that you can add it to your environment variable PATHs. An example is shown:

	``` C:\Program Files (Custom)\binaries\sourcery.exe ```

6. Add Sourcery to your environment variables PATHs.

# Useage Documentation

### Using Sourcery

Sourcery is a CLI application that takes a source file as its input.

```
C:\Development\Projects\myawesomeproject> sourcery ./src/main.cpp
```

<table width="100%">
	<tr>
		<th width="20%">Argument</td>
		<th>Description</td>
	</tr>
	<tr>
		<td>--show-previews</td>
		<td>
			Displays the expanded macro to the user and prompts the user if they
			would like to continue. Limits the preview to 5 lines for each expansion.
		</td>
	</tr>
	<tr>
		<td>--show-full-previews</td>
		<td>
			Displays the expanded macro to the user and prompts the user if they
			would like to continue. Shows the full preview for each expansion.
		</td>
	</tr>
	<tr>
		<td>--revert</td>
		<td>
			Restores a given source file back to its most recent state relative to
			the current source file.
		</td>
	</tr>
	<tr>
		<td>--unrevert</td>
		<td>
			Restores to the most recent source file if it is not the most recent
			source file.
		</td>
	</tr>
	<tr>
		<td>--check-cache</td>
		<td>
			Displays a list of source files that Sourcery has cached.
		</td>
	</tr>
	<tr>
		<td>--clear-cache</td>
		<td>
			Clears the cached sources files.<br/><br/>
			<strong>Note: The cache is used for restoring source files should the user
			see any issues with the macro expansion. This should only be invoked when
			<i>all</i> source files are in a valid and acceptable state.</strong>
		</td>
	</tr>
	<tr>
		<td>--set-datafile</td>
		<td>
			Provices a Comma Delimited Source Sequence to the application.
		</td>
	</tr>
	<tr>
		<td>--skip-invalid</td>
		<td>
			Sourcery will skip any invalid macro expansions and continue parsing the
			file.
		</td>
	</tr>
</table>

### Default Behaviors

Once Sourcery is invoked, it will begin parsing the file for any macros to expand.
Sourcery will overwrite the existing source file with the expanded macros should
they all be valid. This behavior can be overriden with `--skip-invalid` and will
skip over any invalid macros should it encounter any. Additionally, you can preview
the changes before committing them with `--show-preview` and `--show-full-preview`.
You will have the option to decline the changes for each macro expansion as desired.

### Reverting Changes

In the event that there is a problem with your macro expansion, you can revert your
changes back to its last known state by invoking `--revert` in one of two ways:

With the source file:

```
C:\Development\Projects\myawesomeproject> sourcery ./src/main.cpp --revert
```

Or without (Sourcery will know the last file it touched):

```
C:\Development\Projects\myawesomeproject> sourcery --revert
```

Sourcery will cache all the macro expansions for you. You can revert a file as
many times as you want so long as it exists within the cache. You can also "unrevert"
the changes back to the most recent version of the file with `--unrevert`. The most
recent file is the source file before `--revert` was invoked.

### The Line Header Token and the Escape Token

In order for Sourcery to find a line to parse, it must first be headed by `@@`.
The example below shows how to designate a particular line of code to be parsed
by Sourcery.

```C++
std::vector<int> myVector;
@@myVector.push_back(@1:4:);
```

When Sourcery encounters the line header token, it will begin evaluating the line
for any sources to evaluate. In the example above, it will expand to:

```C++
std::vector<int> myVector;
myVector.push_back(1);
myVector.push_back(2);
myVector.push_back(3);
myVector.push_back(4);
```

Additionally, you can escape any `@` symbols with `@@` after the line header
token. When Sourcery encounters any additional pairs of `@@`, it will convert it
to `@` for you. Below is an example of how this escape sequence works:

```C++
@@std::string myStringEscaped = "This @@1:4: is escaped, this @1:4: is not.\n";
```

And Sourcery will parse it as:

```C++
std::string myStringEscape = "This @1:4: is escaped, this 1 is not.\n";
std::string myStringEscape = "This @1:4: is escaped, this 2 is not.\n";
std::string myStringEscape = "This @1:4: is escaped, this 3 is not.\n";
std::string myStringEscape = "This @1:4: is escaped, this 4 is not.\n";
```

### Alphabetical Sequences

Sourcery can generate a series of alpha characters for you. In order for Sourcery
to know where to stop, you must provide a range. The case of the range determines the
output case, such as A-Z or a-z. You can set the start of the range to any character.
In order for the input to be valid, the range must be supplied correctly and share
the case-ness of the start. The range must be in alphabetical order. Note that `@Z:B`
is invalid, but `@B:Z` is not.

```C++
@@std::string charString = "This is the character: @C:F.\n";
```

```C++
std::string charString = "This is the character: C.\n";
std::string charString = "This is the character: D.\n";
std::string charString = "This is the character: E.\n";
std::string charString = "This is the character: F.\n";
```

### Numerical Sequences

Similar to alphabetical sequences, you can generate a series of numbers as well.
Unlike alphabetical sequences, however, you need to end the numerical sequence with
an additional colon in order for Sourcery to recognize the last digit of the 
sequence. The order also matters, as the sequence must begin on the left of the
number scale and end on the right--or more simply the start of the range is the
smallest value, and the end of the range is the largest value.
Below, you will see it in action:

```C++
int myIntegerArr[3];
myIntegerArr[@0:2:] = @0:2:;
```

```C++
int myIntegerArr[3];
myInteger[0] = 0;
myInteger[1] = 1;
myInteger[2] = 2;
```

Consider now a sequence that starts negative:

```C++
std::vector<int> myNumericVec;
@@myNumericVec.push_back(@-3:1:);
```

```C++
std::vector<int> myNumericVec;
myNumericVec.push_back(-3);
myNumericVec.push_back(-2);
myNumericVec.push_back(-1);
myNumericVec.push_back(0);
myNumericVec.push_back(1);
```

### Comma Delimited Source Sequences

For data that isn't alphanumeric, you may want to supply a comma delimited source
file. You can supply this data as a CSV or text file. In short, each row corresponds
to a single iteration, and each column corresponds to an indexable entry. In essence,
the total count of lines within the CSV or text file corresponds to the amount of
iterations applied to the macro, and each column can be used within that iteration.

We will assume a basic text file with some basic data:

```
hello,world,1
great,work,0
another,day,3
another,project,7
```

As you can see, there are four lines, meaning the macro will iterate four times.
Since each line contains three entries, there are three indexable items you can
use within the macro. Since the data is non-sequential, meaning that the output
is not able to be generated at runtime, Sourcery will assume a few things about
your data:

1. The data within an entry is always valid.

	Sourcery does not check the data nor will it parse data. It is the user's
	responsibility to ensure that the data they are inputting is correctly formatted.

2. Empty entries are valid.

	If there are any empty entries within the data, Sourcery will treat it as valid.
	Should `great,work,0` instead be `great,,0` and you use the second index, Sourcery
	will simply not put anything there.

3. Out-of-bound entries are valid.

	Likewise, if you read beyond the column count and index an invalid entry, Sourcery
	will treat it as valid and treat it is as an empty value. Therefore, Sourcery will
	not put anything in there.

4. Asymetrical data is valid.

	The column counts need not be consistent as it would conflict with assumptions 2 & 3.
	Therefore, it is up to the user to ensure the data is symmetrical or take asymetrical
	data into consideration while designing their macro.

In order for Sourcery to tell that you are using a source, you must prefix with `@$`
followed by the column index followed by a colon. The example below uses the basic text
file from above as the source.

```C++
@@std::string myString@$3: = "@$1: @$2:";
```

```C++
std::string myString1 = "hello world";
std::string myString0 = "great work";
std::string myString3 = "another day";
std::string myString7 = "another project";
```

### Multiple Tokens on One Line

For macros that contains more than one expansion tokens, the user must ensure that
each token expands equally. Take, for example:

```C++
std::string myString@1:5: = "Your number is @1:5:!";
```

As you can see, there are two tokens present. Each token expands 5 times. Since
both tokens expand five times, this expansion is considered valid. However, if
they weren't:

```C++
std::string myString@1:5: = "Your number is @1:7:!";
```

Then how would Sourcery interpret this expansion? It couldn't. It doesn't make
sense to create 5 `myString` variables assigned with 7 different values. Therefore,
each token must be the same size. When using comma delimited sources, this rule
also applies.

# Contributing to Sourcery

If you would like to contribute to Sourcery, please feel free to get in contact
through Github, submit pull requests (and appropriately document and explain the
changes) and I will go through them. This project is something I've been meaning
to complete for awhile, as I have found myself wishing there was a sufficiently
powerful way of meta-programming redundant data. The aim of this project is to be
simple and fast--define the macro inline as fast you would program that line without
and run Sourcery on the source file.


