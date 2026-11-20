#!/usr/bin/env python3

import os
import sys
import json
import argparse

def find_unique_libs(search_dir, exclude_dirs=None, exclude_patterns=None):

    if not os.path.exists(search_dir):
        return []
    
    if exclude_dirs is None:
        exclude_dirs = []
    if exclude_patterns is None:
        exclude_patterns = []
    
    search_dir = os.path.abspath(search_dir)
    exclude_dirs = [os.path.abspath(d) for d in exclude_dirs if os.path.exists(d)]
    
    libs_by_name = {}
    
    for root, dirs, files in os.walk(search_dir):
        root_abs = os.path.abspath(root)
        if any(root_abs.startswith(exclude) for exclude in exclude_dirs):
            continue
        
        for file in files:
            if not file.endswith('.a'):
                continue

            if any(pattern in file for pattern in exclude_patterns):
                continue
            
            full_path = os.path.join(root, file)
            
            try:
                stat = os.stat(full_path)
                size = stat.st_size
                mtime = stat.st_mtime
            except:
                continue
            
            if file in libs_by_name:
                old_path, old_size, old_mtime = libs_by_name[file]
                
                if size > old_size:
                    libs_by_name[file] = (full_path, size, mtime)
                elif size == old_size and mtime > old_mtime:
                    libs_by_name[file] = (full_path, size, mtime)
            else:
                libs_by_name[file] = (full_path, size, mtime)

    return [info[0] for info in libs_by_name.values()]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("search_dir", help="Directory to search for libraries")
    parser.add_argument("--exclude-dirs", nargs="+", default=[],
                       help="Directories to exclude")
    parser.add_argument("--exclude-patterns", nargs="+", default=[],
                       help="Filename patterns to exclude")
    parser.add_argument("--json", action="store_true",
                       help="Output as JSON")
    parser.add_argument("--relative", action="store_true",
                       help="Output paths relative to search_dir")
    
    args = parser.parse_args()
    
    libs = find_unique_libs(
        args.search_dir,
        exclude_dirs=args.exclude_dirs,
        exclude_patterns=args.exclude_patterns
    )
    
    if args.relative:
        libs = [os.path.relpath(lib, args.search_dir) for lib in libs]
    
    libs.sort()

    if args.json:
        print(json.dumps(libs))
    else:
        for lib in libs:
            print(lib)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
