.data
_intout: .asciz "%ld "
_strout: .asciz "%s "
_errout: .asciz "Wrong number of arguments"
STR0: .string "a is"
STR1: .string "and b is"
STR2: .string "<<"
STR3: .string "="
STR4: .string ">>"
STR5: .string "="
.data
.globl _main
.text
_main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$1, %rdi
	cmpq	$2,%rdi
	jne		ABORT
	cmpq	$0, %rdi
	jz		SKIP_ARGS
	movq	%rdi, %rcx
	addq	$16, %rsi
PARSE_ARGV:
	pushq	%rcx
	pushq	%rsi
	movq	(%rsi), %rdi
	movq	$0, %rsi
	movq	$10, %rdx
	call	_strtol
	popq	%rsi
	popq	%rcx
	pushq	%rax
	subq	$8, %rsi
	loop	PARSE_ARGV
	popq	%rdi
	popq	%rsi
SKIP_ARGS:
	call	__bitwise_operators
	jmp		END
ABORT:
	leaq	_errout(%rip), %rdi
	call	_puts
END:
	movq	%rax, %rdi
	andq	$-16, %rsp
	call	_exit
__bitwise_operators:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rdi
	pushq	%rsi
	subq	$8, %rsp
	subq	$8, %rsp
	leaq	STR0(%rip), %rsi
	leaq	_strout(%rip), %rdi
	call	_printf
	movq	-8(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	leaq	STR1(%rip), %rsi
	leaq	_strout(%rip), %rdi
	call	_printf
	movq	-16(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	movq	$'\n', %rdi
	call	_putchar
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	-16(%rbp), %rax
	movb	%al, %cl
	shlq	%cl, (%rsp)
	popq	%rax
	movq	%rax, -24(%rbp)
	movq	-8(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	leaq	STR2(%rip), %rsi
	leaq	_strout(%rip), %rdi
	call	_printf
	movq	-16(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	leaq	STR3(%rip), %rsi
	leaq	_strout(%rip), %rdi
	call	_printf
	movq	-24(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	movq	$'\n', %rdi
	call	_putchar
	movq	$40, %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	movq	$'\n', %rdi
	call	_putchar
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	-16(%rbp), %rax
	movb	%al, %cl
	shrq	%cl, (%rsp)
	popq	%rax
	movq	%rax, -24(%rbp)
	movq	-8(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	leaq	STR4(%rip), %rsi
	leaq	_strout(%rip), %rdi
	call	_printf
	movq	-16(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	leaq	STR5(%rip), %rsi
	leaq	_strout(%rip), %rdi
	call	_printf
	movq	-24(%rbp), %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	movq	$'\n', %rdi
	call	_putchar
	movq	$2, %rsi
	leaq	_intout(%rip), %rdi
	call	_printf
	movq	$'\n', %rdi
	call	_putchar
	movq	$0, %rax
	leave
	ret
