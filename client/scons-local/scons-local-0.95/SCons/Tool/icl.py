"""engine.SCons.Tool.icl

Tool-specific initialization for the Intel C/C++ compiler.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/icl.py 0.95.D001 2004/03/08 07:28:28 knight"

import os.path
import string

import SCons.Tool.msvc
import SCons.Util

# Find Intel compiler:
# Could enumerate subkeys here to be more flexible.
def get_intel_compiler_top(version):
    """
    Return the main path to the top-level dir of the Intel compiler,
    using the given version or latest if 0.
    The compiler will be in <top>/Bin/icl.exe,
    the include dir is <top>/Include, etc.
    """

    if version == 0:
        version = "7.0"                   # XXX: should scan for latest

    if not SCons.Util.can_read_reg:
        raise SCons.Errors.InternalError, "No Windows registry module was found"

    K = ('Software\\Intel\\' +
         'Intel(R) C/C++ Compiler for 32-bit apps, Version ' + version)
    # Note: v5 had slightly different key:
    #  HKCU\Software\Intel\Intel C/C++ Compiler for 32-bit apps, Version 5.0
    # Note no (R).
    try:
        k = SCons.Util.RegOpenKeyEx(SCons.Util.HKEY_CURRENT_USER, K)
    except SCons.Util.RegError:
        return None

    try:
        # On my machine, this returns:
        #  c:\Program Files\Intel\Compiler70
        top = SCons.Util.RegQueryValueEx(k, "Directory")[0]
    except SCons.Util.RegError:
        raise SCons.Errors.InternalError, "%s was not found in the registry."%K

    if os.path.exists(os.path.join(top, "ia32")):
        top = os.path.join(top, "ia32")

    if not os.path.exists(os.path.join(top, "Bin", "icl.exe")):
        raise SCons.Errors.InternalError, "Can't find Intel compiler in %s"%top

    return top


def generate(env):
    """Add Builders and construction variables for icl to an Environment."""
    SCons.Tool.msvc.generate(env)

    try:
        icltop = get_intel_compiler_top(0)
    except (SCons.Util.RegError, SCons.Errors.InternalError):
        icltop = None

    if icltop:
        env.PrependENVPath('INCLUDE', os.path.join(icltop, 'Include'))
        env.PrependENVPath('LIB', os.path.join(icltop, 'Lib'))
        env.PrependENVPath('PATH', os.path.join(icltop, 'Bin'))

    env['CC']        = 'icl'
    env['CXX']        = 'icl'
    env['LINK']        = 'xilink'

    env['ENV']['INTEL_LICENSE_FILE'] = r'C:\Program Files\Common Files\Intel\Licenses'

def exists(env):
    try:
        top = get_intel_compiler_top(0)
    except (SCons.Util.RegError, SCons.Errors.InternalError):
        top = None

    if not top:
        return env.Detect('icl')
    return top is not None
