from pytoph import Frame
import numpy as np

p = Frame("parent")
p.set_translation([1, 2, 3])
print(p.translation)  # numpy array [1,2,3]
print(p.world_pose)  # 4x4 numpy matrix

c = Frame("child")
p.add_child(c)
c.translate([0, 1, 0])  # relative move in local coordinates
print(c.world_pose)  # result includes parent's offset
