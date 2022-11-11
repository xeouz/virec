import createGlobalModule from "./VIRELANG.js"

async function loadModuleAsync()
{
    createGlobalModule().then(async(Module)=>{
        let api=Module.VireAPI.loadFromText("", "wasm32");

        return {'module': Module, 'api': api};
    })
}