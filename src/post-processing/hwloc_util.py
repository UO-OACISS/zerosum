#!/usr/bin/env python3

import json
import pandas as pd
import glob
import os
pd.options.mode.chained_assignment = None  # default='warn'

def parseData():
    # Read the CSV data
    all_files = glob.glob(os.path.join('.', "zs.data.*.csv"))
    df = pd.concat((pd.read_csv(f) for f in all_files), ignore_index=True)
    # select the hardware threads
    hwt = df.loc[df['resource'] == 'HWT']
    # Convert the value to a number
    hwt['value'] = pd.to_numeric(hwt['value'])
    # drop the step column
    hwt = hwt.drop('step', axis=1)
    # Get rid of "idle"
    hwt = hwt[hwt.name != 'idle']
    # Group by everything except value to get the mean value
    mean_cols = ['value']
    hwt_mean = hwt.groupby([c for c in hwt.columns if c not in mean_cols]).mean()
    # Reset indices
    hwt_mean.reset_index(inplace=True)
    # drop the name column
    hwt_mean.drop('name', axis=1, inplace=True)
    # Group by everything except name to get the mean value
    sum_cols = ['value']
    hwt_sum = hwt_mean.groupby([c for c in hwt_mean.columns if c not in sum_cols]).sum()
    # Reset indices
    hwt_sum.reset_index(inplace=True)
    #print(hwt_sum.to_string())
    return hwt_sum

def traverseTree(tree, df):
    utilization = int(tree['utilization'])
    if tree['name'].startswith("PU L#"):
        name = tree['name']
        tokens = name.split()
        osindex = tokens[2]
        osindex = int(osindex[2:])
        if osindex in df['index'].values:
            utilization = df.loc[df['index'] == osindex, 'value'].iloc[0]
    else:
        if 'children' in tree.keys():
            newChildren = []
            for c in tree['children']:
                newChild = traverseTree(c, df)
                utilization += newChild['utilization']
                newChildren.append(newChild)
            tree['children'] = newChildren
            utilization = utilization / len(tree['children'])
    tree['utilization'] = utilization;
    return tree

def updateTree(df):
    fp = open('zs.topology.saturn.json', 'r')
    tree = json.load(fp)
    fp.close()
    tree = traverseTree(tree, df)
    fp = open('saturn.json', 'w')
    json.dump(tree, fp)
    fp.close()

def main():
    df = parseData()
    updateTree(df)

if __name__ == '__main__':
    main()
