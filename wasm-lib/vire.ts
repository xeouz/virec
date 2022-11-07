import { createGlobalModule } from "./vire-emcc";

let GlobalModule :any;
let GlobalVireAPI:any;

const importObject:WebAssembly.Imports = {
    env:{
        puti: (n:number)=>{console.log(n)},
        __linear_memory:new WebAssembly.Memory({initial: 256, maximum:256}),
        __stack_pointer:new WebAssembly.Global({value: 'i32', mutable: true}, 16),
        __indirect_function_table:new WebAssembly.Table({initial:2, element:'anyfunc'})
    }
}

export async function loadMainModule(input_code:string = "", target_triple:string = "wasm32", _async_callback:Function = ()=>{}) {
    createGlobalModule().then((Module: any)=>{
        GlobalModule=Module;
        GlobalVireAPI=GlobalModule.VireAPI.loadFromText(input_code, target_triple);
        _async_callback().then();
    })
}

export async function compileSourceCodeFromAPI() :Promise<void> {
    GlobalVireAPI.ParseSourceModule();
    GlobalVireAPI.VerifySourceModule();
    GlobalVireAPI.CompileSourceModule("", false);
}

export async function instantiateOutputFromAPI() :Promise<WebAssembly.WebAssemblyInstantiatedSource> {
    let bin:any = GlobalVireAPI.getByteOutput();
    let arr:Uint8Array = new Uint8Array(bin.size());

    for(let i:number = 0; i<bin.size(); ++i)
    {
        arr[i] = bin.get(i);
    }

    let module=await WebAssembly.instantiate(arr, importObject);
    return module;
}

export async function setSourceCode(input_code:string) {
    GlobalVireAPI.setSourceCode(input_code);
    GlobalVireAPI.resetAST();
}