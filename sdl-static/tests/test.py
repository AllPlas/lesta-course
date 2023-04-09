import sys


def test_sdl_static():
    line = sys.stdin.readline().strip()
    version = line.split(":")[1]
    major_version = int(version.split(".")[0])
    assert major_version >= 3


if __name__ == '__main__':
    test_sdl_static()
