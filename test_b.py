#!/usr/bin/python3

"""
This code is not good practice and likely won't make sense in future use.

Essentially there are two main functions that generate output for:
    - Average swaps with increasing max memory size.
    - Average swaps with increasing process size as proportion of memory.

They are both commented out and one of these functions will need to be 
uncommented out for anything to happen. The option to write the output of each
simulation execution exists, which has also been commented out inside 
singleTest(). Ignore decomposition().

For the program to work, the line that prints the number of swaps in 
simulation.c needs to uncommented/added:

printf("num_swaps %d\n", num_swaps);

"""

from subprocess import getoutput
from random import randint
from math import ceil

getoutput("make")

outFolder = "partB_cases/"
getoutput("mkdir " + outFolder)

def writeInput(contents, testNumber):
    with open(outFolder + str(testNumber) + ".txt", "w") as f:
        f.writelines(contents)
    return outFolder + str(testNumber) + ".txt"

"""
def decomposition(i):
        while i > 0:
            n = randint(1, i)
            yield n
            i -= n
"""

# Makes 100 processes randomly sized between 1 and memSize
def singleTest(alg, memSize, testNumber, modifier):
    contents = []

    """
    processSizes = list(decomposition(memsize))

    i = 0
    while i < 5:
        processSizes += list(decomposition(memsize))
        i += 1
    """
    processSizes = []
    
    processNumber = 0
    while processNumber < 100:
        # modifier can be used to make processes a smaller size of memory.
        memUsage = ceil(randint(1, memSize) * modifier)
        # time_created, process_id, memory_size, job_time;
        contents.append("0 %d %d %d\n" % (processNumber, memUsage, randint(1,20)))
        processSizes.append(randint(1, memSize))
        processNumber += 1
    
    fname = writeInput(contents, testNumber)
    #print(fname)
    command = "./simulation -f %s -a %s -m %s" % (fname, alg, memSize)

    output = getoutput(command)
    numSwaps = output.split('\n')[-1].split()[-1]
    #writeOutput(output, testNumber)

    return int(numSwaps)

def writeOutput(contents, testNumber):
    with open(outFolder + str(testNumber) + "_out.txt", "w") as f:
        f.writelines(contents)

def averageList(inp):
    total = 0
    for i in inp:
        total += i
    return total / len(inp)

# Seeing how swaps change with max memory size.
def runTestsIncreasingMemory(alg, memSize):
    testNumber = 0

    swaps = []
    while testNumber < 1000:
        swaps.append(singleTest(alg, memSize, testNumber, 1))
        testNumber += 1

    #print(swaps)
    print("MaxMemory: %5d. Average swaps: %s" % (memSize, str(averageList(swaps))))

# Seeing how swaps change with 
def runTestsVariableProcessSize(alg, memSize, modifier):
    testNumber = 0

    swaps = []
    while testNumber < 1000:
        swaps.append(singleTest(alg, memSize, testNumber, modifier))
        testNumber += 1

    #print(swaps)
    av = averageList(swaps)
    print("Average swaps: %s. Modifier: %4s. Ratio %4s" % (str(av), str(modifier), str(av/modifier)))


# This is the code for increasing max memory.
"""
memSize = 8
while memSize < 20000:
    runTestsIncreasingMemory("multi", memSize)
    memSize *= 2
"""

# This is the code for increasing process size.
"""
modifier = 1
while modifier <= 1:
    runTestsVariableProcessSize("multi", 2048, modifier)
    modifier += 0.05
"""

getoutput("rm -R " + outFolder)
getoutput("make clean")