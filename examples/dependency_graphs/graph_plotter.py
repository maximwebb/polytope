import copy
import tkinter
from tkinter import ttk

width = 20

class Graph:
    def __init__(self, w=None, h=None, a1=None, a2=None):
        self.V = []
        self.E = []
        if w is not None:
            if h is None:
                h = w
            if a1 is None:
                a1 = 0
            if a2 is None:
                a2 = 0
            for i in range(a1, a1 + w):
                for j in range(a2, a2 + h):
                    self.V.append((i, j))

    def add_vertex(self, v):
        self.V.append(v)

    def add_edge(self, u, v):
        self.E.append((u, v))


def gen_dep_graph(n, deps):
    g = Graph(n)
    for f in deps:
        for i in range(n):
            for j in range(n):
                v_i, v_j = f(i, j)
                if 0 <= v_i < n and 0 <= v_j < n:
                    g.add_edge((v_i, v_j), (i, j))
    return g


def gen_graph(a1, a2, b1, b2):
    g = Graph((b1 - a1), (b2 - a2), a1, a2)
    return g


def transform_graph(T, g):
    g1 = Graph()
    g1.V = [(T[0][0] * x + T[0][1] * y, T[1][0] * x + T[1][1] * y) for (x, y) in g.V]
    return g1


class Canvas:
    def __init__(self, n):
        self.n = n
        self.g = None
        self.g_init = None
        self.root = tkinter.Tk()
        self.width = width * n + 100
        self.height = width * n + 100
        # create canvas
        self.c = tkinter.Canvas(self.root, bg="white", height=self.height, width=self.width)

        # Create axes/labels/gridlines
        l1 = self.c.create_line(*self.convert_point(0, 0), *self.convert_point(0, self.n), arrow=tkinter.LAST)
        l2 = self.c.create_line(*self.convert_point(0, 0), *self.convert_point(self.n, 0), arrow=tkinter.LAST)
        for i in range(self.n):
            x_label = self.convert_point(i, -0.5)
            y_label = self.convert_point(-0.5, i)
            self.c.create_text(*x_label, text=f"{i}")
            self.c.create_text(*y_label, text=f"{i}")

            self.c.create_line(*self.convert_point(i, 0), *self.convert_point(i, self.n), fill='#e8e8e8')
            self.c.create_line(*self.convert_point(0, i), *self.convert_point(self.n, i), fill='#e8e8e8')

    def set_graph(self, g):
        self.c.delete("point")
        self.c.delete("arrow")
        self.g = g
        if self.g_init is None:
            self.g_init = g
        for v in g.V:
            self.add_point(v)

        for (u, v) in g.E:
            self.add_arrow(u, v)

    def convert_point(self, x, y):
        x = width + x * width
        y = self.width - width - (y * width)
        return x, y

    def add_point(self, v):
        w = 4
        x, y = v
        x, y = self.convert_point(x, y)

        l = self.c.create_oval(x - w, y - w, x + w, y + w, fill="red", tags="point")

    def add_arrow(self, u, v):
        x1, y1 = u
        x2, y2 = v
        x1, y1 = self.convert_point(x1, y1)
        x2, y2 = self.convert_point(x2, y2)

        l = self.c.create_line(x1, y1, x2, y2, arrow=tkinter.LAST, tags="arrow")

    def draw(self):
        self.c.pack()

    def loop(self):
        g1 = transform_graph(get_transform(), self.g_init)
        if g1 is not None:
            self.set_graph(g1)
        else:
            print("Enter valid graph")
        self.draw()
        self.c.after(50, self.loop)


def get_transform():
    t = input("Transformation: ")
    elems = [int(c) for c in t.split(",")]
    if elems is not None and len(elems) == 4:
        return [elems[0:2], elems[2:4]]
    else:
        return None


def plot_graph():
    root = tkinter.Tk()
    c = Canvas(41)

    # k = 4
    # g = gen_dep_graph(10, [lambda i, j: (i, k), lambda i, j: (k, j)])
    g = gen_graph(1, 1, 4, 4)
    root.withdraw()

    c.set_graph(g)
    c.draw()
    c.c.after(10, c.loop)
    c.c.mainloop()


if __name__ == '__main__':
    plot_graph()
