import { loadModuleAsync } from "./wrapper/vire-wrapper";

loadModuleAsync().then(async(info)=>{
    console.log(info.module);
})