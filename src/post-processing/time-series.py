#!/usr/bin/env python3

import os.path
import sys
import glob
import ast
import matplotlib.pyplot as plt
import matplotlib as mpl
import re
import pandas as pd
import numpy as np
import itertools, sys
spinner = itertools.cycle(['-', '/', '|', '\\'])

avg = 'average:'
zeroonly = False

def parseFile(f, rank):
    # reading the file
    data = f.read()
    ingpu = False
    inlwp = False
    inhwt = False
    first = True
    data_into_list = data.split("\n")
    prefix = ''
    index = 0
    dfs = []
    didone = False
    saved_step = pd.DataFrame()
    # match *** Final thread summary: ***
    for l in data_into_list:
        line = l.strip()
        if len(line) == 0:
            continue
        # don't do anything until we reach the GPU section
        if re.match(r'GPU \d+ - \(metric: min  avg  max\)', line):
            inlwp = False
            inhwt = False
            ingpu = True
            prefix = 'GPU: '
            if didone:
                if not 'step' in df:
                    if saved_step.empty:
                        df['step'] = range(2, len(df) + 2)
                    else:
                        df = df.join(saved_step)
                df['index'] = index
                dfs.append(df)
                first = True
                index += 1
                didone = False
            index = 0
            continue
        elif ingpu and 'P2P Communication Summary:' in line:
            if didone:
                if not 'step' in df:
                    if saved_step.empty:
                        df['step'] = range(2, len(df) + 2)
                    else:
                        df = df.join(saved_step)
                df['index'] = index
                dfs.append(df)
                first = True
                index += 1
                didone = False
            # we are done
            break
        elif '*** Final thread summary: ***' in line:
            inhwt = False
            inlwp = True
            ingpu = False
            if didone:
                if not 'step' in df:
                    if saved_step.empty:
                        df['step'] = range(2, len(df) + 2)
                    else:
                        df = df.join(saved_step)
                df['index'] = index
                dfs.append(df)
                first = True
                index += 1
                didone = False
            index = 0
            continue
        elif 'Hardware Summary:' in line:
            inhwt = True
            inlwp = False
            ingpu = False
            if didone:
                if not 'step' in df:
                    if saved_step.empty:
                        df['step'] = range(2, len(df) + 2)
                    else:
                        df = df.join(saved_step)
                df['index'] = index
                dfs.append(df)
                first = True
                index += 1
                didone = False
            index = 0
            prefix = ''
            continue
        elif inhwt and 'CPU' in line:
            prefix = 'HWT: '
            if didone:
                if not 'step' in df:
                    if saved_step.empty:
                        df['step'] = range(2, len(df) + 2)
                    else:
                        df = df.join(saved_step)
                df['index'] = index
                dfs.append(df)
                first = True
                index += 1
                didone = False
            continue
        elif inlwp and re.match(r'New Thread: ', line):
            # 'New Thread: [Main,OpenMP] LWP 87159 - CPUs allowed: [1]'
            thread_type = line[line.find('[')+1:line.find(']')].strip()
            #prefix = 'LWP: ' + thread_type + ' '
            prefix = 'LWP: '
            if didone:
                if not 'step' in df:
                    if saved_step.empty:
                        df['step'] = range(2, len(df) + 2)
                    else:
                        df = df.join(saved_step)
                df['index'] = index
                dfs.append(df)
                first = True
                index += 1
                didone = False
            continue
        # does this line have time series data?
        elif (ingpu or inhwt) and line.find(avg) != -1:
            # remove the average
            line = line[:line.find(avg)]
            line = line.replace('Amps:','Amps')
            line = line.replace('In:','In')
            line = line.replace('Power:','Power')
            line = line.replace('Temp:','Temp')
            # get the name of the metric, and remove it
            name = line[:line.find(':')]
            if name != 'step':
                name = prefix + name
            # get the values
            line = line[line.find(':')+2:]
            row = ast.literal_eval(line)
            if max(list(row)) == 0:
                continue
            df2 = pd.DataFrame({name:pd.Series(list(row))})
            if name == 'step':
                saved_step = df2
            if first:
                df = df2
                first = False
            else:
                df = df.join(df2)
            didone = True
        # lwp data doesn't have averages... sheesh
        elif inlwp:
            # get the name of the metric, and remove it
            name = line[:line.find(':')]
            if name != 'step':
                name = prefix + name
            # get the values
            line = line[line.find(':')+2:]
            # are they numbers?
            if not re.search(r'\d', line):
                continue
            row = ast.literal_eval(line)
            if not isinstance(row,list):
                continue
            if max(list(row)) == 0:
                continue
            df2 = pd.DataFrame({name:pd.Series(list(row))})
            if name == 'step':
                saved_step = df2
            if first:
                df = df2
                first = False
            else:
                df = df.join(df2)
            didone = True
    df = pd.concat(dfs)
    df['rank'] = rank
    return df

def pinwheel(header):
    sys.stdout.write(next(spinner) + ' ' + header)   # write the next character
    sys.stdout.flush()                # flush stdout buffer (actual character display)
    sys.stdout.write('\r')            # erase the last written char

def iterateFiles():
    files = sorted(glob.glob('zs.*.log'))
    numfiles = len(files)
    rank = 0
    dfs = []
    # Hide the cursor
    print('\033[?25l', end="")
    for f in files:
        pinwheel('Parsing files...')
        with open(f, "r") as f_in:
            dfs.append(parseFile(f_in, rank))
            rank += 1
    df = pd.concat(dfs)
    pinwheel('\nDone.\n')
    # Restore the cursor
    print('\033[?25h', end="")
    return df

# 50th Percentile
def q25(x):
    return x.quantile(0.25)

# 75th Percentile
def q75(x):
    return x.quantile(0.75)

def graphit(df):
    ignoreme = ['rank', 'step', 'index', 'LWP: processor', 'GPU: Total GTT Bytes', 'GPU: Total VRAM Bytes', 'GPU: Total Visible VRAM Bytes']
    maxx = df['step'].max()
    minx = df['step'].min()
    for series_name, series in df.items():
        if series_name in ignoreme:
            continue
        print("Graphing '", series_name, "'", sep='')
        if zeroonly:
            dfclean = df[df['index'] == 0]
        else:
            dfclean = df
        data = dfclean.groupby(["step"],as_index=False).agg(
            min=pd.NamedAgg(column=series_name, aggfunc="min"),
            max=pd.NamedAgg(column=series_name, aggfunc="max"),
            q25s=pd.NamedAgg(column=series_name, aggfunc=q25),
            q75s=pd.NamedAgg(column=series_name, aggfunc=q75),
            mean=pd.NamedAgg(column=series_name, aggfunc="mean"))
        data.reset_index(inplace=True)

        ax = data.plot(figsize=(8, 5), x='step', y='mean', c='brown')
        ax.fill_between(x='step', y1='min', y2='max', data=data, label='min/max',
                        color=mpl.colors.to_rgba('brown', 0.15))
        ax.fill_between(x='step', y1='q25s', y2='q75s', data=data, label='0.25-0.75q',
                        color=mpl.colors.to_rgba('blue', 0.15))
        plt.xlim(minx,maxx)
        if zeroonly:
            plt.title(series_name + ', index 0')
        else:
            plt.title(series_name)
        plt.legend(loc='upper left', title=None)
        clean = series_name.replace(':','')
        if zeroonly:
            clean = clean + '_0'
        plt.savefig(clean+".pdf", format="pdf", bbox_inches="tight")
        plt.close()
    print('Done.')

def main():
    df = iterateFiles()
    graphit(df)

if __name__ == '__main__':
    main()

