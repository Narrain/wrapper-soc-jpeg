def check_output(inp, out):
    assert len(inp) == len(out)
    for i, (a, b) in enumerate(zip(inp, out)):
        if a != b:
            print("Mismatch at", i, a, b)
            return False
    return True
