import random

def gen_pixels(n=10):
    return [(random.randint(0,255),
             random.randint(0,255),
             random.randint(0,255)) for _ in range(n)]
