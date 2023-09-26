# freelance_job_valgrind_simulator

9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 1/12
Project 2: Memory & Assembly
Due Sep 27 by 12pm Points 100 Available Sep 13 at 2:15pm - Sep 29 at 12pm
Due: 09/27/2023 at Noon
Table of Contents
Honor Policy
Gilligan's Island Rule
Assignment Details
Part 1: A Debugging Memory Allocator (60 points)
Objectives
Testing and Compiling
Background
A Note On Undefined Behavior
Part 1.1: A Debugging Allocator (60%)
Part 1.2: Heavy hitters report (40%)
Misc. Requirements
Style and Code Organization
Submission and Grading
Part 2: Binary Bomb (40 points)
Honor Policy
This is an independent project. We require all the source code you turn in to be your own. Collaboration on high-level and conceptual
concepts is permitted, and even encouraged. In other words, you are welcome to devise high-level strategies to approach the
problem with your peers. You may also use use Internet resources for general information.
However:
DO NOT, UNDER ANY CIRCUMSTANCES, COPY SOMEONE ELSE'S CODE, GIVE A COPY OF YOUR CODE TO SOMEONE
ELSE, OR MAKE IT PUBLICLY AVAILABLE. To do so is a direct violation of ethical/academic standards. Violations will be
referred to Yale's Executive Committee. Modifying code to conceal copying only compounds the offense.
Every student must turn in a separate code repository.
We are going to run sophisticated code comparison tools on all your code and on code from previous editions of this course.
Follow the Gilligan's Island Rule (see below).
You should come to your own understanding of a given problem, and implement the source by yourself.
You must not ask questions on Stack Overflow, paper.camp, or any similar site. (Of course, if you search for some C++ problem,
Stack Overflow answers may come up—just don’t ask questions yourself.)
You must not use solutions from past (or future) years.
Cite help and collaboration. If you use a classmate, other collaborator, or online resource as an aid, acknowledge this in your
assignment. Name the helpers and briefly describe how they helped. (You do not need to cite help from course staff or resources
directly linked from this site.)
A few examples of actions that you will generally want to avoid (these are easier to spot than you might think):
sharing your screen over Zoom (revealing your project code)
writing your code at the same time with a partner even if you don't look at each others code
writing pseudo code with a partner and implementing it in C on your own.
debugging another student's code.
If possible, only seek help with debugging from ULAs and TFs. We advise against helping other students debug their code; if you
help a student in this way, make sure you strictly follow Gilligan's Island Rule to ensure your own code is written independently of
their code (also document the collaboration).9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 2/12
The Gilligan's Island Rule
When discussing an assignment with anyone other than a member of the teaching staff, you may write on a board or a piece of paper,
but you may not keep any written or electronic record of the discussion. Moreover, you must engage in some mind-numbing activity
(e.g., watching an episode of Gilligan's Island) before you work on the assignment again. This will ensure that you can reconstruct
what you learned, by yourself, using your own brain. The same rule applies to reading books or studying on-line sources.
Environment Details
Complete this assignment by yourself on the Zoo computers. Parts of the assignment can only be performed on the zoo machines
-- if this is a problem, please contact us sooner rather than later. You can connect to a Zoo machine using SSH:
ssh <netID>@node.zoo.cs.yale.edu
We provide starter code and tests in /c/cs323/proj2 on the Zoo; you should copy the starter-code directory somewhere into your
home directory. You will also find a program, download, in the same location.
Part 1 : Dmalloc
Copy the starter-code in your home's cs323 directory.
The tests for this project are distributed in a manner that is different from the way they were distributed in project 1. For this project,
the tests are distributed with the starter code. See the sections below for information on how to run the public tests.
Part 2 : Binary Bomb
This program can only be performed on the Zoo computers. Note: the giraffe node may not work with this assignment, please use the
other zoo nodes.
We will provide a program that will download the tar file required for the assignment. To invoke the program, run:
/c/cs323/proj2/download
This will create a bombID.tar file in your ~/cs323/ directory, where ID is a unique number that is assigned to a particular student. If you
don’t have a cs323 folder in your home folder, be sure to make one using mkdir before running the download program. To extract the
tar file, navigate to ~/cs323/
and run the following command:
tar xvf bombID.tar
This will extract the tar file in that directory. Within the directory, you will find a binary bomb, a bomb.c and a README file that
identifies your ID and netID.
Before Starting
The project walk-through was held Friday, 2/3 at 6pm.
slides (https://yale.instructure.com/courses/88524/files/folder/Walk-Through%20Material/Project%202%20Walkthrough?
preview=7298914)
recording (https://yale.instructure.com/courses/88524/files/folder/Walk-Through%20Material/Project%202%20Walkthrough?
preview=7298920)
Assignment Details9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 3/12
Part 1: A Debugging Memory Allocator [60 points]
Let's turn our attention to dynamic memory now. Having full control over explicit allocation and deallocation of dynamic memory allows
us to write fast code for machines. However, this also leads to writing programs that crash due to memory issues. At the same time,
we can also build tools that can debug memory allocation problems.
In this project, you will write a debugging wrapper around malloc and free , that will provide many of the features of Valgrind.
Specifically, your debugging allocator will:
1. Track memory usage,
2. Catch common programming errors (e.g., use after free, double free),
3. Detect writing off the end of dynamically allocated memory (e.g., writing 65 bytes into a 64-byte piece of memory), and
4. Catch less common, somewhat devious, programming errors.
You will also augment your debugging allocator with heavy hitter reporting that tells a programmer where most of the dynamicallyallocated memory is allocated.
Testing and compiling
For this assignment, you run the tests using the make-file in the starter code. To just compile your library and tests, run make. Use the
following command to compile and run the all the tests until one fails:
make check
To run all the tests without stopping:
make check-all
To run a particular test:
make test001
./test001
Running the tests in this way will just show you your program’s output. The expected output is at the bottom of each testNNN.cc file.
NOTE: When you run tests many files will be generated in your directory. Although this has no impact in terms of functionality, you
may occasionally wish to clean up your workspace. You can clean the directory using:
make clean
This is a common operation that is provided by most build systems that are used to compile projects.
Background
Since you are implementing a wrapper around malloc, free, and calloc, here's a brief refresher on the rules and semantics of these
functions. C-style memory allocation uses two basic functions, malloc() and free()
void* malloc(size_t size)
Allocates size bytes of memory and returns a pointer to it. This memory is not initialized to any value and can contain anything.
Returns a null pointer if the allocation failed (for example, if the size was too big).
void free(void* ptr)
Frees a single block of memory pointed by ptr, previously allocated by malloc().9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 4/12
The rules of malloc() and free() are simple: Allocate once, then free once.
Dynamically-allocated memory remains active until it explicitly freed with a call to free()
The pointer argument in free(ptr) must either equal nullptr or a pointer to active dynamically-allocated memory. In particular:
It is not OK to call free(ptr) if ptr points to the program’s code, or into its global variables, or into the stack.
It is not OK to call free(ptr) unless ptr was returned by a previous call to malloc.
It is not OK to call free(ptr) if ptr is currently inactive (i.e., free(ptr) was previously called with the same pointer argument, and
the ptr memory block was not reused by another malloc).
These errors are called invalid frees. The third error is also called double free.
Some notes on boundary cases:
In C++, malloc(0) should return a non-null pointer. If ptr = malloc(0), then ptr does not overlap with any other allocation and can be
passed to free().
free(nullptr) is allowed. It does nothing.
malloc(sz) returns memory whose alignment works for any object (as described in Lecture 0, slide 66). On 64-bit x86 machines,
this means that the address value returned by malloc() must be evenly divisible by 16.
Our versions of these functions simply call basic versions, base_malloc() and base_free() . The dmalloc functions take extra filename
and line number arguments; you’ll use these to track where memory was allocated and to report where errors occur. Our header file,
dmalloc.hh, uses macros so that calls in the test programs supply these arguments automatically.
Part 1.1: Debugging allocator (60%)
You will work on these functions (though you are welcome to write code anywhere in dmalloc.cc):
dmalloc_malloc
dmalloc_free
dmalloc_calloc
dmalloc_get_statistics
dmalloc_print_leak_report
Note: There are no private tests for this section. You may use the tests to guide your implementation/debugging.
Task 1: Heap Usage Statistics
Implement the following function using statistics you gather from dmalloc_malloc and dmalloc_free:
void dmalloc_get_statistics(dmalloc_statistics* stats);
Fill in the stats structure with overall statistics about memory allocations so far.
The dmalloc_statistics structure is defined like this:
struct dmalloc_statistics {
unsigned long long nactive; // number of active allocations [#malloc - #free]
unsigned long long active_size; // number of bytes in active allocations
unsigned long long ntotal; // number of allocations, total
unsigned long long total_size; // number of bytes in allocations, total
unsigned long long nfail; // number of failed allocation attempts
unsigned long long fail_size; // number of bytes in failed allocation attempts
uintptr_t heap_min; // smallest address in any region ever allocated
uintptr_t heap_max; // largest address in any region ever allocated
};
Most of these statistics are easy to track, and you should tackle them first. You can pass tests 1-5 and 7–10 without per-allocation
metadata.9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 5/12
The hardest is active_size, since to track it, your dmalloc_free(ptr) implementation must find the number of bytes originally allocated
for ptr. The easiest, and probably best, way to do this is for your dmalloc_malloc code to request more space than the user when it
calls base_malloc. The initial portion of that space will store metadata about the allocation, including the allocated size, using a
structure you define yourself. Your dmalloc_malloc will initialize this metadata, and then return a pointer to the payload, which is the
portion of the allocation following the metadata. Your dmalloc_free code will take the payload pointer as input, and then use address
arithmetic to calculate the pointer to the corresponding metadata (possible because the metadata has fixed size). From that metadata
it can read the size of the allocation. (But there are other techniques too. You could create a hash table that mapped pointer values to
sizes. dmalloc_malloc would add an entry, and dmalloc_free would check this table and then remove the entry. You might try this
first.)
Run make check to test your work. Test programs test001.cc through test012.cc test your overall statistics functionality. Open one of
these programs and look at its code. You will notice some comments at the end of the file, such as this:
//! alloc count: active 0 total 0 fail 0
//! alloc size : active 0 total 0 fail 0
These lines define the expected output for the test. The make check command checks your actual output against the expected output
and reports any discrepancies. (It does this by calling check.pl .) Note: In expected output, ``???'' can match any number of
characters.
Task 2: Integer Overflow Protection
There is a bug in this implementation of calloc(), our tests will catch it!
void* dmalloc_calloc(size_t nmemb, size_t sz) {
void* ptr = malloc(sz * nmemb);
if (ptr != nullptr) {
memset(ptr, 0, sz * nmemb); // clear memory to 0
}
return ptr;
}
Interger overflows occur when arithmetic causes an integer variable to increase beyond its maximum
(https://en.cppreference.com/w/c/types/limits) . Calloc needs to be robust against integer overflow attacks. Our handout code’s
dmalloc_calloc function is not quite right. Fix this function and fix any other integer overflow errors you find. Test programs test013.cc
through test015.cc check your work.
Task 3: Invalid free and double free detection
dmalloc_free(ptr, file, line) should print an error message and then call C’s abort() or exit(1) function when ptr does not point to active
dynamically-allocated memory. Some things to watch:
Be careful of calls like free((void*) 0x16), where the ptr argument is not nullptr but it also doesn’t point to heap memory. Your
debugging allocator should not crash when passed such a pointer. It should print an error message and exit in an orderly way. Test
program test017.cc checks this.
The test programs define the desired error message format. Here’s our error message for test016:
MEMORY BUG: test016.cc:8: invalid free of pointer
0xffffffffffffffe0, not in heap
Error messages should be printed to standard error (using C’s fprintf(stderr, ...), or, equivalently, C++’s std::cerr).
Different error situations require different error messages; the other test programs define the required messages.
Include the file name and line number of the problematic call to free.
Test programs test016.cc through test024.cc check your work.
Hints: Task 3 and Undefined Behavior9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 6/12
In C using a pointer after freeing it, or double freeing a pointer is typically undefined behavior. Because we want to catch these
bugs, debugging allocators interact with undefined behavior. As we tell you in class, undefined behavior is a major no-no, because
any program that invokes undefined behavior has no meaning. As far as the C language standard is concerned, once undefined
behavior occurs, a program may do absolutely anything, such as force demons to fly out of your nose
(http://catb.org/jargon/html/N/nasal-demons.html) .
Many of our tests explicitly invoke undefined behavior and thus have no meaning. But helpful debuggers catch common bugs,
and undefined-behavior bugs with malloc and free are common, so your helpful debugging allocator must produce specific
warnings for these cases! Is that even possible?
Yes! Debugging allocators work by making certain undefined behaviors well-defined. For instance, when a debugging allocator is
in place, a program with a simple double free will reliably print a specific error message and exit before any undefined behavior
occurs.
To accomplish this magic trick, debugging allocators make use of properties of their underlying allocators: base_malloc and
base_free, which are defined in basealloc.cc. This allocator behaves like malloc and free, but has the following additional
properties:
base_free does not modify freed memory (the contents of freed storage remain unchanged until the storage is reused by a
future basealloc
base_free never returns freed memory to the operating system.
The heap used by base_malloc is contiguous.
This makes it much easier to write a debugging allocator with base_malloc/free than with C’s default malloc/free. Because
undefined behavior is imposed by language-defined interfaces, such as the system malloc and free . While base_malloc and
base_free are used in the same way as malloc and free , they are not language-defined, so they can offer slightly more relaxed
rules.
Note that even for base_malloc and base_free , double frees, invalid frees, and wild writes can cause truly undefined behavior, so
you need to catch it in dmalloc_malloc and dmalloc_free .
Task 4: Boundary Write Error Detection
A boundary error is when a program reads or writes memory beyond the actual dimensions of an allocated memory block. An
example boundary write error is to write the 11th entry in an array of size 10:
int* array = (int*) malloc(10 * sizeof(int));
...
for (int i = 0; i <= 10 /* WHOOPS */; ++i) {
array[i] = calculate(i);
}
These kinds of errors are relatively common in practice. (Other errors can happen, such as writing to totally random locations in
memory or writing to memory before the beginning of an allocated block, rather than after its end; but after-the-end boundary writes
seem most common.)
A debugging memory allocator can’t detect boundary read errors, but it can detect many boundary write errors. Your dmalloc_free(ptr,
file, line) should print an error message and call abort() or exit(1) if it detects that the memory block associated with ptr suffered a
boundary write error.
No debugging allocator can reliably detect all boundary write errors. For example, consider this:
int* array = (int*) malloc(10 * sizeof(int));
int secret = array[10]; // save boundary value
array[10] = 1384139431; // boundary write error
array[10] = secret; // restore old value! dmalloc can't tell there was an error!
int* array = (int*) malloc(10 * sizeof(int));
array[200000] = 0; // a boundary write error, but very far from the boundary!9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 7/12
We’re just expecting your code to catch common simple cases. You should definitely catch the case where the user writes one or
more zero bytes directly after the allocated block. Test programs test025.cc through test027.cc check your work.
Task 5: Memory Leak Reporting
A memory leak happens when code allocates a block of memory but forgets to free it. Memory leaks are not as serious as other
memory errors, particularly in short-running programs. They don’t cause a crash. (The operating system always reclaims all of a
program’s memory when the program exits.) But in long-running programs, such as your browser, memory leaks have serious effect
and are important to avoid.
Write an dmalloc_print_leak_report() function that, when called, prints a report about every allocated object in the system. This report
should list every object that has been malloced but not freed. Print the report to standard output (not standard error). A report should
look like this:
LEAK CHECK: test033.cc:23: allocated object 0x9b811e0 with size 19
LEAK CHECK: test033.cc:21: allocated object 0x9b81170 with size 17
LEAK CHECK: test033.cc:20: allocated object 0x9b81140 with size 16
LEAK CHECK: test033.cc:19: allocated object 0x9b81110 with size 15
LEAK CHECK: test033.cc:18: allocated object 0x9b810e0 with size 14
LEAK CHECK: test033.cc:16: allocated object 0x9b81080 with size 12
LEAK CHECK: test033.cc:15: allocated object 0x9b81050 with size 11
A programmer would use this leak checker by calling dmalloc_print_leak_report() before exiting the program, after cleaning up all the
memory they could using free calls. Any missing frees would show up in the leak report.
To implement a leak checker, you’ll need to keep track of every active allocated block of memory. It’s easiest to do this by adding
more information to the block metadata. You will use the file and line arguments to dmalloc_malloc().
Note: You may assume that the file argument to these functions has static storage duration. This means you don’t need to copy the
string's contents into your block metadata—it is safe to use the string pointer.
Test programs test028.cc through test030.cc check your work.
Task 6: Advanced Reports and Checking
Test programs test031.cc, test032.cc, and test033.cc require you to update your reporting and error detection code to print better
information and defend against more diabolically invalid frees. You will need to read the test code and understand what is being
tested to defend against it.
Update your invalid free message. After determining that a pointer is invalid, your code should check whether the pointer is inside a
different allocated block. This will use the same structures you created for the leak checker. If the invalid pointer is inside another
block, print out that block, like so:
MEMORY BUG: test031.cc:10: invalid free of pointer 0x833306c, not allocated
test031.cc:9: 0x833306c is 100 bytes inside a 2001 byte region allocated here
And make sure your invalid free detector can handle all situations in the other tests.
Test034 Hint
test034 calls dmalloc_malloc 500,000 times, supplying tens of thousands of different filename/line-number pairs. Your solution
should run test034 in a second or less. Our solution, with heavy hitters, runs test034 in about 0.2 seconds on a modern laptop.
What will slow you down the most is if you have to do a linked list iteration on each malloc or free. In order to pass test034, you
want to find a way to screen incoming pointers to dmalloc_free, in such a way that you know for with a high degree of confidence
that they are valid.
One way to do this screening, is to add a "present" field into the block's header/metadata. The present field could be set to some
unique magic number when allocated, this allows you to have a high probability that the pointer is from a previous allocation9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 8/12
without needing to iterate through a linked list. This design pattern is also used to identify some file types.
(https://gist.github.com/leommoore/f9e57ba2aa4bf197ebc5)
Part 1.2 Heavy hitters report (40%)
Note: the tests for this section are all private.
We provide a stress testing tool for you to use: hhtest. You will need to compile it: make hhtest . More information is given below.
Memory allocation is expensive, and you can speed up a program a lot by optimizing how that program uses malloc and by optimizing
malloc itself (Google and Facebook use their own internal version of malloc; tcmalloc (http://code.google.com/p/gperftools/) is
google's version of malloc).
But before optimizing a program, we must collect data. Programmer intuition is frequently wrong: programmers tend to assume the
slowest code is either the code they found most difficult to write or the last thing they worked on. This recommends a memory
allocation profiler — a tool that tracks and reports potential memory allocation problems.
Your job is to design and implement a particular kind of profiling, heavy hitter reports, for your memory allocator. This has two parts.
You will:
1. Track the heaviest users of malloc() by code location (file and line). A “heavy” location is a location that is responsible for allocating
many payload bytes
2. Generate a readable report that summarizes this information, using this exact format (each location on one line, sorted in
descending order by percentage):
HEAVY HITTER: hhtest.cc:48: 817311692 bytes (~25.0%)
HEAVY HITTER: hhtest.cc:47: 403156951 bytes (~12.4%)
Rule 1: If a program makes lots of allocations, and a single line of code is responsible for 20% or more of the total payload bytes
allocated by a program, then your heavy-hitter report should mention that line of code (possibly among others).
Rule 2: Your design should handle both large numbers of allocations and large numbers of allocation sites. In particular, you should
be able to handle a program that calls \malloc at tens of thousands of different file-line pairs (such as test034.)
Note: You should only count bytes requested by the program; don't include allocator overhead such as metadata space.
How should you implement this? We leave this up to you. You can implement this using something very simple if you like. Here are
some tips -
Sampling is acceptable. It would be okay, for example, to report information extrapolated from a sample of the allocations. This
could cut down the amount of data you need to store.
Make sure that you still follow Rule 1 with very high probability
You could sample exactly every Nth allocation, but random sampling is usually better, since it avoids synchronization effects.
(For instance, if the program cycled among 4 different allocation sites, then sampling every 20th allocation would miss 75\% of
the allocation sites!) For random sampling you’ll need a source of randomness. Use random() or drand48()
Here are some papers describing algorithms that catch all heavy hitters with O(1) space and simple data structures. YOU DO
NOT NEED TO USE THESE ALGORITHMS! Try to take a look, they are surprisingly simple.
A Simple Algorithm for Finding Frequent Elements in Streams and Bags
(http://www.cs.yale.edu/homes/el327/datamining2011aFiles/ASimpleAlgorithmForFindingFrequentElementsInStreamsAndBags.pdf)
, Karp, Shenker, and Papadimitriou
Frequency Estimation of Internet Packet Streams with Limited Space
(http://erikdemaine.org/papers/NetworkStats_ESA2002/paper.pdf) , Demaine, López-Ortiz, and Munro. The paper's context
doesn't matter; the relevant algorithms, "Algorithm MAJORITY" and "Algorithm FREQUENT," appear on pages 6-7, where they
are simply and concisely presented. (You want FREQUENT, but MAJORITY is helpful for understanding.)
We provide a test program for you to test heavy hitter reports, hhtest.cc. This program contains 40 different allocators that allocate
regions of different sizes. Its first argument, the skew, varies the relative probabilities that each allocator is run. Before exiting, it calls9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 9/12
dmalloc_print_heavy_hitters(), a function defined in dmalloc.cc.
Running ./hhtest 0 will call every allocator with equal probability. But allocator \#39 allocates twice as much data as any other. So
when we run our simple heavy hitter detector against ./hhtest 0, it reports:
HEAVY HITTER: hhtest.cc:49: 1643786191 bytes (~50.1%)
HEAVY HITTER: hhtest.cc:48: 817311692 bytes (~25.0%)
HEAVY HITTER: hhtest.cc:47: 403156951 bytes (~12.4%)
If we run ./hhtest 1, however, then the first allocator (hhtest.cc:10) is called twice as often as the next allocator, which is called twice
as often as the next allocator, and so forth. There is almost no chance that allocator #39 is called at all. The report for ./hhtest 1 is:
HEAVY HITTER: hhtest.cc:10: 499043 bytes (~50.0%)
HEAVY HITTER: hhtest.cc:11: 249136 bytes (~25.0%)
HEAVY HITTER: hhtest.cc:12: 123995 bytes (~12.5%)
At some intermediate skews, though, and there may be no heavy hitters at all. Our code reports nothing when run against ./hhtest 0.4
.
Negative skews call the large allocators more frequently. ./hhtest -0.4 :
HEAVY HITTER: hhtest.cc:49: 15862542908 bytes (~62.1%)
HEAVY HITTER: hhtest.cc:48: 6004585020 bytes (~23.5%)
Try ./hhtest --help to get a full description of hhtest’s arguments. You should test with many different arguments; for instance, make
sure you try different allocation “phases.” A great software engineer would also create tests of her own; we encourage you to do this!
Frequently Asked Questions:
Yes, you may report lines that have less than 20% of the total bytes allocated.
Yes, approximations of the total bytes output is completely fine. You do not have to match the exact output numbers or
percentages given above.
C++ Usage and standard Libraries
We are using the C++ compiler to build your code, so you may choose to use C++ syntax or just use C-style syntax. You may include
additional headers from the C/C++ STL. Do not use any external libraries other than the C++ Standard Libraries (i.e. do not link other
libraries in your compilation step). Here's a list of C++ includes you may use: https://en.cppreference.com/w/cpp/header
(https://en.cppreference.com/w/cpp/header)
Note: using C++ features and syntax may limit ULA's ability to help debug your code.
Style and Code Organization
Your code should be clean, clear, correct, and consistent. The most important style guideline is consistency. Don’t write code that
changes style from line to line. In addition, as a general rule, this assignment will be easier to write if you break up your program into
smaller (1-50 lines), reusable functions with readable and unambiguous names.
Grading
We will provide a set of 38 public tests for part 2.1, and we will use a small set of additional private tests to grade your code Heavy
Hitter code. You will lose additional points for other compilation issues (including warnings).
Submission9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 10/12
To submit your code you will need to run the submission tool in the project directory as follows
/c/cs323/proj2/submit-dmalloc
This binary will submit dmalloc.cc . Make sure to run this inside the directory containing dmalloc.cc; you are not allowed to use other
files for this project. You can submit any number of times; we will only look at the most recent submission.
Part 2: Binary Bomb [40 points]
Assignment Description
The purpose of the assignment is for you to become familiar with x86- IA32 Instruction Set Architecture (ISA). The nefarious Dr. Evil
has planted a slew of binary bombs on our machines. A binary bomb is a program that consists of a sequence of phases. Each phase
expects you to type a particular string on stdin. If you type the correct string, then the phase is defused and the bomb proceeds to the
next phase. Otherwise, the bomb explodes by printing BOOM!!! and then terminating. The bomb is defused when every phase has
been defused. There are too many bombs for us to deal with, so we are giving everyone a bomb to defuse. Your mission, which you
have no choice but to accept, is to defuse your bomb before the due date. Good luck, and welcome to the bomb squad!
Instructions
Your job is to defuse the bomb. You can use many tools to help you with this; please look at the tools section for some tips and ideas.
The best way is to use a debugger to step through the disassembled binary. The bomb has 9 phases. The phases get progressively
harder to defuse, but the expertise you gain as you move from phase to phase should offset this difficulty. Nonetheless, the latter
phases are not easy, so please don't wait until the last minute to start. The bomb ignores blank input lines. If you run your bomb with a
command-line argument, for example,
./bomb mysolution.txt
then it will read the input lines from mysolution.txt until it reaches EOF (end of file), and then switch over to stdin (standard input from
the terminal). In a moment of weakness, Dr. Evil added this feature so you don't have to keep retyping the solutions to phases you
have already defused. To avoid accidentally detonating the bomb, you will need to learn how to single-step through the assembly
code and how to set breakpoints. You will also need to learn how to inspect both the registers and the memory states. One of the nice
side-effects of doing the lab is that you will get very good at using a debugger. This is a crucial skill that will pay big dividends the rest
of your career.
Important: Every time that the bomb explodes, you will lose 0.5 points (from the max achievable 100 points of this part). The points
you score at the end will be scaled to 40. It is important that you use breakpoints and avoid those unnecessary explosions.
Checking your work
We have provided a webpage where you can check your work. Here, you can access the scoreboard to verify how many points you
have, up to which phase you have diffused the bomb, and so on. You have to be on the Yale network for this link to work, i.e. using
the VPN.
http://bertie.cs.yale.internal:17200/scoreboard
Tools:
There are many ways of defusing your bomb. You can examine it in great detail without ever running the program, and figure out
exactly what it does. This is a useful technique, but it not always easy to do. You can also run it under a debugger, watch what it does
step by step, and use this information to defuse it. This is probably the fastest way of defusing it. We do make one request, please do
not use brute force! You could write a program that will try every possible key to find the right one, but the number of possibilities is so9/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 11/12
large that you wont be able to try them all in time. There are many tools which are designed to help you figure out both how programs
work, and what is wrong when they don't work. Here is a list of some of the tools you may find useful in analyzing your bomb, and
hints on how to use them.
gdb: The GNU debugger is a command line debugger tool available on virtually every platform. You can trace through a program
line by line, examine memory and registers, look at both the source code and assembly code (we are not giving you the source
code for most of your bomb), set breakpoints, set memory watch points, and write scripts. Here are some tips for using gdb:
To keep the bomb from blowing up every time you type in a wrong input, you’ll want to learn how to set breakpoints.
For other documentation, type help at the gdb command prompt, or type man gdb , or info gdb at a Unix prompt. Some people
also like to run gdb under gdb-mode in emacs.
We have a small GDB cheatsheet (https://docs.google.com/document/d/1jgef7-
IugGAQahemIBXYnm9SvW7JXzrTi5CnlYrBflc/edit?usp=sharing) , and there are plenty of good resources on using gdb online
apart from the tutorial videos you saw in week one of the class. For instance:
http://www.unknownroad.com/rtfm/gdbtut/gdbtoc.html (http://www.unknownroad.com/rtfm/gdbtut/gdbtoc.html) ,
http://www.yolinux.com/TUTORIALS/GDB-Commands.html (http://www.yolinux.com/TUTORIALS/GDB-Commands.html)
You may use other debuggers if you wish to (like lldb or vscode debugger), but note that you will be responsible for setting up
to use it in the zoo with the bombs, and anything that follows (eg, if the bomb explodes).
objdump -t bomb: This will print out the bombs symbol table. The symbol table includes the names of all functions and global
variables in the bomb, the names of all the functions the bomb calls, and their addresses. You may learn something by looking at
the function names.
objdump -d bomb: Use this to disassemble all of the code in the bomb. You can also just look at individual functions. Reading the
assembler code can tell you how the bomb works. Although objdump -d gives you a lot of information, it doesn’t tell you the whole
story. You should save the output of this command in a file for quick access.
strings -t x bomb: This utility will display the printable strings in your bomb and their offset within the file.
Don’t forget, the commands apropos , and man are your friends. These will help you figure out what the commands do, if you
forget how they are supposed to be used.
The Intel X86 Instruction Set Manual (https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-
architectures-software-developer-instruction-set-reference-manual-325383.pdf) contains the explanation for all assembly instructions
that you will find in this assignment. This website (https://www.felixcloutier.com/x86/) is slightly more readable, but it is autogenerated by a script and may not have some information. Even simply searching for the instruction online should give you
enough information about it.
The lecture slides also give you information about assembly! Things that you might find particularly relevant are jump tables, function
call conventions, flags, recursion, not to mention explanation and examples for some of the basic instructions!
You can experiment with the C to assembly conversion process by writing a C program, compiling it, and using the tools listed above
to examine the executable. If you do this, make sure to pass the -m32 flag to GCC to ensure that your executable uses the same 32-
bit calling convention used in the binary bomb.
Grading
Your grade will be based on the number of stages of the bomb you have defused. As we have noted before, each time the bomb
explodes, we shall deduct 0.5 points from the score that you obtain. The total score for this part will be scaled to 40 for the final grade.
The number of points given for each phase is shown below
Points
Phase 1 7
Phase 2 10
Phase 3 139/15/23, 1:41 PM Project 2: Memory & Assembly
https://yale.instructure.com/courses/88524/assignments/370052 12/12
Phase 4 10
Phase 5 10
Phase 6 10
Phase 7 10
Phase 8 10
Phase 9 10
Phase 10 10
Submission
You do not need to submit any particular file or folder for this project as we will have a record of your progress as well as points on the
web server. In case you have a mismatch of points, please contact the TFs.
