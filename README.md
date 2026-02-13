# `Vire Programming Language`

**Vire** is a custom programming language I designed and implemented, featuring a complete compilation pipeline targeting **WebAssembly** and **LLVM**.

```rust
extern puti(n: int);

puti(42); 
```

- Run it directly in your browser: [Vire Web Demo](https://vire-lang.web.app/)  
- Source code for website frontend: [Vire Web Frontend Code](https://github.com/xeouz/virelang-website)
- [Vire Syntax Guide](https://docs.google.com/document/d/1rOWaFbiwEMJ1P3Kho6rANaR0Qqln93tmBB4grQYO7VM/edit?usp=sharing)

## `Language Features 🛠️`

- **Primitive Types**: `bool`, `int`, `double`, `float`, and `char` with support for type casting.
- **Data Structures**: `structs` with dot-member access and `Array` indexing.
- **Control Flow**: `if`/`else-if`/`else` blocks, `for`/`while` loops, and `break`/`continue` statements.
- **Interoperability**: C-style interop via the `extern` keyword.

## `Technical Overview 💻`

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Backend** | LLVM | Handles IR generation and optimization. |
| **Frontend** | Custom Parser | Manual implementation of semantic analysis and error reporting. |
| **Web Port** | WASM | Ported via `Emscripten` for browser execution. |
| **Architecture** | C++ | ~7,500 lines focusing on `unique_ptr` and `RAII`. |

## `Code Snippet 📜`
A quick demo about this language. Implementing the Square Root Algorithm.
```rust
extern puti(n: int);

struct Vector {
    float x;
    float y;
}

func sqrt(n: float) returns float {
    if (n < 0) { return 0.0; }

    let x = n;
    let y = 1.0;
    let precision = 0.000001;

    while((x - y) > precision) {
        x += y;
        x /= 2.0;
        y = n / x;
    }

    return x;
}

let vec: Vector;
vec.x = 10.5;
vec.y = 20.4;

let squared_sum = (vec.x * vec.x + vec.y * vec.y);
let magnitude_of_vector = sqrt(squared_sum);
if (magnitude_of_vector > 0.0) {
    vec.x /= magnitude_of_vector;
    vec.y /= magnitude_of_vector;
}

vec.x *= 100;
vec.y *= 100;
puti(vec.x);
puti(vec.y);
```

## `Prerequisites 📋`
### System Dependencies:
- **LLVM**: Verified on **LLVM 23.0.0-git**. (Should support 18.0+ with minor adjustments, but 23.0 is recommended for the current build)
- Ensure `llvm-config` is in your PATH.
- **CMake & Ninja**: For high-speed native builds.
- **Clang/GCC**: A modern compiler supporting C++20.
- **Emscripten**: (Optional) Required only for WASM builds.
### Python Environment: 
The build.py script requires Python 3.10+ and the `halo` module for build status UI.
```bash
pip install halo
```

## `Build & Tooling ⚙️`
| Target | Command | Description |
| :--- | :--- | :--- |
| **Native** | `python3 build.py --compile` | Default native build via `CMake` and `Ninja`. |
| **Web** | `python3 build.py --wasm` | Cross compiles to `WASM` with `Emscripten` |
| **Debug** | `python3 build.py --debug` | Runs the build with `Valgrind` for memory leak analysis. |

*This script handles automated cache clearing, file compression, and cross-compilation linking for the LLVM-WASM backend.*

## `Final Thoughts ✉️`

I’m looking for technical feedback or criticism regarding the architecture. If you have pointers on making the design more modular or industry-standard, feel free to reach out!
