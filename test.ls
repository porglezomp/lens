def foo(x: i64) -> i64:
    return x * 2 + 3

def bar(x: i64, y: i64) -> i64:
    let a = x * y
    let b = x + y
    let c = a - b * x
    let d = c - b * 2 + a / 2
    let e = d - c - b - a
    return (e + a) * x

def fib(n: i64) -> i64:
    if n == 0:
        return 0
    else:
        if n == 1:
            return 1
        else:
            return fib(n - 1) + fib(n - 2)

printi64(fib(13))
