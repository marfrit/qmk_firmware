loop:
.wrap_target
jmp    pin, checkosr
read:
setbitcounterin:
set    x, 10
readbit:
wait   0 pin, 1
in     pins, 1
wait   1 pin, 1
jmp    x--, readbit
jmp    loop
checkosr:
jmp    !osre, write
jmp    loop
write:
set    pindirs, 1             [31]
set    pindirs, 0             [2]
set    pindirs, 2
wait   0 pin, 1
setbitcounterout:
set    x, 9
writebit:
out    pindirs, 1
wait   1 pin, 1
wait   0 pin, 1
jmp    x--, writebit
release:
set    pindirs, 3
waitack:
wait   0 pin, 1
wait   1 pin, 1
flagreply:
set    y, 1
mov    isr, ::y
.wrap
