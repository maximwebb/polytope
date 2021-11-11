import tkinter
from tkinter import ttk

class Graph:
    def __init__(self, n=None):
        self.V = []
        self.E = []
        if n is not None:
            for i in range(n):
                for j in range(n):
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


class Canvas:
    def __init__(self, n):
        self.root = tkinter.Tk()
        self.width = 50 * n + 100
        self.height = 50 * n + 100
        # create canvas
        self.c = tkinter.Canvas(self.root, bg="white", height=self.height, width=self.width)

    def convert_point(self, x, y):
        x = 50 + x * 50
        y = self.width - 50 - (y * 50)
        return (x, y)

    def add_point(self, v):
        w = 4
        x, y = v
        x, y = self.convert_point(x, y)

        l = self.c.create_oval(x - w, y - w, x + w, y + w, fill="red")

    def add_arrow(self, u, v):
        x1, y1 = u
        x2, y2 = v
        x1, y1 = self.convert_point(x1, y1)
        x2, y2 = self.convert_point(x2, y2)

        l = self.c.create_line(x1, y1, x2, y2, arrow=tkinter.LAST)


    def draw(self):
        self.c.pack()
        self.root.mainloop()


def plot_graph():
    root = tkinter.Tk()

    # create canvas
    c = Canvas(10)

    k = 4

    g = gen_dep_graph(10, [lambda i, j: (i, k), lambda i, j: (k, j)])

    for v in g.V:
        c.add_point(v)

    for (u,v) in g.E:
        c.add_arrow(u,v)

    c.draw()

if __name__ == '__main__':
    plot_graph()
