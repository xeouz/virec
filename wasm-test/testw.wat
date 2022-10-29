(module
  (type (;0;) (func (result i32)))
  (type (;1;) (func (param i32) (result i32)))
  (type (;2;) (func (param i32 i32) (result i32)))
  (import "env" "__linear_memory" (memory (;0;) 0))
  (import "env" "__stack_pointer" (global (;0;) (mut i32)))
  (import "env" "puti" (func (;0;) (type 1)))
  (import "env" "__indirect_function_table" (table (;0;) 0 funcref))
  (func (;1;) (type 0) (result i32)
    (local i32)
    global.get 0
    i32.const 16
    i32.sub
    local.tee 0
    i32.const 1
    i32.store offset=12
    local.get 0
    i32.load offset=12)
  (func (;2;) (type 0) (result i32)
    (local i32 i32)
    global.get 0
    i32.const 16
    i32.sub
    local.tee 0
    global.set 0
    local.get 0
    i32.const 10
    i32.store offset=8
    i32.const 10
    call 0
    drop
    local.get 0
    i32.const 0
    i32.store offset=12
    local.get 0
    i32.load offset=12
    local.set 1
    local.get 0
    i32.const 16
    i32.add
    global.set 0
    local.get 1)
  (func (;3;) (type 2) (param i32 i32) (result i32)
    call 2)
  (export "test" (func 1))
  (export "main" (func 2)))
