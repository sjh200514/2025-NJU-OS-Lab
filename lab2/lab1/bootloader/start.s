/* Protected Mode Loading Hello World APP */
.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	#TODO: Protected Mode Here
  cli 
  inb $0x92, %al
  orb $0x02, %al
  outb %al, $0x92

  lgdt gdtDesc

  movl %cr0, %eax
  orl $1, %eax
  movl %eax, %cr0
  
  ljmp $0x08, $start32

.code32
start32:
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	
	movl $0x8000, %eax # setting esp
	movl %eax, %esp
	jmp bootMain # jump to bootMain in boot.c

.p2align 2
gdt: 
	#GDT definition here
  .word 0x0000
  .word 0x0000
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00

  .word 0xFFFF
  .word 0x0000
  .byte 0x00
  .byte 0x9A
  .byte 0xCF
  .byte 0x00

  .word 0xFFFF
  .word 0x0000
  .byte 0x00
  .byte 0x92
  .byte 0xCF
  .byte 0x00

  .word 0xFFFF
  .word 0x8000
  .byte 0x0B
  .byte 0x92
  .byte 0x40
  .byte 0x00

gdtDesc: 
	#gdtDesc definition here
  .word gdtDesc - gdt -1
  .long gdt
