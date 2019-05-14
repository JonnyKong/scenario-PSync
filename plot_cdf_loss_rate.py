# -*- coding: utf-8 -*-
"""
    Plot CDFs under loss rates of 0, 0.05, 0.2, 0.5.
    @Author jonnykong@cs.ucla.edu
    @Date   2019-04-09
"""

from __future__ import print_function
import os
import sys
import numpy as np
import matplotlib.pyplot as plt

class CdfPlotter(object):
    """
    This class takes as input a list of filenames, calculate their average, and
    plot the averaged CDF, or save it to disk.
    """
    def __init__(self):
        """
        Initializes default x range to [0, 2400], and assume there are 20 nodes
        in total.
        """
        self._filenames = []
        self._dirs = [
            "results"
        ]
        self._x_mesh = np.linspace(0, 2400, num=2400+1)
        self._node_num = 20
        self._ys_mesh = []   # 2D array
        
        self._ys_tmp = []
        for _, dir in enumerate(self._dirs):
            for i in range(12):
                self.add_file(os.path.join(dir, "result_%d.txt" % (i+1)))
            y_mean = [float(sum(l)) / len(l) for l in zip(*self._ys_mesh)]
            self._ys_tmp.append(y_mean)
            self._ys_mesh = []
        self._ys_mesh = self._ys_tmp
            

    def add_file(self, filename):
        """
        Add a new file and parse the output.
        """
        self._filenames.append(filename)
        self._parse_file(filename)

    def plot_cdf(self, save=False):
        """
        Draw CDF graph for every element in self._ys_mesh, or save the graph
        to "tmp.png" if specified.
        Args:
            save (bool): Save the output graph to disk, rather than displaying it
                on the screen (e.g. if you are working on a remote server).
        """
        fig = plt.figure()
        ax = fig.add_subplot(111)
        for i, ele in enumerate(self._ys_mesh):
            ax.plot(self._x_mesh, ele, label = self._dirs[i])
        plt.ylim((0, 1))
        plt.legend()
        if save:
            fig.savefig('tmp.png')
        else:
            plt.show()

    def _parse_file(self, filename):
        """
        Read one file and interpolate its values. Then, normalize the values
        to [0, 1] and add to self._ys_mesh.
        """
        x_coord = []
        data_store = set()
        with open(filename, "r") as f:
            for line in f.readlines():
                # if line.find("Store New Data") == -1:
                #     continue
                if line.find("Update New Seq") == -1:
                    continue
                elements = line.strip().split(' ')
                time = elements[0]
                data = elements[-1]
                if data not in data_store:
                    data_store.add(data)
                x_coord.append(int(time) / 1000000)
        y_mesh = self._interp0d(x_coord)
        y_mesh = [float(l / (len(data_store) * self._node_num)) for l in y_mesh]
        self._ys_mesh.append(y_mesh)
        print("Avail: %f" % y_mesh[-1])

    def _interp0d(self, x_coord):
        """
        0-d interpolation against self._x_mesh
        """
        y_interp0d = [0 for i in range(len(self._x_mesh))]
        for i, _ in enumerate(x_coord):
            y_interp0d[int(x_coord[i])] += 1
        for i in range(1, len(y_interp0d)):
            y_interp0d[i] += y_interp0d[i - 1]

        return y_interp0d

if __name__ == "__main__":
    plotter = CdfPlotter()
    plotter.plot_cdf(save=True)
