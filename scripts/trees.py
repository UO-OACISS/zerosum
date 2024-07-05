#!/usr/bin/env python3

import os.path
import sys
import glob
import re

def printstack(stack):
    for s in stack:
        print('\t',s[:80])
    print('')

class Node:
    total = 0
    def __init__(self, value, count, ranks):
        self.value = value
        self.children = []
        self.count = count
        self.ranks = ranks
        self.index = Node.total
        Node.total = Node.total + 1

def dotstacks(stacks):
    root = Node('root', 0, [])
    current_node = root
    i = 0

    for s in stacks:
        current_node = root
        ranks = stacks[s]
        count = len(ranks)
        current_node.count = current_node.count + count
        current_node.ranks = current_node.ranks + ranks
        #printstack(s)
        for f in s:
            found = False
            for c in current_node.children:
                if f == c.value:
                    current_node = c
                    current_node.count = current_node.count + count
                    current_node.ranks = current_node.ranks + ranks
                    found = True
            if not found:
                new_node = Node(f, count, ranks)
                current_node.children.append(new_node)
                current_node = new_node
    return root

def printnode(node,index):
    i = 0
    print(node.count, end='\t')
    while i < index:
        print(' ', end='')
        i = i + 1
    print(node.value)
    for c in node.children:
        printnode(c,index+1)

#digraph {
#    1 [label="root"];
#    2 [label="main"];
#    1 -> 2 [label="20"];
#}
import itertools

def ranges(i):
    for a, b in itertools.groupby(enumerate(i), lambda pair: pair[1] - pair[0]):
        b = list(b)
        yield b[0][1], b[-1][1]

# We have a list of start/end tuples, so convert to strings
def printlist(inlist):
    delim = ""
    if len(inlist) > 1:
        delim = "["
    tmpstr = ""
    for t in inlist:
        tmpstr += delim
        if t[0] == t[1]:
            tmpstr += str(t[0])
        else:
            #tmpstr += "[" + str(t[0]) + "-" + str(t[1]) + "]"
            tmpstr += str(t[0]) + "-" + str(t[1])
        delim = ','
    if len(inlist) > 1:
        tmpstr += "]"
    if len(tmpstr) > 40:
        return tmpstr[:40] + '...'
    return tmpstr

def writenode(node,f,total):
    f.write(str(node.index))
    f.write(" ")
    f.write(" [shape=box; style=filled; label=\"")
    f.write(node.value)
    f.write("\"; fillcolor=\"0.000 1.000 1.000 ")
    f.write(str(node.count / total))
    f.write("\"];\n")

def writeedge(parent,child,f):
    f.write(str(parent.index))
    f.write(" -> ")
    f.write(str(child.index))
    f.write(" [label=\"")
    f.write(str(child.count))
    f.write(":")
    child.ranks.sort()
    tmpstr = printlist(list(ranges(child.ranks)))
    f.write(tmpstr)
    #f.write(''.join(str(x) for x in list(ranges(child.ranks))))
    f.write("\"];\n")

def writechild(node,f,parent,total):
    writenode(node,f,total)
    writeedge(parent,node,f)
    for c in node.children:
        writechild(c,f,node,total)

def writegraph(node,f,total):
    writenode(node,f,total)
    for c in node.children:
        writechild(c,f,node,total)

def parseFile(f):
    # reading the file
    data = f.read()

    data_into_list = data.split("\n")
    # match Thread 1 (Thread 0x7fff703f3000 (LWP 1096789) "xgc-eem-cpp-gpu"):
    stack = []
    active = False
    for line in data_into_list:
        if active:
            # Look for #0  0x00007fff7b882b32 in the string
            if line[0] == '#':
                line = line[3:].strip()
                if line.startswith('0x'):
                    line = line[21:].strip()
                # get the source location, if possible
                location = ''
                at = line.rfind(' at ')
                if (at > 0):
                    location = '\n' + line[at+4:]
                else:
                    from_ = line.rfind(' from ')
                    if (from_ > 0):
                        location = '\n' + line[from_+6:]
                end = line.rfind(' (')
                line = line[:end]
                if line.find('<') > 0 :
                    end = line.find('<')
                    line = line[:end]
                line = line.strip() + location
                stack.append(line)
            else:
                active = False
        if "Thread 1 (Thread 0x" in line:
            #print(line)
            active = True
    reverseStack = []
    while len(stack) > 0:
        line = stack.pop()
        if len(stack) > 1 and line.find(' at ') > 0 :
            end = line.find(' at ')
            line = line[:end]
        reverseStack.append(line)
    #printstack(reverseStack)
    return tuple(reverseStack)

def iterateFiles():
    stacks = []
    files = sorted(glob.glob('zsgdb.*.log'))
    numfiles = len(files)
    index = 0
    for f in files:
        with open(f, "r") as f_in:
            stacks.append(parseFile(f_in))
    return stacks

def findUnique(stacks):
    uniques = {}
    index = 0
    for s in stacks:
        if s not in uniques:
            uniques[s] = []
        uniques[s].append(index)
        index = index + 1
    print(len(uniques), "unique stacks found.")
    return uniques

def main():
    stacks = iterateFiles()
    uniques = findUnique(stacks)
    root = dotstacks(uniques)
    #printnode(root,0)
    f=open("graph.dot","w")
    f.write("digraph {\n")
    writegraph(root,f,root.count)
    f.write("}\n")
    f.close()
    print('done.')

if __name__ == '__main__':
    main()

