#!/usr/bin/env python3

from cnake import CMake

# import argparse
#
# parser = argparse.ArgumentParser()
#
# parser.add_argument('-G', action='store', dest='generator', default='Ninja')
# parser.add_argument('-S', required=True, action='store', dest='source_dir')
#
# args = parser.parse_args()

print('Creating project')


cmake_args = ['cmake', '-S', '.', '-G', 'Ninja', '-B', 'build']
# if args.generator:
#     cmake_args.extend(['-G', args.generator])


inst = CMake(cmake_args)
inst.project(["TestProject"])
inst.add_executable(["test_target", "main.cxx"])
inst.configure()
print('Done')
