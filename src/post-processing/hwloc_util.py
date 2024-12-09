#!/usr/bin/env python3

import json
import pandas as pd
import glob
import os
import re
pd.options.mode.chained_assignment = None  # default='warn'
template_location = "@CMAKE_INSTALL_PREFIX@/etc/"
d3_js = template_location + "d3.v3.js"
template = template_location + "zs-hwloc-template.html"
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
    return df

def parseHWTData(df):
    # select the hardware threads
    hwt = df.loc[df['resource'] == 'HWT']
    # Convert the value to a number
    hwt['value'] = pd.to_numeric(hwt['value'])
    # drop the step column
    #hwt = hwt.drop('step', axis=1)
    # select only user time
    hwt_user = hwt[hwt.name == 'user']
    # Group by everything except value to get the mean value
    mean_cols = ['value','step']
    hwt_mean = hwt_user.groupby([c for c in hwt_user.columns if c not in mean_cols]).mean()
    # Reset indices
    hwt_mean.reset_index(inplace=True)
    # drop the name column
    hwt_mean.drop('name', axis=1, inplace=True)
    # Group by everything except name to get the mean value
    sum_cols = ['value','step']
    hwt_sum = hwt_mean.groupby([c for c in hwt_mean.columns if c not in sum_cols]).sum()
    # Reset indices
    hwt_sum.reset_index(inplace=True)
    #print(hwt_sum.to_string())
    return hwt_sum,hwt

def parseGPUData(df):
    # Find this:
    # "x1921c0s0b0n0",1,0,0,"GPU","Property","0","PCI Address","0000:18:00.0"
    # select the GPUs
    gpu = df.loc[(df['resource'] == 'GPU') & (df['name'] == 'PCI Address')]
    if gpu.empty:
        gpu = df.loc[(df['resource'] == 'GPU') & (df['name'] == 'Bus ID')]
    #print(gpu.to_string())
    addresses = {}
    for index, row in gpu.iterrows():
        #print(row['rank'], row['value'])
        # our key is a list of two values, the MPI rank and the GPU index
        addresses[(row['rank'],row['index'])] = row['value'].lower()
    # Get the utilization data for each gpu
    gpu = df.loc[(df['resource'] == 'GPU') & (df['name'].str.contains('L0 All Engines, subdevice'))]
    tiles = True
    scale = 100.0
    if gpu.empty:
        gpu = df.loc[(df['resource'] == 'GPU') & (df['name'] == 'Utilization %')]
        tiles = False
        scale = 1.0

    # Convert the value to a number
    gpu['value'] = pd.to_numeric(gpu['value']) * scale
    # Group by everything except value to get the mean value
    mean_cols = ['value','step']
    gpu_mean = gpu.groupby([c for c in gpu.columns if c not in mean_cols]).mean()
    # Reset indices
    gpu_mean.reset_index(inplace=True)
    #print(gpu_mean.to_string())
    for index, row in gpu_mean.sort_values(by=['rank','name']).iterrows():
        subdevice = '0'
        if tiles:
            p = re.compile('L0 All Engines, subdevice (\d+), Active Time')
            subdevice = p.findall(row['name'])[0]
        gpu_properties = df.loc[(df['resource'] == 'GPU') & (df['type'] == 'Property') & (df['rank'] == row['rank']) & (df['index'] == row['index'])]
        properties = {}
        for index2, row2 in gpu_properties.sort_values(by=['name']).iterrows():
            properties[row2['name']] = row2['value']
        addresses[(row['rank'],row['index'])] = list((addresses[(row['rank'],row['index'])], row['value'], subdevice, properties))
    #print(addresses)
    return addresses

def traverseHWTTree(tree, df, hwt_all):
    utilization = int(tree['utilization'])
    rank = int(tree['rank'])
    # Is this a processing unit (HWT)? if so, assign utilization
    if tree['name'].startswith("PU L#"):
        name = tree['name']
        tokens = name.split()
        osindex = tokens[2]
        osindex = int(osindex[2:])
        if osindex in df['index'].values:
            utilization = df.loc[df['index'] == osindex, 'value'].iloc[0]
            rank = int(df.loc[df['index'] == osindex, 'rank'].iloc[0])
            #print(osindex, utilization)
            # Get all metrics
            metrics = hwt_all['name'].to_numpy().tolist()
            for m in metrics:
                values = hwt_all.loc[(hwt_all['index'] == osindex) & (hwt_all['name'] == m), 'value'].to_numpy().tolist()
                tree[m] = values
    else:
        # otherwise, aggregate the children
        if 'children' in tree.keys():
            newChildren = []
            for c in tree['children']:
                newChild = traverseHWTTree(c, df, hwt_all)
                if tree['name'].startswith("Core L#"):
                    utilization += newChild['utilization']
                    rank = max(rank,newChild['rank'])
                newChildren.append(newChild)
            tree['children'] = newChildren
            utilization = utilization / len(tree['children'])
    tree['utilization'] = utilization;
    tree['detail_name'] = tree['detail_name'].lstrip(', ');
    tree['rank'] = rank;
    return tree

def traverseGPUTree(tree, in_rank, in_index, address, in_utilization, subdevice, properties):
    utilization = int(tree['utilization'])
    rank = int(tree['rank'])
    duplicate = False
    if tree['name'].startswith("PCIDev"):
        detail_name = tree['detail_name']
        if address in detail_name:
            # If we have already assigned this one, we need a duplicate
            if 'subdevice' in tree and tree['subdevice'] != subdevice:
                duplicate = True
                detail_name = detail_name[:detail_name.find('subdevice:')]
                tree['detail_name'] = detail_name + 'subdevice: ' + subdevice
            else:
                for key,value in properties.items():
                    tree['detail_name'] += ', ' + key + ': ' + value
                tree['detail_name'] += ', subdevice: ' + subdevice
            tree['subdevice'] = subdevice
            utilization = in_utilization
            rank = in_rank
            #print(rank, subdevice, utilization)
    else:
        if 'children' in tree.keys():
            newChildren = []
            #utilization = 0
            for c in tree['children']:
                # The child is mutable, so make a copy first
                oldChild = c.copy()
                newChild,duplicate = traverseGPUTree(c, in_rank, in_index, address, in_utilization, subdevice, properties)
                if duplicate:
                    # Keek the old child
                    newChildren.append(oldChild)
                    #utilization += oldChild['utilization']
                    duplicate = False
                #utilization += newChild['utilization']
                #rank = max(rank,newChild['rank'])
                newChildren.append(newChild)
            tree['children'] = newChildren
            #utilization = utilization / len(tree['children'])
    tree['utilization'] = utilization;
    tree['rank'] = rank;
    return tree,duplicate

def simplifyTree(tree):
    if 'children' in tree.keys():
        for c in tree['children']:
            simplifyTree(c)
    if tree['name'].startswith("PU L#"):
        tree['name'] = "PU"
    if tree['name'].startswith("Core L#"):
        tree['name'] = "Core"
    return tree

def updateTree(hwt_df, hwt_all, gpu_addresses):
    all_files = glob.glob(os.path.join('.', "zs.topology.*.json"))
    job = {}
    job['name'] = 'job'
    job['detail_name'] = '(jobid)'
    job['utilization'] = 0
    job['rank'] = -1;
    job['children'] = []
    for f in all_files:
        fp = open(f, 'r')
        tree = json.load(fp)
        job['children'].append(tree)
        fp.close()
    tree = traverseHWTTree(job, hwt_df, hwt_all)
    for key,value in gpu_addresses.items():
        tree,modified = traverseGPUTree(tree, key[0], key[1], value[0], value[1], value[2], value[3])
    tree = simplifyTree(tree)
    return tree

def main():
    args = parseArgs()
    print("Parsing CSV data...")
    df = parseData()
    print("Extracting CSV data...")
    hwt_df,hwt_all = parseHWTData(df)
    gpu_addresses = parseGPUData(df)
    print("Updating JSON tree...")
    tree = updateTree(hwt_df, hwt_all, gpu_addresses)
    print("Writing HTML...")
    if args.standalone:
        with open(standalone_template,'r') as file:
            populate_me = file.read()
        with open(d3_js,'r') as file:
            javascript = file.read()
        java_as_string = json.dumps(tree)
        from string import Template
        s = Template(populate_me)
        with open('zs-topology.html', 'w') as fp:
            fp.write(s.substitute(JSONDATA=java_as_string, SCRIPT_TEXT=javascript))

    else:
        with open(template,'r') as file:
            populate_me = file.read()
        with open(d3_js,'r') as file:
            javascript = file.read()
        java_as_string = json.dumps(tree)
        from string import Template
        s = Template(populate_me)
        with open('zs-topology.html', 'w') as fp:
            fp.write(s.substitute(JSONDATA=java_as_string))

if __name__ == '__main__':
    main()

