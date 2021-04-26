import sys

def main(arg):
    f = open(arg[0], 'rb')
    f2 = open("result.txt", 'w+')
    b = f.read()
    
    tim = int.from_bytes(b[0:8],byteorder='little',signed=False)
    print("Time:", tim)
    size = int.from_bytes(b[8:16],byteorder='little',signed=False)
    f2.write("Time: " + str(tim) + "\n")
    print("Size:", size)
    f2.write("Size: " + str(size) + "\n")
    mini = int.from_bytes(b[16:24],byteorder='little',signed=False)
    print("Min:", mini)
    f2.write("Min: " + str(mini) + "\n")
    maxi = int.from_bytes(b[24:32],byteorder='little',signed=False)
    print("Max:", maxi)
    f2.write("Max: " + str(maxi) + "\n")

    print("BF:", int.from_bytes(b[32:10272],byteorder='little',signed=False))
    print("Index:")
    b = b[10272:]
    content = b[size*12:].decode()
    i = 0
    while i < size * 12:
        key = int.from_bytes(b[i:i+8],byteorder='little',signed=False)
        #print("key:", key, end=', ')
        offset = int.from_bytes(b[i+8:i+12],byteorder='little',signed=False)
        if i == size*12 - 12:
            f2.write("key: "+str(key)+", offset: "+ str(offset) + ", content: " +content[offset-10272-size*12:]+"\n")
        else:
            nextoffset = int.from_bytes(b[i+20:i+24],byteorder='little',signed=False)
        #print("offset:", offset)
            f2.write("key: "+str(key)+", offset: "+ str(offset) + ", content: " +content[offset-10272-size*12:nextoffset-10272-size*12]+"\n")
        #print("value:", content[offset:nextoffset])
        i += 12
    f.close()
    f2.close()

if __name__=="__main__":
    if len(sys.argv) > 1:
        main(sys.argv[1:])
    else:
        main([input("Input filename: ")])
