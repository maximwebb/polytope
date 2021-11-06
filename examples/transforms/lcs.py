from utils import print_arr


def lcs(u, v):
    m = len(u)
    n = len(v)
    a = [[0 for _ in range(n+1)] for _ in range(m+1)]

    for i in range(1, m+1):
        for j in range(1, n+1):
            a[i][j] = max(a[i-1][j],
                          a[i][j-1],
                          a[i-1][j-1] + (1 if u[i-1] == v[j-1] else 0))
    return a[-1][-1]


def fast_lcs(u, v):
    m = len(u)
    n = len(v)
    a = [[0 for _ in range(n+1)] for _ in range(m+1)]

    for i in range(2, m+n+3):
        for j in range(max(1, i-m), min(n+1, i)):
            a[i-j][j] = max(a[i-j-1][j],
                            a[i-j][j-1],
                            a[i-j-1][j-1] + (1 if u[i-j-1] == v[j-1] else 0))
    return a[-1][-1]
