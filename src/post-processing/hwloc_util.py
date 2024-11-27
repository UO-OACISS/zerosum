#!/usr/bin/env python3

import json
import pandas as pd
import glob
import os
pd.options.mode.chained_assignment = None  # default='warn'
template_location = "@CMAKE_INSTALL_PREFIX@/etc/"
template = template_location + "zs-hwloc-template.html"
d3_js = template_location + "d3.v3.js"
standalone_template = template_location + "zs-hwloc-template-standalone.html"

def parseArgs():
    import argparse
    parser = argparse.ArgumentParser(description='Post-process ZeroSum HWLOC topology utilization.')
    parser.add_argument('--standalone', dest='standalone', action='store_true',
        help='Generate standalone HTML file to be used offline (default: false)', default=False)
    args = parser.parse_args()
    return args

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
    # select only user time
    hwt = hwt[hwt.name == 'user']
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
    shmrank = int(tree['shmrank'])
    if tree['name'].startswith("PU L#"):
        name = tree['name']
        tokens = name.split()
        osindex = tokens[2]
        osindex = int(osindex[2:])
        if osindex in df['index'].values:
            utilization = df.loc[df['index'] == osindex, 'value'].iloc[0]
            shmrank = int(df.loc[df['index'] == osindex, 'shmrank'].iloc[0])
            #print(osindex, utilization)
    else:
        if 'children' in tree.keys():
            newChildren = []
            for c in tree['children']:
                newChild = traverseTree(c, df)
                utilization += newChild['utilization']
                shmrank = max(shmrank,newChild['shmrank'])
                newChildren.append(newChild)
            tree['children'] = newChildren
            utilization = utilization / len(tree['children'])
    tree['utilization'] = utilization;
    tree['shmrank'] = shmrank;
    return tree

def updateTree(df):
    all_files = glob.glob(os.path.join('.', "zs.topology.*.json"))
    job = {}
    job['name'] = 'job'
    job['detail_name'] = '(jobid)'
    job['utilization'] = 0
    job['shmrank'] = 0;
    job['children'] = []
    for f in all_files:
        fp = open(f, 'r')
        tree = json.load(fp)
        job['children'].append(tree)
        fp.close()
    tree = traverseTree(job, df)
    return tree

def main():
    args = parseArgs()
    df = parseData()
    tree = updateTree(df)
    if args.standalone:
        with open(template,'r') as file:
            populate_me = file.read()
        with open(d3_js,'r') as file:
            javascript = file.read()
        java_as_string = json.dumps(tree)
        from string import Template
        s = Template(populate_me)
        with open('zs-topology.html', 'w') as fp:
            fp.write(s.substitute(JSONDATA=java_as_string, SCRIPTTEXT=javascript))

    else:
        with open('zs.topology.json', 'w') as fp:
            json.dump(tree, fp)
        import shutil
        shutil.copyfile(template,os.getcwd()+"/zs-topology.html")

if __name__ == '__main__':
    main()

