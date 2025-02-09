.global _start
_start:

#    la t0, test1
#    li a7, 0
#    li a0, 0
#    jal ra, test1
#    addi a0, a0, 1
# continue:
#    ecall


# test1:
#    jalr  t0, 4(ra)
#    ecall

    add t0, t0, t0
    ecall