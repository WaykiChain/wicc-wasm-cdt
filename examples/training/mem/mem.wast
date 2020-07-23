(module
  (type (;0;) (func (param i32 i32 i32)))
  (type (;1;) (func (param i64)))
  (type (;2;) (func (param i32 i32)))
  (type (;3;) (func (param i32 i32 i32) (result i32)))
  (type (;4;) (func (result i32)))
  (type (;5;) (func (param i32 i32) (result i32)))
  (type (;6;) (func))
  (type (;7;) (func (param i32) (result i32)))
  (type (;8;) (func (param i32)))
  (type (;9;) (func (param i64 i64 i64)))
  (type (;10;) (func (param i64 i64 i32) (result i32)))
  (import "env" "printui" (func (;0;) (type 1)))
  (import "env" "wasm_assert" (func (;1;) (type 2)))
  (import "env" "memset" (func (;2;) (type 3)))
  (import "env" "action_data_size" (func (;3;) (type 4)))
  (import "env" "read_action_data" (func (;4;) (type 5)))
  (import "env" "memcpy" (func (;5;) (type 3)))
  (func (;6;) (type 6)
    call 9)
  (func (;7;) (type 7) (param i32) (result i32) ;;malloc
    (local i32 i32 i32)
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            get_local 0
            i32.eqz
            br_if 0 (;@4;)        ;;新申请size==0, 转65退出
            i32.const 0
            i32.const 0
            i32.load offset=8204  ;;[8204]页数
            get_local 0
            i32.const 16
            i32.shr_u
            tee_local 1           ;;[1]=size>>16, size的页数(64k一页)
            i32.add
            tee_local 2           ;;[2]=[8204]+size>>16, 新的总页数
            i32.store offset=8204 ;;[8204]=size>>16
            i32.const 0
            i32.const 0
            i32.load offset=8196  ;;[8196]当前内地字节数
            tee_local 3           ;;[3]=当前内地字节数
            get_local 0           ;;[0]新申请size
            i32.add               ;;新申请总字节数=[3]+[0]
            i32.const 7
            i32.add
            i32.const -8
            i32.and
            tee_local 0           ;;[0]=(总字节数+7)/8, 8字节对齐
            i32.store offset=8196 ;;[8196]=[0]
            get_local 2
            i32.const 16
            i32.shl               ;;[2]<<16, 总页数所能容纳的字节数
            get_local 0
            i32.le_u
            br_if 1 (;@3;)        ;;总页数所能容纳的字节数<新申请总字节数, 转68增加一页
            get_local 1
            memory.grow           ;;申请[1]页数
            i32.const -1
            i32.eq
            br_if 2 (;@2;)        ;;失败转81
            br 3 (;@1;)           ;;成功转87
          end
          i32.const 0
          return                  ;;返回值0
        end
        i32.const 0
        get_local 2
        i32.const 1
        i32.add
        i32.store offset=8204     ;;[8204]=[2]+1
        get_local 1
        i32.const 1
        i32.add
        memory.grow               ;;申请[1]+1
        i32.const -1
        i32.ne
        br_if 1 (;@1;)
      end
      i32.const 0
      i32.const 8208
      call 1
      get_local 3
      return
    end
    get_local 3)
  (func (;8;) (type 8) (param i32)) ;;free
  (func (;9;) (type 6)
    (local i32)
    get_global 0
    i32.const 16
    i32.sub
    tee_local 0           ;;[0]=[global0] - 16
    i32.const 0
    i32.store offset=12   ;;[0]+12=0
    i32.const 0
    get_local 0
    i32.load offset=12  
    i32.load              ;;[地址0]已用的内存字节数total_bytes_size入栈
    i32.const 7
    i32.add               ;;total_bytes_size+7
    i32.const -8     
    i32.and               ;;size_and_minus_8= (total_bytes_size+7)&-8
    tee_local 0           ;;[0]=size_and_minus_8
    i32.store offset=8196 ;;[8196]=size_and_minus_8
    i32.const 0
    get_local 0
    i32.store offset=8192 ;;[8192]=size_and_minus_8
    i32.const 0
    memory.size
    i32.store offset=8204);;[8204]=内存页数
  (func (;10;) (type 8) (param i32))
  (func (;11;) (type 0) (param i32 i32 i32) ;;prints
    get_local 0
    get_local 2
    i32.store offset=32    ;;inst.balance2=[0]
    get_local 0
    get_local 1
    i32.store offset=28    ;;inst.balance1=[1]
    get_local 1
    i64.extend_u/i32
    call 0                 ;;打印i64(b1)
    get_local 0
    i64.load32_u offset=32 ;;inst + 32(8(_self) + 8(_first_receiver) + 4(_ds) + 4(balance1))
    call 0)                ;;打印i64(balance2)
  (func (;12;) (type 9) (param i64 i64 i64) ;;apply
    (local i32)           ;;栈里有4个元素
    get_global 0
    i32.const 16
    i32.sub
    tee_local 3
    set_global 0          ;;函数局部内存(栈)8k, 从大到小global0表示首地址, apply用16字节, [global0]=[3]=8192-16=8176
    call 6                ;;设置当前页数和已用内存数
    block  ;; label = @1
      get_local 1
      get_local 0
      i64.ne
      br_if 0 (;@1;)      ;;receiver!=code, 转160退出
      get_local 2
      i64.const -5918507402524622848
      i64.ne
      br_if 0 (;@1;)      ;;action!=-5918507402524622848(prints), 转160退出
      get_local 3
      i32.const 0
      i32.store offset=12 ;;[[3]+12]=0
      get_local 3
      i32.const 1
      i32.store offset=8  ;;[[3]+8]=1
      get_local 3
      get_local 3
      i64.load offset=8
      i64.store           ;;[[3]]=1, 函数间接地址1, 通过elem转换指向11号函数
      get_local 1
      get_local 1
      get_local 3         ;;保存有函数间接地址1的[3](入栈)
      call 13             
      drop
    end
    i32.const 0
    call 10
    get_local 3
    i32.const 16
    i32.add
    set_global 0)          ;;[global0]=[3]+16=8192
  (func (;13;) (type 10) (param i64 i64 i32) (result i32) ;;execute_action
    (local i32 i32 i32 i32 i32 i32 i32)
    get_global 0
    i32.const 64
    i32.sub
    tee_local 3
    set_local 4
    get_local 3
    set_global 0          ;;execute_action用到64字节, [global0]=[3]=[4]=8176-64=8112
    get_local 2
    i32.load offset=4
    set_local 5           ;;[5]=0
    get_local 2
    i32.load
    set_local 6 ;;[6]=函数间接地址1
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            call 3
            tee_local 7   ;;[7]=action_data_size
            i32.eqz
            br_if 0 (;@4;);;action_data_size==0, 转198
            get_local 7
            i32.const 513
            i32.lt_u
            br_if 1 (;@3;);;action_data_size<=512, 转202, 新增局部内存分配buffer
            get_local 7
            call 7
            set_local 2   ;;[2]=buffer首地址
            br 2 (;@2;)   ;;转212, 读取action_data
          end
          i32.const 0
          set_local 2     ;;[2]=0, 不用读取action_data, 转217反序列化
          br 2 (;@1;)
        end
        get_local 3
        get_local 7
        i32.const 15
        i32.add
        i32.const -16
        i32.and
        i32.sub
        tee_local 2
        set_global 0      ;;[2]=[global0]=[3]-(action_data_size+15)&-16, 新增局部内存分配buffer
      end
      get_local 2
      get_local 7
      call 4              ;;读取[7]大小的action_data到[2](buffer) 
      drop                ;;返回值出栈
    end
    get_local 4           ;;从buffer反序列化到指定内存
    i64.const 0
    i64.store offset=56   ;;[[4]+56] = 0
    block  ;; label = @1
      get_local 7
      i32.const 3
      i32.gt_u
      br_if 0 (;@1;)      ;;action_data_size>3, 转229反序列化b1
      i32.const 0
      i32.const 8233
      call 1              ;;action_data_size<=3, assert("read")
    end
    get_local 2
    get_local 7
    i32.add
    set_local 3           ;;[3]=buffer+size
    get_local 4
    i32.const 56
    i32.add
    i32.const 4
    i32.or                ;;([4]+56)|4
    set_local 8           ;;[8]=([4]+56)|4=[4]+60
    get_local 4
    i32.const 56
    i32.add               ;;[4]+56
    get_local 2           
    i32.const 4
    call 5                ;;[[4]+56]=buffer第一个4字节
    drop
    get_local 2
    i32.const 4
    i32.add
    set_local 9           ;;[9]指向buffer第二个4字节
    block  ;; label = @1
      get_local 7
      i32.const -4
      i32.and
      i32.const 4
      i32.ne
      br_if 0 (;@1;)      ;;action_data_size&-4 == 4, 转261读取b2
      i32.const 0
      i32.const 8233
      call 1              ;;action_data_size&-4 != 4,, assert("read")
    end
    get_local 8
    get_local 9
    i32.const 4
    call 5                 ;;[[4]+60]=buffer第二个4字节
    drop
    get_local 4            ;;开始分配类内存
    i32.const 24
    i32.add               
    get_local 3
    i32.store              ;;[[4]+24] = buffer+size
    get_local 4
    get_local 2
    i32.store offset=16    ;;[[4]+16] = buffer
    get_local 4
    i32.const 20
    i32.add
    get_local 2
    i32.const 8
    i32.add
    i32.store              ;;[[4]+20] = buffer+8
    get_local 4
    get_local 1
    i64.store offset=8     ;;[[4]+8]=code
    get_local 4
    get_local 0
    i64.store              ;;[[4]]=self
    get_local 4
    get_local 5
    i32.const 1
    i32.shr_s
    i32.add
    set_local 3            ;;[3]=[4]+0, [3]就是inst起始地址
    get_local 4
    i32.load offset=60
    set_local 8            ;;[8]=[[4]+60]
    get_local 4
    i32.load offset=56
    set_local 9            ;;[9]=[[4]+56]
    block  ;; label = @1
      get_local 5
      i32.const 1
      i32.and
      i32.eqz
      br_if 0 (;@1;)       ;;[5]==0, 转312执行1=>11号函数
      get_local 3
      i32.load
      get_local 6
      i32.add
      i32.load
      set_local 6          ;;[5]!=0, [6]=[3].[6]?
    end
    get_local 3
    get_local 9
    get_local 8
    get_local 6
    call_indirect (type 0) ;;执行11号函数, [3]inst,[9]self,[8]code,[6]函数间接地址1=>11号函数
    block  ;; label = @1
      get_local 7
      i32.const 513
      i32.lt_u
      br_if 0 (;@1;)       ;;action_data_size<=512, 转325
      get_local 2 
      call 8               ;;free(buffer)
    end
    get_local 4
    i32.const 64
    i32.add
    set_global 0           ;;[global0]=[4]+64, 恢复局部内存
    i32.const 1)           ;;return true
  (table (;0;) 2 2 anyfunc)
  (memory (;0;) 1)
  (global (;0;) (mut i32) (i32.const 8192))
  (global (;1;) i32 (i32.const 8238))
  (global (;2;) i32 (i32.const 8238))
  (export "apply" (func 12))
  (elem (i32.const 1) 11)
  (data (i32.const 8208) "failed to allocate pages\00read\00")
  (data (i32.const 0) "0 \00\00"))
