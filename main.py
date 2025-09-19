from pytoph import Frame

f1 = Frame("one")
f2 = Frame("two")
f1.add_child(f2)
f2.translate([1.0, 0.0, 0.0])
f1.show()
