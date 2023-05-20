# fliki

A command-line program
(called `fliki`) whose purpose is to apply patches (On-the-Fly), given a filename specified as a command-line argument, it reads that
file, tries to interpret its contents as diffs, successively applies
these diffs to source file lines read from standard input, and writes
the modified output to standard output.  In case errors are detected,
error reports are written to the standard error output.

## Stucture

<pre>
.
├── .gitlab-ci.yml
└── hw1
    ├── .gitignore
    ├── hw1.sublime-project
    ├── include
    │   ├── debug.h
    │   ├── fliki.h
    │   └── global.h
    ├── Makefile
    ├── rsrc
    │   ├── empty
    │   ├── empty_file1.diff
    │   ├── file1
    │   ├── file1_empty.diff
    │   ├── file1_file2.diff
    │   └── file2
    ├── src
    │   ├── fliki.c
    │   ├── main.c
    │   └── validargs.c
    ├── test_output
    │   └── .git-keep
    └── tests
        └── basecode_tests.c
</pre>

## Usage

pass arguments
as follows:

- If no flags are provided, it will display the usage and return with an
`EXIT_FAILURE` return code.

- If the `-h` flag is provided, it will display the usage for the program and
  exit with an `EXIT_SUCCESS` return code.

- If the `-h` flag is not provided, then the last argument on the command line must
be the name of a file that contains "diffs" (more about this below).  The program
will read a source file to be patched from standard input (`stdin`), it will edit
it in accordance with instructions read from the diff file, and it will write
the edited result to standard output (`stdout`).  If this patching process is
successful, the program will exit with an `EXIT_SUCCESS` return code, otherwise
the program will exit with an `EXIT_FAILURE` return code.
In the latter case, the program will print to standard error (`stderr`) an error
message or messages describing the error(s) that were discovered.

- If the `-n` (no output) flag is provided, then the program performs a "dry run"
in which it does everything it normally would do, except that it does not produce
any output on `stdout`.

- If the `-q` (quiet mode) flag is provided, then any error messages that would
otherwise be printed on `stderr` are suppressed.  The program will still exit
with `EXIT_FAILURE` if an error is encountered, however.

Note that the program reads a file to be patched from `stdin` and writes the edited
result to `stdout`.  Error messages are issued to `stderr`.  No other output
is produced.  If the program runs without error, then it will exit with the
`EXIT_SUCCESS` status code; if any error occurs during the execution of the program,
then it will exit with the `EXIT_FAILURE` status code.

## Diffs and Patching
The original `diff` program was written in the early 1970s for the Unix
system by M. Douglas McIlroy and James Hunt at Bell Telephone Laboratories.
The underlying idea of `diff` is to compare two versions of a source file
and produce some kind of "minimal" list of differences between them.
The original diff program output these differences in the form of a
script for the line-oriented text editor `ed`, which could run the script
to transform one version of the source file into the other.
The modern version of `diff` used on Linux systems is a reimplementation
done as part of Project GNU.

A significant application of the `diff` utility is to make it possible
to store multiple versions of a file (some source code, say) in a compressed
form by storing just one complete version together with a set of "diffs"
by which that version can be transformed into the others.  This idea
became the basis for revision-control systems such as `SCCS`, `CVS, and,
later, `git`.
In 1984, Larry Wall wrote a program called `patch` which could accept
diffs in a variety of formats (which were introduced to improve upon the
somewhat fragile nature of patching using the original `ed` script format)
and apply the diffs to one version of a file, transforming it into another.
The `patch` program provided on modern-day Linux systems is a remote descendant
of Larry Wall's original code.

A specification for the format of output files produced by `diff` can be found
[here](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/diff.html).
We will be concerned with the "Diff Default Output Format" discussed there,
also known as "traditional" diff format.
As an example, consider the following two text files:

```
This is file1.
This line does not appear in file2.
This line is in common between file1 and file2.
Here is another line that they have in common.
This line also does not appear in file2.
```

```
This is file2.
This line is in common between file1 and file2.
This line does not appear in file1.
Here is another line that they have in common.
```

The following is the output of the command `diff file1 file2`:

```
1,2c1
< This is file1.
< This line does not appear in file2.
---
> This is file2.
3a3
> This line does not appear in file1.
5d4
< This line also does not appear in file2.
```

The output consists of what the `patch` documentation refers to as "hunks",
each of which resembles (but is not exactly the same thing as) an `ed` command.
The sequence of hunks describes a series of changes to be made to `file1`
to transform it into `file2`.
There are three types of hunks, examples of which can be seen above.
One type of hunk describes deletions to be made from `file1`.
For example, the first hunk, which starts `1,2c1`, says that the range
of lines in `file1` from line 1 to line 2, inclusive, is to be
changed to (i.e. deleted and replaced by) a different line, which will appear as line 1
of `file2`.  The lines to be deleted are listed explicitly, with the
special two-character sequence "`< `" prepended.  The line to be inserted
is also listed, identified by the two character sequence "`> `".
The deletions and additions are separated by a line that contains
only "`---`".
The second hunk, which starts `3a3`, consists only of additions.
It says that a new line is to be added, at a position that corresponds
to line number 3 in both `file1` and `file2`.
The third hunk, which starts `5d4`, consists only of deletions.
It says that line number 5 in `file1` is to be deleted, and that
line number 4 in `file2` would be the immediately preceding line.

In general, the first line of each hunk has an "old range", an edit
command, and a "new range".  The old range and new range either
take the form of two line numbers separated by a comma, or a single
line number, which abbreviates the case in which the two numbers are
the same.  The single-character command is one of `a`, `d`, or `c`.
Following the first line of the hunk will be a series of lines to
be deleted (in the case of `d` or `c` commands) and a series of
lines to be appended (in the case of `a` or `c`) commands.
In the case of a `c` command, both the deletions and additions sections
are present, and they are separated by a line containing `---`.

## On-the-Fly

The mode of operation of `fliki` is to apply patches `on-the-fly` as
the input is being read in a character-by-character fashion, without
needing to buffer in memory either the contents file being patched or the
contents of hunks from the diff file.  An advantage of implementing it
this way is that it is not necessary to use dynamic storage allocation
or to make any assumptions about the maximum sizes of things like
lines of text or the number of text lines in a hunk.
Disadvantages include less flexibility in the way the program can operate.

## Input/Output Redirection
The `fliki` program reads from `stdin` and writes to `stdout`.
If you want the program to take input from a file or produce output to
a file, you may run the program using **input and output redirection**,
which is implemented by the shell using facilities provided by the operating
system kernel.
A simple example of a command that uses such redirection is the following:

```
$ bin/fliki mydiffs < old > new
```

This will cause the input to the program to be redirected from the text file
`old` and the output from the program to be redirected to the file `new`.
The redirection is accomplished by the shell, which interprets the `<` symbol
to mean "input redirection from a file" and the `>` symbol to mean
"output redirection to a file".  It is important to understand that redirection
is handled by the shell and that the `bin/fliki` program never sees any
of the redirection arguments; in the above example it sees only `bin/fliki mydiffs`
and it just reads from `stdin` and writes to `stdout`.

Alternatively, the output from a command can be **piped**
to another program, without the use of a disk file.
This could be done, for example, by the following command:

```
$ bin/fliki mydiffs < old | less
```

This sends the output to a program called `less`, which displays the first
screenful of the output and then gives you the ability to scan forward and
backward to see different parts of it.  Type `h` at the `less` prompt to get
help information on what you can do with it.  Type `q` at the prompt to exit `less`.
The `less` program is indispensible for viewing lengthy output produced by
a program.

Pipelines are a powerful tool for combining simple component programs into
combinations that perform complex transformations on data.
For example, you could connect several instances of `fliki` together as
follows:

```
$ bin/fliki mydiffs1 < old | bin/fliki mydiffs2 | bin/fliki mydiffs3 > new
```

Here there are three instances of `fliki`, each running as a separate process
and each using its own diffs.  The first instance of `fliki` patches
the contents of the file `old` using `mydiffs1` and sends the result along
to the second instance of `fliki`.  The second instance applies the
patches from `mydiffs2` and sends the result along to the third instance,
which applies patches from `mydiffs3`.  The final result is stored in
file `new`.


