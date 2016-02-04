import re
import os
import sys

import versioneer
versioneer.VCS = 'git'
versioneer.versionfile_source = 'lib/tracer/_version.py'
versioneer.versionfile_build = 'tracer/_version.py'
versioneer.tag_prefix = 'v'
versioneer.parentdir_prefix = 'tracer-'

try:
    import ez_setup
    ez_setup.use_setuptools()
    from setuptools import setup
    has_setuptools = True
except ImportError:
    try:
        from setuptools import setup
        has_setuptools = True
    except:
        pass
    has_setuptools = False
    from distutils.core import setup

basedir = os.path.abspath(os.path.dirname(__file__))

def join_path(*args):
    return os.path.abspath(os.path.normpath(os.path.join(*args)))

def read(*parts):
    with open(join_path(basedir, *parts), 'r') as f:
        return f.read()

def find_packages(base):
    packages = []
    for pkg in ['tracer']:
        path = os.path.join(base, pkg)
        for (dirname, subdirs, files) in os.walk(path):
            if '__init__.py' in files:
                (lib, fragment) = dirname.split(os.sep, 1)
                packages.append(fragment.replace(os.sep, '.'))
    return packages

def version_pre_versioneer():
    return (
        re.compile(r"^__version__ = '(.*?)'$", re.S)
          .match(read('lib', 'tracer', '__init__.py'))
          .group(1)
    )

def run_setup():
    setup(
        name='tracer',
        version=versioneer.get_version(),
        cmdclass=versioneer.get_cmdclass(),
        license='MIT',
        description="Tracer",
        author='Trent Nelson',
        author_email='trent@nospam.trent.me',
        url='http://github.com/tpn/tracer',
        packages=find_packages('lib'),
        package_dir={'': 'lib'},
        entry_points={
            'console_scripts': [
                'tracer = tracer.cli:main',
            ],
        },
        classifiers=[
            'Environment :: Console',
            'License :: OSI Approved :: MIT',
            'Development Status :: 5 - Production/Stable',
            'Operating System :: Microsoft :: Windows',
            'Intended Audience :: Developers',
            'Intended Audience :: System Administrators',
            'Intended Audience :: Information Technology',
            'Programming Language :: Python',
            'Programming Language :: Unix Shell',
        ],
    )

if __name__ == '__main__':
    run_setup()

# vim:set ts=8 sw=4 sts=4 tw=78 et:
