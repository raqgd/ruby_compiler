.data
newline: .asciiz "\n"
.text
.globl main
main:
li $t0, 5
subu $sp, $sp, 4
sw $t0, 0($sp)
lw $t0, 0($sp)
la $t1, a
sw $t0, 0($t1)
addu $sp, $sp, 4
li $v0, 1
lw $a0, 0($sp)
syscall
li $v0, 4
la $a0, newline
syscall
addu $sp, $sp, 4
li $t0, 10
subu $sp, $sp, 4
sw $t0, 0($sp)
lw $t0, 0($sp)
la $t1, b
sw $t0, 0($t1)
addu $sp, $sp, 4
li $v0, 1
lw $a0, 0($sp)
syscall
li $v0, 4
la $a0, newline
syscall
addu $sp, $sp, 4
la $t0, a
lw $t1, 0($t0)
subu $sp, $sp, 4
sw $t1, 0($sp)
la $t0, b
lw $t1, 0($t0)
subu $sp, $sp, 4
sw $t1, 0($sp)
lw $t1, 0($sp)
addu $sp, $sp, 4
lw $t0, 0($sp)
addu $sp, $sp, 4
add $t2, $t0, $t1
subu $sp, $sp, 4
sw $t2, 0($sp)
lw $t0, 0($sp)
la $t1, c
sw $t0, 0($t1)
addu $sp, $sp, 4
li $v0, 1
lw $a0, 0($sp)
syscall
li $v0, 4
la $a0, newline
syscall
addu $sp, $sp, 4
li $v0, 10
syscall
