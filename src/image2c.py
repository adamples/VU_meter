import sys
import math
from struct import unpack


DITHERING_ALGORITHMS = {
  'floyd-steinberg': [
    ( 1, 0, 7.0 / 16.0),
    (-1, 1, 3.0 / 16.0),
    ( 0, 1, 5.0 / 16.0),
    ( 1, 1, 1.0 / 16.0),
  ],
  'jarvis-judice-ninke': [
    ( 1, 0, 7.0 / 42.0),
    ( 2, 0, 5.0 / 42.0),
    (-2, 1, 3.0 / 42.0),
    (-1, 1, 5.0 / 42.0),
    ( 0, 1, 7.0 / 42.0),
    ( 1, 1, 5.0 / 42.0),
    ( 2, 1, 3.0 / 42.0),
    (-2, 2, 1.0 / 42.0),
    (-1, 2, 3.0 / 42.0),
    ( 0, 2, 5.0 / 42.0),
    ( 1, 2, 3.0 / 42.0),
    ( 2, 2, 1.0 / 42.0)
  ],
  'stucki': [
    ( 1, 0, 8.0 / 42.0),
    ( 2, 0, 4.0 / 42.0),
    (-2, 1, 2.0 / 42.0),
    (-1, 1, 4.0 / 42.0),
    ( 0, 1, 8.0 / 42.0),
    ( 1, 1, 4.0 / 42.0),
    ( 2, 1, 2.0 / 42.0),
    (-2, 2, 1.0 / 42.0),
    (-1, 2, 2.0 / 42.0),
    ( 0, 2, 4.0 / 42.0),
    ( 1, 2, 1.0 / 42.0),
    ( 2, 2, 1.0 / 42.0)
  ]
}


def srgb_to_linear(value):
  if value < 0.04045:
    return value / 12.92

  return ((value + 0.055) / 1.055) ** 2.4


def linear_to_srgb(value):
  if value <= 0.0031308:
    return 12.92 * value

  return 1.055 * (value ** (1/2.4)) - 0.055


def srgb_8bit_to_grayscale(r, g, b):
  rl = srgb_to_linear(r / 255.0)
  gl = srgb_to_linear(g / 255.0)
  bl = srgb_to_linear(b / 255.0)
  grl = 0.2126 * rl + 0.7152 * gl + 0.0722 * bl
  return linear_to_srgb(grl)


class Bmp(object):

  def __init__(self, path):
    self.path = path
    self.file = open(path, 'rb')
    self.read_header()


  def __iter__(self):
    self.x = 0
    self.y = self.height - 1
    self.file.seek(self.data_offset)
    return self


  def __next__(self):
    if self.y < 0:
      raise StopIteration()

    if self.x == 0:
      self.row = bytearray(self.file.read(self.row_length))

    if self.bpp == 24:
      b, g, r = self.row[self.x * 3:(self.x + 1) * 3]
      result = (self.x, self.y, (r, g, b))

    self.x += 1

    if self.x == self.width:
      self.x = 0
      self.y -= 1

    return result


  def next(self):
    return self.__next__()


  def read_header(self):
    start = self.file.read(14)
    magick_bytes, bitmap_size, _, __, self.data_offset = unpack('<HIHHI', start)

    if magick_bytes != 0x4d42:
      raise RuntimeError('Not a bitmap file or nsupported bitmap format')

    rest = self.file.read(self.data_offset - 14)
    _, self.width, self.height, __, self.bpp, compression = unpack('<IIIHHI', rest[:20])

    if compression != 0:
      raise RuntimeError('Bmp compression is not supported')

    if self.bpp != 24:
      raise RuntimeError('Only 24-bit bitmaps are supported')

    self.row_length = int(math.floor((self.bpp * self.width + 31) / 32) * 4)


class CBitmap(object):

  def __init__(self, name, width, height):
    self.name = name
    self.width = width
    self.height = height
    self.data = [0 for i in range(self.width * int(math.ceil(float(self.height) / 8)))]

  def set_pixel(self, x, y, value):
    byte_offset = int(y / 8) * self.width + x
    bit_offset = y % 8

    if value > 0.5:
      self.data[byte_offset] |= 1 << bit_offset


  def write(self, path):
    with open(path, 'w') as f:
      f.write("#include <stdint.h>\n")
      f.write("#include <avr/pgmspace.h>\n\n")
      f.write("const uint8_t %s[%d] PROGMEM = {\n" % (self.name, len(self.data) + 2))
      f.write("  0x%02x, /* = width */\n  0x%02x, /* = height */\n" % (self.width, self.height))

      dataiter = iter(self.data)

      while True:
        try:
          f.write('  ')

          for i in range(15):
            f.write('0x%02x, ' % next(dataiter))

          f.write('0x%02x,\n' % next(dataiter))

        except StopIteration:
          break

      f.write("};\n")


def dither(source, target, algo='floyd-steinberg', inverted=False):
  algo = DITHERING_ALGORITHMS[algo]

  def empty_error_row():
    return [0.0 for i in range(source.width)]

  errors = [empty_error_row() for i in range(3)]

  for x, y, pixel in source:
    grayscale = srgb_8bit_to_grayscale(*pixel)
    with_error = grayscale + errors[0][x]

    if with_error < 0.5:
      discriminated = 0.0
    else:
      discriminated = 1.0

    if inverted:
      target_pixel = 1.0 - discriminated
    else:
      target_pixel = discriminated

    target.set_pixel(x, y, target_pixel)

    error = with_error - discriminated

    for dx, dy, k in algo:
      if x + dx >= 0 and x + dx < source.width:
        errors[dy][x + dx] += error * k

    if x == source.width - 1:
      errors = errors[1:] + [empty_error_row()]


input_path = sys.argv[1]
output_path = sys.argv[2]
name = sys.argv[3]

if len(sys.argv) == 5:
  inverted = sys.argv[4] == 'inverted'
else:
  inverted = False

if len(sys.argv) >= 6:
  dithering = sys.argv[5]
else:
  dithering = 'jarvis-judice-ninke'


input_bitmap = Bmp(input_path)
output_bitmap = CBitmap(name, input_bitmap.width, input_bitmap.height)
dither(input_bitmap, output_bitmap, dithering, inverted=True)
output_bitmap.write(output_path)
