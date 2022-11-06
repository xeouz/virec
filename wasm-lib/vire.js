"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const VIRELANG_1 = __importDefault(require("./VIRELANG/VIRELANG"));
let GlobalModule;
let GlobalVireAPI;
const importObject = {
    env: {
        puti: (n) => { console.log(n); },
        __linear_memory: new WebAssembly.Memory({ initial: 256, maximum: 256 }),
        __stack_pointer: new WebAssembly.Global({ value: 'i32', mutable: true }, 16),
        __indirect_function_table: new WebAssembly.Table({ initial: 2, element: 'anyfunc' })
    }
};
async function loadMainModule(input_code = "", target_triple = "wasm32", _async_callback = () => { }) {
    (0, VIRELANG_1.default)().then((Module) => {
        GlobalModule = Module;
        GlobalVireAPI = GlobalModule.VireAPI.loadFromText(input_code, target_triple);
        _async_callback().then();
    });
}
async function compileSourceCodeFromAPI() {
    GlobalVireAPI.ParseSourceModule();
    GlobalVireAPI.VerifySourceModule();
    GlobalVireAPI.CompileSourceModule("", false);
}
async function instantiateOutputFromAPI() {
    let bin = GlobalVireAPI.getByteOutput();
    let arr = new Uint8Array(bin.size());
    for (let i = 0; i < bin.size(); ++i) {
        arr[i] = bin.get(i);
    }
    let module = await WebAssembly.instantiate(arr, importObject);
    return module;
}
async function setSourceCode(input_code) {
    GlobalVireAPI.setSourceCode(input_code);
    GlobalVireAPI.resetAST();
}
loadMainModule("extern puti(n:int) returns int; func main() returns int{let a=69; puti(a);}", "wasm32", async () => {
    await compileSourceCodeFromAPI();
    let m = await instantiateOutputFromAPI();
    let main = m.instance.exports.main;
    main();
});
