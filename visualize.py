#!/usr/bin/env python3

import sys
import matplotlib.pyplot as plt

data = list(map(int, sys.stdin))

plt.plot(range(0, len(data)), data)
plt.show()
