set -e
flag=$1

f() {
    times=$1
    yes ./ot both $flag -m $times | head -n 100 | bash | tee ot-${times}${flag}.log
}

f 100000
f 1000000
f 10000000
