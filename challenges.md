All in all, this was a very challenging task, especially because I had no previous experience with WebAssembly. 
However, I was able to apply my existing knowledge of low-level programming concepts so a lot of concepts were familiar.
Additionally, the scope of this project was huge. I focused on the first four test files.

## Approach

I started by building the smallest possible version of the interpreter, such that it could execute the first few tests 
from the first test file. To do that, I only parsed the sections I needed for the first test file 
(sections 1, 3, 5, 6, 7, and 10). Once that worked, I added a few basic instructions so I could execute the first tests 
and confirm the parsing and execution flow worked. From there, I added more and more instructions.

After getting some core instructions running, I wrote a script to run all the tests automatically and check the outputs. 
This made it easy to spot missing instructions or incorrect behavior. Once basic operations were reliable, I completed 
function calling and moved on to control flow.

Control flow ended up being the hardest part of the project. Handling nested blocks and branches took a lot of time to 
debug, and the most complex tests still aren’t fully correct. At some point I decided that spending all my time on 
control flow would block progress in other areas, so I shifted my focus to adding and stabilizing other features.

Originally, I tried to parse the .wat files directly to execute tests, but that quickly became tedious once 
all data types came into play. To simplify testing, I switched to converting the WAT tests into C++-based tests instead. 
This made test execution much easier to control and debug. I relied on AI tools to speed up the conversion of WAT 
tests to C++ test. 

In the end, the interpreter runs 191 out of 313 test cases where I achieved 189/210 on the first four test cases.

## Design Choices

- The architecture is split into two components: a Parser that creates a Module representation, and an Interpreter that executes it. 
This keeps the execution logic clean from the decoding logic and makes the code easier to read and understand.
- To avoid code duplication, I implemented core functionalities (load, store, push, pop) that are used for multiple 
datatypes using templates. This allowed for reuse for all of i32, i64, f32 and f64 ensuring maintainability and reduced
code duplication.
- The development process was driven entirely by the automated tests. The test suite provided immediate feedback on any 
changes, making debugging far more precise because I could quickly see which instructions or test cases were failing. 
Switching to code-based tests also made it much easier to verify that the expected results were implemented correctly,
without the need for a complex parsing logic.

## Implementation Status

### Fully Implemented
- Arithmetic: Full support for i32, i64, f32, and f64 arithmetic, bitwise, and comparison operations.
- Memory: Full support for load and store operations of all sizes. memory.size and memory.grow are also supported.
- Function Calls: Direct function calls, including argument passing and return value handling, are fully implemented. 

### Partially Implemented
- Parser: Handles sections: 1, 3, 5, 6, 7, 10 (Type, Function, Memory, Global (partially), Export, Code).
- Control Flow: The foundational logic for block, if, else, br, br_if, and return is implemented. The interpreter manages control. However, as noted, complex nested scenarios are not robust.

### Not Implemented
- br_table (switch-case instruction)
- call_indirect (dynamic function calls)

## Next Steps
The next improvements I would tackle at this point were:
- Data Section Support: Here I need to the implement parse_data_section in the parser and update the interpreter 
to copy this data into memory at startup.
- call_indirect: This is required for many test cases in file 3. The parser needs to handle the Table (section 4) and 
Element (section 9) sections, and the interpreter need to implement the call_indirect instruction (0x11). This instruction 
requires a lot of safety checking and validation.
- Control Flow: I would work on further debugging the control flow. Additionally, there are some parts, such as block 
results, that are not implemented yet.