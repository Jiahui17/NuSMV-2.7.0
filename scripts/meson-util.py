#!/usr/bin/env python

import argparse
import dataclasses
import logging
import os
import shutil
import sys

import mesonbuild.mdist

from pathlib import Path
from typing import Optional

logger = logging.getLogger(__name__)


def get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog=Path(sys.argv[0]).name,
        description=
            "Utilities for command `meson dist`.")
    parser.add_argument(
        '--debug', dest='log_level', action='store_const', const=logging.DEBUG,
        help="Set logger level to DEBUG")
    subparsers = parser.add_subparsers(
        dest='cmd', metavar='CMD',
        required=True,
        help="one of the following required subcommands:")

    # copytree
    dist = subparsers.add_parser('copytree',
        help="Install a build directory",
        description=
            "Copy a whole sub-directory from SRC to DST when called from"
            " `meson.add_install_script()`",
        epilog=
            "In meson 1.5.1, only named files or source directories can be"
            " installed.")
    dist.add_argument(
        'src', metavar='SRC',
        help="Path to content to copy from."
             " Must be a sub directory of MESON_BUILD_ROOT")
    dist.add_argument(
        'dst', metavar='DST',
        help="Path to copy content to")
    dist.add_argument(
        '--from-build-root', '-b', action='store_true',
        help=
            "Preprend environment variable MESON_BUILD_ROOT to SRC")
    dist.add_argument(
        '--to-install-destdir-prefix', '-d', action='store_true',
        help=
            "Preprend environment variable MESON_INSTALL_DESTDIR_PREFIX to DST")

    # fix
    dist = subparsers.add_parser('fix',
        help="Fix the source code being released",
        description=
            "Replace symbolink links with actual content when called from"
            " `meson.add_dist_script()`.",
        epilog=
            "In meson 1.5.1, release of a subproject with subsubprojects is not"
            " supported. The workaround consists in using symbolic links which"
            " are replaced with the real content when `meson dist` is run.")
    dist.add_argument(
        '--symlinks', '-s', nargs='+', metavar='FILE|DIR', default=[],
        help=
            "Fix symbolic links in meson's distribution folder by replacing"
            " the links with the actual file content")
    dist.add_argument(
        '--check', '-c', action='store_true',
        help=
            "After fixing the symlinks, check that MESON_DIST_ROOT does not"
            " contain links violating the sandbox")

    # pack
    pack = subparsers.add_parser('pack',
        help="Create an archive for installation",
        description=
            "Create an archive suitable for installation when called from"
            " `meson.add_install_script()`.",
        epilog=
            "Archives are created either in $MESON_BUILD_ROOT or"
            " $MESON_BUILD_ROOT/meson-dist, respectively when install"
            " phase is run from `meson install` or `meson dist`.")
    pack.add_argument(
        'name',
        help="Package base name")
    pack.add_argument(
        '--formats', '-f', default='',
        help="Comma separated list of archive types to create."
            f" Supports: {', '.join(get_formats())}."
             " Default value depends on --system")
    pack.add_argument(
        '--version', '-v',
        help="Version of the project to append to package name")
    pack.add_argument(
        '--system', '-s', choices=SYSTEMS, metavar='SYSTEM', default=sys.platform,
        help="Host machine's system to append to package name."
            " %(metavar)s is one of %(choices)s (default %(default)s)")
    pack.add_argument(
        '--owner', '-w', default=None,
        help="Owner for a tar archive (defaults to current owner)")
    pack.add_argument(
        '--group', '-g', default=None,
        help="Group for a tar archive (defaults to current group)")
    pack.add_argument(
        '--dry-run', action='store_true',
        help="Just pretend to create the archives. Environment variable"
            " MESON_INSTALL_DRY_RUN is always checked")

    return parser.parse_args()


def is_dist_install() -> bool:
    """Check whether the install phase is running from dist command"""
    env = get_environ()
    assert env is not None

    # When executing dist, install phase sees:
    # - MESON_SOURCE_ROOT=<user-build>/meson-private/dist-unpack/<name>-<ver>
    # - MESON_BUILD_ROOT =<user-build>/meson-private/dist-build
    return env.build_root.stem == 'dist-build'\
       and env.source_root.parent.stem == 'dist-unpack'\
       and env.source_root.parent.parent.samefile(env.build_root.parent)


def get_dist_path() -> Path|None:
    """Return the path where meson places dist's output"""
    if is_dist_install():
        env = get_environ()
        assert env is not None
        # MESON_BUILD_ROOT =<user-build>/meson-private/dist-build
        path = env.build_root.parent.parent / 'meson-dist'
        if path.exists():
            return path
    return None


### environ #################################################################

@dataclasses.dataclass(frozen=True)
class Environ:
    build_root            : Path
    destdir               : Path|None # when install
    dist_root             : Path|None # when dist
    estools_root          : Path
    install_destdir_prefix: Path|None
    install_dry_run       : bool
    install_prefix        : Path|None
    project_dist_root     : Path|None # when dist
    project_source_root   : Path|None # not when install
    source_root           : Path

    @staticmethod
    def get_option(var: str) -> bool:
        return var in os.environ

    @staticmethod
    def get_path(var: str) -> Path:
        return Path(os.environ[var])

    @staticmethod
    def get_optional_path(var: str) -> Path|None:
        return Path(os.environ[var]) if var in os.environ else None

_env: Optional[Environ] = None


def get_environ() -> Environ|None:
    global _env
    if _env is None:
        try:
            _env = Environ(
                build_root=
                    Environ.get_path('MESON_BUILD_ROOT'),
                destdir=
                    Environ.get_optional_path('DESTDIR'),
                dist_root=
                    Environ.get_optional_path('MESON_DIST_ROOT'),
                estools_root=
                    (Environ.get_path('MESON_SOURCE_ROOT') / '..').resolve(),
                install_destdir_prefix=
                    Environ.get_optional_path('MESON_INSTALL_DESTDIR_PREFIX'),
                install_dry_run=
                    Environ.get_option('MESON_INSTALL_DRY_RUN'),
                install_prefix=
                    Environ.get_optional_path('MESON_INSTALL_PREFIX'),
                project_dist_root=
                    Environ.get_optional_path('MESON_PROJECT_DIST_ROOT'),
                project_source_root=
                    Environ.get_optional_path('MESON_PROJECT_SOURCE_ROOT'),
                source_root=
                    Environ.get_path('MESON_SOURCE_ROOT'),
            )
        except KeyError as e:
            logger.error(f"no environment variable {e} (script not called via meson?)")
        else:
            logger.info(_env)
    return _env

### copytree ################################################################

def copytree(
    args: argparse.Namespace,
) -> bool:
    env = get_environ()
    assert env is not None

    src = Path(args.src)
    if args.from_build_root:
        src = env.build_root / src
    else:
        try:
            src.relative_to(env.build_root)
        except ValueError:
            logger.error(f"source path not relative to MESON_BUILD_ROOT: {src}")
            return False

    dst = Path(args.dst)
    if args.to_install_destdir_prefix:
        if 'MESON_INSTALL_DESTDIR_PREFIX' not in os.environ:
            logger.error("environment variable MESON_INSTALL_DESTDIR_PREFIX not set")
            return False
        dst = Path(os.environ['MESON_INSTALL_DESTDIR_PREFIX']) / dst

    shutil.copytree(src, dst, dirs_exist_ok=True)
    return True


### fix-symlinks ############################################################

def fix_symlink(
    symlink: str,
) -> bool:
    env = get_environ()
    assert env is not None
    assert env.dist_root
    assert env.project_source_root

    dist_dst = env.dist_root / symlink
    if not dist_dst.is_symlink():
        logger.warning(f"link does not exist: {dist_dst}")
        return False

    proj_dst = env.project_source_root / symlink
    proj_src = proj_dst.resolve()
    if not proj_src.exists():
        logger.warning(f"destination does not exist: {proj_src}")
        return False

    if proj_src.is_file():
        logger.info(f"copy file {proj_src} -> {dist_dst}")
        dist_dst.unlink()
        shutil.copy(proj_src, dist_dst)
    elif proj_src.is_dir():
        logger.info(f"copy dir {proj_src} -> {dist_dst}")
        dist_dst.unlink()
        shutil.copytree(proj_src, dist_dst)
    else:
        logger.warning(f"don't know how to handle {proj_src}")
        return False
    return True


def check_symlinks() -> bool:
    env = get_environ()
    assert env is not None
    assert env.dist_root

    logger.info("checking symlinks:")
    for (dirpath, _, filenames) in env.dist_root.walk():
        logger.debug(f"dir: {dirpath}")
        for filename in filenames:
            file = dirpath / filename
            if file.is_symlink():
                if file.readlink().is_relative_to(env.dist_root):
                    logger.info(f"{filename} OK")
                else:
                    logger.error(f"sandbox violation: {file}")
                    return False
    return True


def fix_symlinks(
    args: argparse.Namespace,
) -> bool:
    for symlink in args.symlinks:
        if not fix_symlink(symlink):
            return False
    if args.check:
        if not check_symlinks():
            return False
    return True



### pack ####################################################################

FORMAT_DEFAULT = {
    'cygwin' : 'xztar',
    'darwin' : 'xztar',
    'linux'  : 'xztar',
    'windows': 'zip'  ,
}

SYSTEMS = {
    'cygwin' : 'linux64',
    'darwin' : 'macosx' ,
    'linux'  : 'linux64',
    'windows': 'win64'  ,
}


class Rename:
    """Context manager for renaming package root directory

    We want 'rootdir' be the root folder in the package, not
    'destdir'. shutil.make_archive() works on existing folders only.
    Thus, we rename 'destdir' and restore it.
    """
    def __init__(self, destdir: Path, pkg_name: str):
        if not destdir or destdir == Path('/'):
            raise NotADirectoryError(destdir)
        self.destdir : Path = destdir
        self.rootdir : Path = destdir.parent
        self.basedir : Path = Path(pkg_name)

    def __enter__(self):
        logger.info(f"renaming: {self.destdir} -> {self.basedir}")
        self._destdir: Path = self.destdir.rename(self.rootdir / self.basedir)
        return self

    def __exit__(self, *_) -> bool:
        self._destdir.rename(self.destdir)
        logger.info("renaming reverted")
        return False


def get_formats() -> list[str]:
    return [name for name, _ in shutil.get_archive_formats()]


def make_pack(
    name: str,
    datadir: Path,
    formats: list[str],
    version: str,
    system: str,
    path: Path,
    owner: Optional[str] = None,
    group: Optional[str] = None,
    dry_run: bool = False,
) -> bool:
    env = get_environ()
    assert env is not None
    assert env.destdir

    system = SYSTEMS.get(system, 'unknown')
    full_name = f'{name}-{version}-{system}'

    try:
        with Rename(datadir, full_name) as rename:
            for format in formats:
                pkg_name = shutil.make_archive(
                    base_name = str(path / full_name),
                    format    = format,
                    root_dir  = rename.rootdir,
                    base_dir  = rename.basedir,
                    dry_run   = dry_run or env.install_dry_run,
                    owner     = owner,
                    group     = group,
                    logger    = logger,
                )
                if not pkg_name:
                    return False
                if not dry_run:
                    logger.info(f"made archive {pkg_name}")
                    print(f"Created {pkg_name}")
                    mesonbuild.mdist.create_hash(pkg_name)
    except NotADirectoryError as e:
        logger.info(f"skip archive creation: {e}")

    return True


def pack(
    args: argparse.Namespace,
) -> bool:
    env = get_environ()
    assert env is not None

    if datadir := env.destdir:
        if args.system == 'windows':
            # avoid /msys64 in .zip
            datadir /= 'msys64'
        path = get_dist_path() or env.build_root

        if not make_pack(
                name    = args.name,
                datadir = datadir,
                formats = (args.formats or FORMAT_DEFAULT[args.system]).split(','),
                version = args.version,
                system  = args.system,
                path    = path,
                owner   = args.owner,
                group   = args.group,
                dry_run = args.dry_run):
            return False

    return True


### main ####################################################################

def main() -> int:
    logging.basicConfig()
    args = get_args()
    logger.setLevel(args.log_level or logging.WARNING)

    if get_environ() is None:
        return 1

    ret: bool = False
    match args.cmd:
        case 'copytree':
            ret = copytree(args)
        case 'fix':
            ret = fix_symlinks(args)
        case 'pack':
            ret = pack(args)
    return 0 if ret == True else 1


if __name__ == "__main__":
    exit(main())
