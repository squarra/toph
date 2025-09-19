from pytoph import Frame
import numpy as np

f1 = Frame("one")
f2 = Frame("two")
f1.add_child(f2)

print(f2.parent)
print(f1.children)

# get matrix
print(f1.matrix)  # numpy 4x4

# set full matrix
M = np.eye(4)
M[:3, 3] = [1, 2, 3]
f1.matrix = M

# translation get/set
print(f1.translation)  # numpy vec3
f1.translation = [5, 5, 5]
print(f1.translation)

# rotation get/set
print(f1.rotation)  # numpy 3x3
f1.rotation = np.eye(3)  # reset

# quaternion get/set
print(f1.quaternion)  # numpy vec4 (w, x, y, z)
f1.quaternion = [0, 0, 1, 0]
print(f1.quaternion)

f1.translate([1, 2, 3])
print(f1.matrix)

print(f2.matrix)
f2.translate([-1, -1, -1])
print(f2.matrix)

print(f2.world_matrix)
print(f2.translation)
