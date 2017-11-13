from PIL import Image, ImageOps
import sys


def die():
  print("Usage: image2c.py <input.image> <output.c>")
  sys.exit(1)


if len(sys.argv) != 4:
  die()

input_image = Image.open(sys.argv[1])
bw_image = ImageOps.invert(input_image.convert(mode="RGB")).convert(mode="1")
#bw_image = input_image.convert(mode="1")
image_data = bw_image.getdata()

width, height = bw_image.size
size = width * height // 8

with open(sys.argv[2], "w") as output_file:
  output_file.write("#include <stdint.h>\n")
  output_file.write("#include <avr/pgmspace.h>\n\n")
  output_file.write("const uint8_t %s[%d] PROGMEM = {\n" % (sys.argv[3], size + 2))
  output_file.write("  0x%02x, /* = width */\n  0x%02x, /* = height */\n" % (width, height))

  i = 0

  for seg in range(height // 8):
    for x in range(width):
      byte = 0
      for bit in range(8):
        y = seg * 8 + 7 - bit
        byte *= 2
        if bw_image.getpixel((x, y)):
          byte += 1

      if i % 16 == 0:
        output_file.write("  ")

      output_file.write("0x%02x" %  byte)

      i += 1

      if i < size:
        output_file.write(",")

      if i % 16 == 0:
        output_file.write("\n")
      else:
        output_file.write(" ")

  output_file.write("};\n")
