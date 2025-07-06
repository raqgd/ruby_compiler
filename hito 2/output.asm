.data
newline: .asciiz "\n"
z: .space 256
y: .space 256
x: .space 256
.text
.globl main
main:
.data
string_label_0: .asciiz "Hola, "
.text
la $a0, string_label_0
li $v0, 4
syscall
lw $t0, 0($sp)
la $t1, x
sw $t0, 0($t1)
addu $sp, $sp, 4
.data
string_label_1: .asciiz "mundo!"
.text
la $a0, string_label_1
li $v0, 4
syscall
lw $t0, 0($sp)
la $t1, y
sw $t0, 0($t1)
addu $sp, $sp, 4
la $t0, x
lw $t1, 0($t0)
subu $sp, $sp, 4
sw $t1, 0($sp)
la $t0, y
lw $t1, 0($t0)
subu $sp, $sp, 4
sw $t1, 0($sp)
# Concatenar dos strings
lw $a1, 0($sp)
addu $sp, $sp, 4
lw $a0, 0($sp)
addu $sp, $sp, 4
li $v0, 9
li $a2, 256
syscall
move $t0, $v0
move $t1, $a0
move $t2, $t0
copy_loop_1:
lb $t3, 0($t1)
sb $t3, 0($t2)
beqz $t3, end_copy_1
addi $t1, $t1, 1
addi $t2, $t2, 1
j copy_loop_1
end_copy_1:
addi $t2, $t2, -1
move $t1, $a1
copy_loop_2:
lb $t3, 0($t1)
sb $t3, 0($t2)
beqz $t3, end_copy_2
addi $t1, $t1, 1
addi $t2, $t2, 1
j copy_loop_2
end_copy_2:
subu $sp, $sp, 4
sw $t0, 0($sp)
lw $t0, 0($sp)
la $t1, z
sw $t0, 0($t1)
addu $sp, $sp, 4
li $v0, 10
syscall
