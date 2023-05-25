# -*- coding: utf-8 -*-
import argparse
import struct
import os

#define
MEM_S  = 128 
MAX_MEM_SIZE = MEM_S*1024

# main
if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='hashApp')
    parser.add_argument('input', help='Input file path')
    parser.add_argument('output', help='Output file path')

    args = parser.parse_args()
    filePath = args.input
    filePath2 = args.output

    cnt = 0
    a_data = 0
    b_data = 0

    with open(filePath, 'rb') as f:
        while True:
            chunk = f.read(2)
            a_data += int.from_bytes(chunk, byteorder='little')
            if len(chunk) == 0:
                chunk = f.read(1)
                a_data += int.from_bytes(chunk, byteorder='little') & 0x00FF
                break

    a_data = a_data & 0x000000FFFF
    b_data = a_data.to_bytes(4, 'little')
    
    print("sum: {0}".format(filePath, hex(a_data)))

    
    f=open(filePath2, mode ='ab')
    a_size = os.path.getsize(filePath)
    b_size = a_size.to_bytes(4, 'little')
    print("size: {0}".format(a_size))
    f.write(b_size)
    f.write(b_data)
    
    f.close
