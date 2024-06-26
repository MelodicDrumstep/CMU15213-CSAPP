# Archlab

这个 lab 分为 3 个部分， Part A 是写 y86 汇编， Part B 是写 HCL, Part C 是利用 CPU 流水线优化汇编。

### Part A

就是写这个 3 个 C 函数的对应汇编：

```c
/* sum_list - Sum the elements of a linked list */
long sum_list(list_ptr ls)
{
    long val = 0;
    while (ls) {
	val += ls->val;
	ls = ls->next;
    }
    return val;
}

/* rsum_list - Recursive version of sum_list */
long rsum_list(list_ptr ls)
{
    if (!ls)
	return 0;
    else {
	long val = ls->val;
	long rest = rsum_list(ls->next);
	return val + rest;
    }
}

/* copy_block - Copy src to dest and return xor checksum of src */
long copy_block(long *src, long *dest, long len)
{
    long result = 0;
    while (len > 0) {
	long val = *src++;
	*dest++ = val;
	result ^= val;
	len--;
    }
    return result;
}
```

写出的对应汇编是这样：

sum.ys:

```asm
.pos 0                  ;主程序入口
irmovq stack, %rsp      ;把栈的地址放到 rsp 里面
call main               ;调用 main 然后 halt
halt

.align 8                ;确保下面从 8 字节对齐的位置开始
ele1:
    .quad 0x00a         ;存值 0x00a
    .quad ele2          ;存ele2 这个 symbol 的地址
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0             ;存 0 地址

main:
    irmovq ele1, %rdi   ;把 ele 地址赋给 rdi 寄存器作为参数
    call sum_l          ;调用 sum_l
    ret

sum_list:
    pushq %r8           ;因为之后要用到 r8 寄存器，所以先入栈保存一下
    xorq %rax, %rax     ;初始化 rax 为 0. xorq src des, 结果存在 des。

loop:
    andq %rdi, %rdi     ;看一下 rdi 是否为 0
    je loopend          ;如果是， 结束循环
    mrmovq (%rdi), %r8  ;否则把 rdi 所存的地址中的值取出来赋给 r8 寄存器。  mrmovq (src) des, src 是内存地址， des 是一个寄存器
    addq %r8, %rax      ;rax += r8, 表示存结果
    mrmovq 8(%rdi), %rdi;取链表 next
    jmp loop            ;继续循环

loopend:
    popq %r8            ;把存的 r8 pop 出来
    ret

.pos 0x200              ;栈的位置
stack:
```

rsum:

```asm
.pos 0
irmovq stack, %rsp
call main
halt

.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0

main:
    irmovq ele1, %rdi
    call rsum_list
    ret

rsum_list:
    pushq %r8               ;push r8
    andq %rdi, %rdi         ;检查 rdi 是否为 0
    jne recursive           ;如果不是 0, 跳转 recursive
    xorq %rax, %rax         ;返回值设置成 0
    popq %r8                ;取出 r8
    ret

recursive:
    mrmovq (%rdi), %r8      ;取值到 r8
    mrmovq 8(%rdi), %rdi    ;递归参数是 next 地址
    call rsum_list          ;递归
    addq %r8, %rax          ;最后把 r8 和递归的返回值加起来

.pos 0x200                  
stack:       
```

copy.ys:

```asm
.align 8
 # Source block
 src:
    .quad 0x00a
    .quad 0x0b0
    .quad 0xc00
 # Destination block
 dest:
    .quad 0x111
    .quad 0x222
    .quad 0x333

.pos 0
irmovq stack, %rsp      
call main
halt

main:
    irmovq src, %rdi        ;第一个参数设成 src   
    irmovq dest, %rsi       ;第二个 dest
    irmovq $3, %rdx         ;第三个 3， 表示长度
    call copy
    ret

copy:
    pushq %r8
    pushq %r9
    pushq %r10              ;把要用到的寄存器都备份到栈里面
    irmovq $1, %r9          
    irmovq $8, %r8          ;r9 = 1, r8 = 8
    xorq $rax, $rax         ;rax = 0

loop:
    andq %rdx, %rdx         
    jle loopend             ;if(rdx > 0)
    mrmovq (%rdi), %r10     ;r10 = *rdi, 也就是 src 的东西
    addq %r8, %rdi          ;rdi += 8
    addq %r8, %rsi          ;rsi += 8
    rmmovq %r10, (%rsi)     ;rsi 的地址上面填 r10
    xorq %r10, %rax         ;rax 存返回值（所有异或起来）
    subq %r9, %rdx          ;rdx -=1
    jmp loop

loopend:
    popq %r10
    popq %r9
    popq %r8                ;结束了之后恢复所有寄存器
    ret

.pos 0x200
stack:
```

这部分还是很简单的。

### Part B

这部分就是简单改一改那个 seq-full.hcl。 感觉要自己写的地方很少很少。

### Part C

这部分是优化这个函数：

```asm
	xorq %rax,%rax		# count = 0;
	andq %rdx,%rdx		# len <= 0?
	jle Done		# if so, goto Done:

Loop:	
	mrmovq (%rdi), %r10	# read val from src...
	rmmovq %r10, (%rsi)	# ...and store it to dst
	andq %r10, %r10		# val <= 0?
	jle Npos		# if so, goto Npos:
	irmovq $1, %r10
	addq %r10, %rax		# count++
Npos:	irmovq $1, %r10
	subq %r10, %rdx		# len--
	irmovq $8, %r10
	addq %r10, %rdi		# src++
	addq %r10, %rsi		# dst++
	andq %rdx,%rdx		# len > 0?
	jg Loop			# if so, goto Loop:
```

首先在 PartB 里面， 我们新加了一个指令 iaddq， 可以直接把 Intermediate 加到寄存器里面， 所以先把能替换的地方都替换了：


把

```asm
irmovq $1, %r10
addq %r10, %rax		# count++
```

改成

```asm
iaddq $1, %rax
```

之后， 主要优化点在于循环展开。 这里循环展开有很多好处

+ 利于 CPU 流水线

+ 减少 `iaddq` `src` 和 `dst` 的指令数。

当然， 循环展开也不是越多越好， 如果展开层数太多， 则指令总长度太大， 会增加加载指令的体积， 指令的缓存命中率下降。

最后大概是这样：


```asm
Loop:	
	mrmovq (%rdi), %rcx
	mrmovq 8(%rdi), %r8
	mrmovq 16(%rdi), %r9
    mrmovq 24(%rdi), %r10
    mrmovq 32(%rdi), %r11
    mrmovq 40(%rdi), %r12 

	rmmovq %rcx, (%rsi)
    rmmovq %r8, 8(%rsi)
    rmmovq %r9, 16(%rsi)
    rmmovq %r10, 24(%rsi)
    rmmovq %r11, 32(%rsi)
    rmmovq %r12, 40(%rsi)

check0:
	andq %rcx, %rcx
	jle check1
	iaddq $1, %rax

check1:
	andq %r8, %r8
	jle check2
	iaddq $1, %rax

check2:
	andq %r9, %r9
	jle check3
	iaddq $1, %rax

check3:
	andq %r10, %r10
	jle check4
	iaddq $1, %rax

check4:
	andq %r11, %r11
	jle check5
	iaddq $1, %rax

check5:
	andq %r12, %r12
	jle check3
	iaddq $1, %rax

loopback:
	# src += 6, dst += 6
	# len -= 6, if > 0, loop again
	iaddq $48, %rdi
	iaddq $48, %rdi
	iaddq $-6, %rdx
	jge Loop

# Deal with the elements less than 6
Rest:
	iaddq $9, %rdx
	je Done
```