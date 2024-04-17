import serial, struct
'''
rom = bytearray([0xea] * 32768)

with open("rom.bin", "wb") as out_file:
    out_file.write(rom)
    print("File written")
'''
DATA_MAX_BYTE_SIZE = 512

try:
  ser = serial.Serial("COM6", 115200, timeout=.1)
except:
  print("Cannot open COM6")

def sendBytes(sendStr):
  ser.write(sendStr)

def twoBytesToInt(a):
  return a[0] * 0x100 + a[1]

def intToTwoBytes(a):
  return a.to_bytes(2, 'big')

def controlRangeSize(start, end):
  startInt = twoBytesToInt(start)
  endInt = twoBytesToInt(end)

  n =  endInt - startInt + 1
  if (n > DATA_MAX_BYTE_SIZE):
    newEnd = startInt + 512
    print("Address range was bigger than",DATA_MAX_BYTE_SIZE)
    print("New range is from {:02X} to {:02X}".format(startInt, newEnd))
    return (start,intToTwoBytes(newEnd))
  else:
    return (start, end)

def controlBytesSize(data):
  if (len(data) > DATA_MAX_BYTE_SIZE):
    print("Data size was bigger than",DATA_MAX_BYTE_SIZE)
    return data[0:DATA_MAX_BYTE_SIZE]
  return data

def printByte(start, end, data, i):
  if (i % 16 == 0):
    if (start % 16 == 0):
      print("{:04X}:".format(start + i), end='')
    else:
      startTmp = start - start % 16
      print("{:04X}:".format(startTmp + i), end='')
      if (i == 0):
        while (i < start % 16):
          if (i % 8 == 0):
            print(" ", end='')
          print(" ..",end='')
          i += 1
  print(("  " if i % 8 == 0 else " " )+ data.hex() + "", end='')
  
  if (i % 16 == 15):
    print()
  elif (end - start + (start % 16) - 1 == i):
    while (i % 16 != 15):
      print(("  " if i % 16 == 7 else " " )+ "..", end='')
      i += 1
  i +=1
  return i

def writeEEPROM(start, data):
  opCode = b'\x02'
  arr = bytearray(opCode)
  arr += start
  arr += controlBytesSize(data)

  sendBytes(arr)
  ser.flushInput()

  data = None
  while(data != b'\x0b'):
    data = ser.read()

def readEEPROM(start, end):
  #start, end = controlRangeSize(start, end)

  startInt = int.from_bytes(start, "big")
  endInt = int.from_bytes(end, "big")

  opCode = b'\x01'
  arr = bytearray(opCode)
  arr += start
  arr += end

  sendBytes(arr)
  ser.flushInput()

  print("\n"+' '*12+"Reading data from {:04X} to {:04X}.\n".format(startInt, endInt))

  read = False
  i = 0
  while(True):
    data = ser.read()
    if (data != b''):
      read = True
      i = printByte(startInt, endInt, data, i)
    elif(read):
      break

#writeEEPROM(b'\x00\x70', b'\x24\x08\xaa\xcd')
readEEPROM(b'\x00\x50', b'\x03\x48')