struct Range:
    start: i64 = 0
    end: i64 = 0
    step: i64 = 1

def range(a: i64, b: i64 default, c: i64 default) -> Range:
    if b is default:
        return Range(end=a)
    elif c is default:
        return Range(start=a, end=b)
    else:
        return Range(start=a, end=b, step=c)

def fibo(n: i64) -> i64:
    let mut a = 0, mut b = 1
    for _ in range(n):
        a, b = b, a+b
    return a

print(fibo(1))

def nop() -> ():
    return