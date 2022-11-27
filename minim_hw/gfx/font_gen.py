from PIL import Image
import numpy
import sys

#python 'name' height width

out_name = sys.argv[1]
char_height = int(sys.argv[2])
char_width = int(sys.argv[3])
img_name = out_name + ".png"

img = Image.open(img_name)
data = numpy.asarray(img.getdata())

byte = 0
w = img.size[0]
h = img.size[1]

with open(out_name + ".hpp", 'w') as hpp:
    hpp.write("#pragma once\n\n")
    hpp.write("#include <libmaple/libmaple_types.h>\n")
    hpp.write("extern uint8_t " + out_name + "_tall;\n") 
    hpp.write("extern uint8_t " + out_name + "_wide;\n") 
    hpp.write("extern uint16_t " + out_name + "_count;\n") 
    hpp.write("extern uint" + str(char_height) + "_t " + out_name + "_data [" + str(w) + "];\n");

with open(out_name + ".cpp", 'w') as f:
    f.write("#include \"" + out_name + ".hpp\" \n\n")
    f.write("uint8_t " + out_name + "_tall = " + str(char_height) + ";\n")
    f.write("uint8_t " + out_name + "_wide = " + str(char_width) + ";\n")
    f.write("uint16_t " + out_name + "_count = " + str((w/char_width)) + ";\n")

    f.write("uint" + str(char_height) + "_t " + out_name + "_data [" + str(w) + "] = {\n")
    for x in range(w):
        for y in range(h):
            if(data[x + (y*w)][0] > 0):
                byte |= (1<<y)
        
        end =  ",\n" if (x<w-1) else " \n"
        f.write("\t" + hex(byte) + end)
        byte = 0
    f.write("};\n")
