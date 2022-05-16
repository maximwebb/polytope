import copy
import tkinter
from tkinter import ttk

width = 50


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


def gen_dep_graph(a1, a2, b1, b2, deps):
    g = Graph((b1 - a1), (b2 - a2), a1, a2)
    for f in deps:
        for i in range(a1, b1):
            for j in range(a2, b2):
                v_i, v_j = f(i, j)
                if a1 <= v_i <= b1 and a2 <= v_j <= b2:
                    g.add_edge((v_i, v_j), (i, j))
    return g


def gen_graph(a1, a2, b1, b2):
    g = Graph((b1 - a1), (b2 - a2), a1, a2)
    return g


def transform_graph(T, g):
    g1 = Graph()
    g1.V = [(T[0][0] * x + T[0][1] * y, T[1][0] * x + T[1][1] * y) for (x, y) in g.V]
    g1.E = [((T[0][0] * x1 + T[0][1] * y1, T[1][0] * x1 + T[1][1] * y1),
             (T[0][0] * x2 + T[0][1] * y2, T[1][0] * x2 + T[1][1] * y2))for ((x1, y1), (x2, y2)) in g.E]
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
        for i in range(self.n):
            x_label = self.convert_point(i, -0.2)
            y_label = self.convert_point(-0.2, i)
            self.c.create_text(*x_label, text=f"{i}")
            self.c.create_text(*y_label, text=f"{i}")

            self.c.create_line(*self.convert_point(i, 0), *self.convert_point(i, self.n), fill='#e8e8e8')
            self.c.create_line(*self.convert_point(0, i), *self.convert_point(self.n, i), fill='#e8e8e8')
        l1 = self.c.create_line(*self.convert_point(0, 0), *self.convert_point(0, self.n), arrow=tkinter.LAST)
        l2 = self.c.create_line(*self.convert_point(0, 0), *self.convert_point(self.n, 0), arrow=tkinter.LAST)
        self.c.create_text(*self.convert_point(self.n, -0.2), text="p")
        self.c.create_text(*self.convert_point(-0.2, self.n), text="q")

    def set_graph(self, g):
        self.c.delete("point")
        self.c.delete("arrow")
        self.g = g
        if self.g_init is None:
            self.g_init = g
        for (u, v) in g.E:
            self.add_arrow(u, v)
        for v in g.V:
            self.add_point(v)

    def convert_point(self, x, y):
        x = width + x * width
        y = self.width - width - (y * width)
        return x, y

    def add_point(self, v):
        w = 4
        shift = 0
        x, y = v
        x, y = self.convert_point(x, y)
        l = self.c.create_oval(x - w + shift, y - w - shift, x + w + shift, y + w - shift, fill="#2196F3", tags="point")

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
    c = Canvas(6)

    # k = 4
    # g = gen_dep_graph(1, 1, 4, 3, [lambda i, j: (i, j-1), lambda i, j: (i-1, j)])
    # g = gen_dep_graph(0, 0, 7, 5, [lambda i, j: (i-1, j), lambda i, j: (i, j-1), lambda i, j: (i+1, j-1)])
    g = gen_graph(0, 0, 5, 7)
    root.withdraw()

    c.set_graph(g)
    c.draw()
    c.c.after(10, c.loop)
    c.c.mainloop()


if __name__ == '__main__':
    plot_graph()
