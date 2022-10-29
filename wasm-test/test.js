const fs=require('fs');
let wasmExports=null;

/*
let wasmMemory=new WebAssembly.Memory({initial:256, maximum:256});
let wasmTable=new WebAssembly.Table(
    {
        'initial':1,
        'maximum':1,
        'element':'anyfunc'
    }
);

let asmLibraryArg={
    "__handle_stack_overflow":()=>{},
    "emscripten_resize_heap":()=>{},
    "__lock":()=>{},
    "__unlock":()=>{},
    "memory":wasmMemory,
    "table":wasmTable
};

var info={
    "env":asmLibraryArg,
    "wasi_snapshot_preview1":asmLibraryArg
}
*/

const putd = (int) =>
{
    console.log(int);
};

const importObject={
    env:{
        putd: putd,
        __linear_memory:new WebAssembly.Memory({initial: 256, maximum:256}),
        __stack_pointer:new WebAssembly.Global({value: 'i32', mutable: true}, 16),
        __indirect_function_table:new WebAssembly.Table({initial:2, element:'anyfunc'})
    }
};

async function loadWasm()
{
    let src=fs.readFileSync('./test.wasm');
    let bytes=new Uint8Array(src);

    let wasmObj=await WebAssembly.instantiate(bytes, importObject);
    wasmObj.instance.exports.main();
}

loadWasm();