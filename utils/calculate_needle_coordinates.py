import sys
import math


def die():
  print("Usage: calculate_needle_coordinates.py <resolution (int)>")
  sys.exit(1)

def deg_to_rad(deg):
  return math.pi * deg / 180


if len(sys.argv) != 2:
  die()

try:
  RESOLUTION = int(sys.argv[1])
except ValueError:
  die()


print("#include \"needle_coordinates.h\"")
print("#include <stdint.h>")
print("#include <avr/pgmspace.h>\n")
print("const needle_coordinates_t NEEDLE_COORDINATES[NEEDLE_RESOLUTION] PROGMEM = {")

DISPLAY_WIDTH = 128
DISPLAY_HEIGHT = 64

MIN_ANGLE = deg_to_rad(-41.4)
MAX_ANGLE = deg_to_rad(41.4)
LENGTH = 96
AXIS_X = DISPLAY_WIDTH / 2
AXIS_Y = LENGTH

for angle_idx in range(RESOLUTION):
  angle = MIN_ANGLE + (MAX_ANGLE - MIN_ANGLE) * angle_idx / (RESOLUTION - 1)
  ax = int(AXIS_X + math.sin(angle) * LENGTH)
  ay = int(AXIS_Y - math.cos(angle) * LENGTH)
  print("  { %3d, %3d }," % (ax, ay))

print("};")
