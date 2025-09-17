from pytoph import Frame
import numpy as np

f = Frame("demo")

# get matrix
print(f.matrix)  # numpy 4x4

# set full matrix
M = np.eye(4)
M[:3, 3] = [1, 2, 3]
f.matrix = M

# translation get/set
print(f.translation)  # numpy vec3
f.translation = [5, 5, 5]
print(f.translation)

# rotation get/set
print(f.rotation)  # numpy 3x3
f.rotation = np.eye(3)  # reset

# quaternion get/set
print(f.quaternion)  # numpy vec4 (w, x, y, z)
f.quaternion = [0, 0, 1, 0]  # identity
print(f.quaternion)

f.translate([1, 2, 3])
print(f.matrix)
