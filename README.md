# libraries 
## stb style MIT licensed C libraries 

This project inspired from [nothings's stb](https://github.com/nothings/stb). 
Following **Sean Barrett's** approach, this project aims to allow users easily 
include any of these libraries in their project. All the libraries have 
associated examples in the `./examples/` folder.

### Table of libraries
| Name              | Description                                                                                                                    | Examples                                   |
|-------------------|--------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| `arena_allocator` | This is a demo library trying to implement arena allocator to improve memory allocation in C.                                  | `./examples/arena_allocator.c`             |
| `rit_dyn_arr`     | This is a demo library trying to implement dynamic array in C, mimicking std::vector from C++.                                 | `./examples/rda.c`, `./examples/rda_aos.c` |
| `rit_str`         | This is a demo library trying to implement C++ like heap allocated strings and non owning string refereces(string views) in C. | `./examples/rstr.c`, `./examples/rsv.c`    |

### How to use these libraries
Copy and paste the needed library headers to your project. Then check out the 
examples associated with the library that you included in your project, to learn 
how to use that specifc library.

### Contribution
Feel free to open pull requests or issues to comment about any issue in the 
libraries, and the solution if you have one. Though I don't check **Github** daily,
so it may take time to approve your pull requests or check out your issues.

### Acknowledgements
Thanks to [Sean Barrett](https://github.com/nothings/) to teach me about how to
build single file libraries but also about licenses. I recommend to check out his
[stb](https://github.com/nothings/stb) repo to learn these yourself.
