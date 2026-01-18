import subprocess
from gen_pixels import gen_pixels
from check_output import check_output

pixels = gen_pixels(10)

with open("input.txt", "w") as f:
    for r,g,b in pixels:
        f.write(f"{r} {g} {b}\n")

subprocess.run(["./sim_isp"])

out = []
with open("output.txt") as f:
    for line in f:
        r,g,b = map(int, line.split())
        out.append((r,g,b))

print("PASS" if check_output(pixels, out) else "FAIL")
