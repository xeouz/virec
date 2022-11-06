import createGlobalModule from "./src/VIRELANG/VIRELANG.js";

async function loadModule(_callback)
{
    createGlobalModule().then((Module) => {
        GVireAPI=Module.VireAPI.loadFromText("extern puti(n:int) returns int; func main() returns int{let a:int = 10.6; puti(a);}", "wasm32");
        GModule=Module;
        _callback().then();
    });
}

async function run()
{
    await loadModule(async ()=>{
        console.log("Running VIRE-WASM...");
        
        let success;
        success=GVireAPI.parseSourceModule();
        console.log("Parse Success: ", success?"yes":"no");
        if(!success) return;
        
        success=GVireAPI.verifySourceModule();
        console.log("Verification Success: ", success?"yes":"no");
        if(!success) return;
        
        success=GVireAPI.compileSourceModule("", false);
        console.log("Compilation Success: ", success?"yes":"no");
        if(!success) return;
        
        let bin=GVireAPI.getByteOutput();
        let a=new Uint8Array(bin.size());

        for(let i=0; i<bin.size(); ++i)
        {
            a[i] = bin.get(i);
        }
        
        const importObject={
            env:{
                puti: (n)=>{console.log("Printing from JS: "+n)},
                __linear_memory:new WebAssembly.Memory({initial: 256, maximum:256}),
                __stack_pointer:new WebAssembly.Global({value: 'i32', mutable: true}, 16),
                __indirect_function_table:new WebAssembly.Table({initial:2, element:'anyfunc'})
            }
        }
        
        let m=await WebAssembly.instantiate(a, importObject)
        m.instance.exports.main()
    })
}

run().then()