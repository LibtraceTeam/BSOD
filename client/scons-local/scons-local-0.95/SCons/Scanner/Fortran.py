"""SCons.Scanner.Fortran

This module implements the dependency scanner for Fortran code. 

"""

#
# Copyright (c) 2001, 2002, 2003, 2004 Steven Knight
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Scanner/Fortran.py 0.95.D001 2004/03/08 07:28:28 knight"


import re

import SCons.Node
import SCons.Node.FS
import SCons.Scanner
import SCons.Util
import SCons.Warnings

def FortranScan(fs = SCons.Node.FS.default_fs):
    """Return a prototype Scanner instance for scanning source files
    for Fortran INCLUDE statements"""
    scanner = SCons.Scanner.Classic("FortranScan",
                                    [".f", ".F", ".for", ".FOR"],
                                    "F77PATH",
                                    "(?i)INCLUDE[ \t]+'([\\w./\\\\]+)'",
                                    fs = fs)
    return scanner
