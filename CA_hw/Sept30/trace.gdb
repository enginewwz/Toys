# trace.gdb
set logging file trace_log.txt
set logging enabled on
set pagination off

b _start
r

while ($_thread != 0)
    echo \n------------------- STEP -------------------\n
    x/i $pc
    info registers
    stepi
end

set logging enabled off
quit