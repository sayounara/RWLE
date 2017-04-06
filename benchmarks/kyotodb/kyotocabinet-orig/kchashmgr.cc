/*************************************************************************************************
 * The command line utility of the file hash database
 *                                                               Copyright (C) 2009-2010 FAL Labs
 * This file is part of Kyoto Cabinet.
 * This program is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either version
 * 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************************************/


#include <kchashdb.h>
#include "cmdcommon.h"


// global variables
const char* g_progname;                  // program name


// function prototypes
int main(int argc, char** argv);
static void usage();
static void dberrprint(kc::BasicDB* db, const char* info);
static int32_t runcreate(int argc, char** argv);
static int32_t runinform(int argc, char** argv);
static int32_t runset(int argc, char** argv);
static int32_t runremove(int argc, char** argv);
static int32_t runget(int argc, char** argv);
static int32_t runlist(int argc, char** argv);
static int32_t runclear(int argc, char** argv);
static int32_t runimport(int argc, char** argv);
static int32_t runcopy(int argc, char** argv);
static int32_t rundump(int argc, char** argv);
static int32_t runload(int argc, char** argv);
static int32_t rundefrag(int argc, char** argv);
static int32_t runcheck(int argc, char** argv);
static int32_t proccreate(const char* path, int32_t oflags,
                          int32_t apow, int32_t fpow, int32_t opts, int64_t bnum);
static int32_t procinform(const char* path, int32_t oflags, bool st);
static int32_t procset(const char* path, const char* kbuf, size_t ksiz,
                       const char* vbuf, size_t vsiz, int32_t oflags, int32_t mode);
static int32_t procremove(const char* path, const char* kbuf, size_t ksiz, int32_t oflags);
static int32_t procget(const char* path, const char* kbuf, size_t ksiz,
                       int32_t oflags, bool px, bool pz);
static int32_t proclist(const char* path, const char*kbuf, size_t ksiz, int32_t oflags,
                        int64_t max, bool pv, bool px);
static int32_t procclear(const char* path, int32_t oflags);
static int32_t procimport(const char* path, const char* file, int32_t oflags, bool sx);
static int32_t proccopy(const char* path, const char* file, int32_t oflags);
static int32_t procdump(const char* path, const char* file, int32_t oflags);
static int32_t procload(const char* path, const char* file, int32_t oflags);
static int32_t procdefrag(const char* path, int32_t oflags);
static int32_t proccheck(const char* path, int32_t oflags);


// main routine
int main(int argc, char** argv) {
  g_progname = argv[0];
  kc::setstdiobin();
  if (argc < 2) usage();
  int32_t rv = 0;
  if (!std::strcmp(argv[1], "create")) {
    rv = runcreate(argc, argv);
  } else if (!std::strcmp(argv[1], "inform")) {
    rv = runinform(argc, argv);
  } else if (!std::strcmp(argv[1], "set")) {
    rv = runset(argc, argv);
  } else if (!std::strcmp(argv[1], "remove")) {
    rv = runremove(argc, argv);
  } else if (!std::strcmp(argv[1], "get")) {
    rv = runget(argc, argv);
  } else if (!std::strcmp(argv[1], "list")) {
    rv = runlist(argc, argv);
  } else if (!std::strcmp(argv[1], "clear")) {
    rv = runclear(argc, argv);
  } else if (!std::strcmp(argv[1], "import")) {
    rv = runimport(argc, argv);
  } else if (!std::strcmp(argv[1], "copy")) {
    rv = runcopy(argc, argv);
  } else if (!std::strcmp(argv[1], "dump")) {
    rv = rundump(argc, argv);
  } else if (!std::strcmp(argv[1], "load")) {
    rv = runload(argc, argv);
  } else if (!std::strcmp(argv[1], "defrag")) {
    rv = rundefrag(argc, argv);
  } else if (!std::strcmp(argv[1], "check")) {
    rv = runcheck(argc, argv);
  } else if (!std::strcmp(argv[1], "version") || !std::strcmp(argv[1], "--version")) {
    printversion();
  } else {
    usage();
  }
  return rv;
}


// print the usage and exit
static void usage() {
  eprintf("%s: the command line utility of the file hash database of Kyoto Cabinet\n",
          g_progname);
  eprintf("\n");
  eprintf("usage:\n");
  eprintf("  %s create [-otr] [-onl|-otl|-onr] [-apow num] [-fpow num] [-ts] [-tl] [-tc]"
          " [-bnum num] path\n", g_progname);
  eprintf("  %s inform [-onl|-otl|-onr] [-st] path\n", g_progname);
  eprintf("  %s set [-onl|-otl|-onr] [-add|-rep|-app|-inci|-incd] [-sx] path key value\n",
          g_progname);
  eprintf("  %s remove [-onl|-otl|-onr] [-sx] path key\n", g_progname);
  eprintf("  %s get [-onl|-otl|-onr] [-sx] [-px] [-pz] path key\n", g_progname);
  eprintf("  %s list [-onl|-otl|-onr] [-max num] [-sx] [-pv] [-px] path [key]\n", g_progname);
  eprintf("  %s clear [-onl|-otl|-onr] path\n", g_progname);
  eprintf("  %s import [-onl|-otl|-onr] [-sx] path [file]\n", g_progname);
  eprintf("  %s copy [-onl|-otl|-onr] path file\n", g_progname);
  eprintf("  %s dump [-onl|-otl|-onr] path [file]\n", g_progname);
  eprintf("  %s load [-otr] [-onl|-otl|-onr] path [file]\n", g_progname);
  eprintf("  %s defrag [-onl|-otl|-onr] path\n", g_progname);
  eprintf("  %s check [-onl|-otl|-onr] path\n", g_progname);
  eprintf("\n");
  std::exit(1);
}


// print error message of database
static void dberrprint(kc::BasicDB* db, const char* info) {
  const kc::BasicDB::Error& err = db->error();
  eprintf("%s: %s: %s: %d: %s: %s\n",
          g_progname, info, db->path().c_str(), err.code(), err.name(), err.message());
}


// parse arguments of create command
static int32_t runcreate(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  int32_t apow = -1;
  int32_t fpow = -1;
  int32_t opts = 0;
  int64_t bnum = -1;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-otr")) {
        oflags |= kc::HashDB::OTRUNCATE;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-apow")) {
        if (++i >= argc) usage();
        apow = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-fpow")) {
        if (++i >= argc) usage();
        fpow = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-ts")) {
        opts |= kc::HashDB::TSMALL;
      } else if (!std::strcmp(argv[i], "-tl")) {
        opts |= kc::HashDB::TLINEAR;
      } else if (!std::strcmp(argv[i], "-tc")) {
        opts |= kc::HashDB::TCOMPRESS;
      } else if (!std::strcmp(argv[i], "-bnum")) {
        if (++i >= argc) usage();
        bnum = kc::atoix(argv[i]);
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = proccreate(path, oflags, apow, fpow, opts, bnum);
  return rv;
}


// parse arguments of inform command
static int32_t runinform(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  bool st = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-st")) {
        st = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procinform(path, oflags, st);
  return rv;
}


// parse arguments of set command
static int32_t runset(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  const char* vstr = NULL;
  int32_t oflags = 0;
  int32_t mode = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-add")) {
        mode = 'a';
      } else if (!std::strcmp(argv[i], "-rep")) {
        mode = 'r';
      } else if (!std::strcmp(argv[i], "-app")) {
        mode = 'c';
      } else if (!std::strcmp(argv[i], "-inci")) {
        mode = 'i';
      } else if (!std::strcmp(argv[i], "-incd")) {
        mode = 'd';
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else if (!vstr) {
      vstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !kstr || !vstr) usage();
  char* kbuf;
  size_t ksiz;
  char* vbuf;
  size_t vsiz;
  if (sx) {
    kbuf = kc::hexdecode(kstr, &ksiz);
    kstr = kbuf;
    vbuf = kc::hexdecode(vstr, &vsiz);
    vstr = vbuf;
  } else {
    ksiz = std::strlen(kstr);
    kbuf = NULL;
    vsiz = std::strlen(vstr);
    vbuf = NULL;
  }
  int32_t rv = procset(path, kstr, ksiz, vstr, vsiz, oflags, mode);
  delete[] kbuf;
  delete[] vbuf;
  return rv;
}


// parse arguments of remove command
static int32_t runremove(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  int32_t oflags = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !kstr) usage();
  char* kbuf;
  size_t ksiz;
  if (sx) {
    kbuf = kc::hexdecode(kstr, &ksiz);
    kstr = kbuf;
  } else {
    ksiz = std::strlen(kstr);
    kbuf = NULL;
  }
  int32_t rv = procremove(path, kstr, ksiz, oflags);
  delete[] kbuf;
  return rv;
}


// parse arguments of get command
static int32_t runget(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  int32_t oflags = 0;
  bool sx = false;
  bool px = false;
  bool pz = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else if (!std::strcmp(argv[i], "-px")) {
        px = true;
      } else if (!std::strcmp(argv[i], "-pz")) {
        pz = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !kstr) usage();
  char* kbuf;
  size_t ksiz;
  if (sx) {
    kbuf = kc::hexdecode(kstr, &ksiz);
    kstr = kbuf;
  } else {
    ksiz = std::strlen(kstr);
    kbuf = NULL;
  }
  int32_t rv = procget(path, kstr, ksiz, oflags, px, pz);
  delete[] kbuf;
  return rv;
}


// parse arguments of list command
static int32_t runlist(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  int32_t oflags = 0;
  int64_t max = -1;
  bool sx = false;
  bool pv = false;
  bool px = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-max")) {
        if (++i >= argc) usage();
        max = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else if (!std::strcmp(argv[i], "-pv")) {
        pv = true;
      } else if (!std::strcmp(argv[i], "-px")) {
        px = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  char* kbuf = NULL;
  size_t ksiz = 0;
  if (kstr) {
    if (sx) {
      kbuf = kc::hexdecode(kstr, &ksiz);
      kstr = kbuf;
    } else {
      ksiz = std::strlen(kstr);
      kbuf = new char[ksiz+1];
      std::memcpy(kbuf, kstr, ksiz);
      kbuf[ksiz] = '\0';
    }
  }
  int32_t rv = proclist(path, kbuf, ksiz, oflags, max, pv, px);
  delete[] kbuf;
  return rv;
}


// parse arguments of clear command
static int32_t runclear(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procclear(path, oflags);
  return rv;
}


// parse arguments of import command
static int32_t runimport(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procimport(path, file, oflags, sx);
  return rv;
}


// parse arguments of copy command
static int32_t runcopy(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !file) usage();
  int32_t rv = proccopy(path, file, oflags);
  return rv;
}


// parse arguments of dump command
static int32_t rundump(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procdump(path, file, oflags);
  return rv;
}


// parse arguments of load command
static int32_t runload(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-otr")) {
        oflags |= kc::HashDB::OTRUNCATE;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procload(path, file, oflags);
  return rv;
}


// parse arguments of defrag command
static int32_t rundefrag(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procdefrag(path, oflags);
  return rv;
}


// parse arguments of check command
static int32_t runcheck(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::HashDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::HashDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::HashDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = proccheck(path, oflags);
  return rv;
}


// perform create command
static int32_t proccreate(const char* path, int32_t oflags,
                          int32_t apow, int32_t fpow, int32_t opts, int64_t bnum) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (apow >= 0) db.tune_alignment(apow);
  if (fpow >= 0) db.tune_fbp(fpow);
  if (opts > 0) db.tune_options(opts);
  if (bnum > 0) db.tune_buckets(bnum);
  if (!db.open(path, kc::HashDB::OWRITER | kc::HashDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform inform command
static int32_t procinform(const char* path, int32_t oflags, bool st) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (st) {
    std::map<std::string, std::string> status;
    status["opaque"] = "";
    status["fbpnum_used"] = "";
    status["bnum_used"] = "";
    if (db.status(&status)) {
      uint32_t type = kc::atoi(status["type"].c_str());
      oprintf("type: %s (%s) (type=0x%02X)\n",
              kc::BasicDB::typecname(type), kc::BasicDB::typestring(type), type);
      uint32_t rtype = kc::atoi(status["realtype"].c_str());
      if (rtype > 0 && rtype != type)
        oprintf("real type: %s (%s) (realtype=0x%02X)\n",
                kc::BasicDB::typecname(rtype), kc::BasicDB::typestring(rtype), rtype);
      uint32_t chksum = kc::atoi(status["chksum"].c_str());
      oprintf("format version: %s (libver=%s.%s) (chksum=0x%02X)\n", status["fmtver"].c_str(),
              status["libver"].c_str(), status["librev"].c_str(), chksum);
      oprintf("path: %s\n", status["path"].c_str());
      int32_t flags = kc::atoi(status["flags"].c_str());
      oprintf("status flags:");
      if (flags & kc::HashDB::FOPEN) oprintf(" open");
      if (flags & kc::HashDB::FFATAL) oprintf(" fatal");
      oprintf(" (flags=%d)", flags);
      if (kc::atoi(status["recovered"].c_str()) > 0) oprintf(" (recovered)");
      if (kc::atoi(status["reorganized"].c_str()) > 0) oprintf(" (reorganized)");
      if (kc::atoi(status["trimmed"].c_str()) > 0) oprintf(" (trimmed)");
      oprintf("\n", flags);
      int32_t apow = kc::atoi(status["apow"].c_str());
      oprintf("alignment: %d (apow=%d)\n", 1 << apow, apow);
      int32_t fpow = kc::atoi(status["fpow"].c_str());
      int32_t fbpnum = fpow > 0 ? 1 << fpow : 0;
      int32_t fbpused = kc::atoi(status["fbpnum_used"].c_str());
      int64_t frgcnt = kc::atoi(status["frgcnt"].c_str());
      oprintf("free block pool: %d (fpow=%d) (used=%d) (frg=%lld)\n",
              fbpnum, fpow, fbpused, (long long)frgcnt);
      int32_t opts = kc::atoi(status["opts"].c_str());
      oprintf("options:");
      if (opts & kc::HashDB::TSMALL) oprintf(" small");
      if (opts & kc::HashDB::TLINEAR) oprintf(" linear");
      if (opts & kc::HashDB::TCOMPRESS) oprintf(" compress");
      oprintf(" (opts=%d)\n", opts);
      if (status["opaque"].size() >= 16) {
        const char* opaque = status["opaque"].c_str();
        oprintf("opaque:");
        if (std::count(opaque, opaque + 16, 0) != 16) {
          for (int32_t i = 0; i < 16; i++) {
            oprintf(" %02X", ((unsigned char*)opaque)[i]);
          }
        } else {
          oprintf(" 0");
        }
        oprintf("\n");
      }
      int64_t bnum = kc::atoi(status["bnum"].c_str());
      int64_t bnumused = kc::atoi(status["bnum_used"].c_str());
      int64_t count = kc::atoi(status["count"].c_str());
      double load = 0;
      if (count > 0 && bnumused > 0) {
        load = (double)count / bnumused;
        if (!(opts & kc::HashDB::TLINEAR)) load = std::log(load + 1) / std::log(2.0);
      }
      oprintf("buckets: %lld (used=%lld) (load=%.2f)\n",
              (long long)bnum, (long long)bnumused, load);
      std::string cntstr = unitnumstr(count);
      oprintf("count: %lld (%s)\n", count, cntstr.c_str());
      int64_t size = kc::atoi(status["size"].c_str());
      int64_t msiz = kc::atoi(status["msiz"].c_str());
      int64_t realsize = kc::atoi(status["realsize"].c_str());
      std::string sizestr = unitnumstrbyte(size);
      oprintf("size: %lld (%s) (map=%lld)", size, sizestr.c_str(), (long long)msiz);
      if (size != realsize) oprintf(" (gap=%lld)", (long long)(realsize - size));
      oprintf("\n");
    } else {
      dberrprint(&db, "DB::status failed");
      err = true;
    }
  } else {
    uint8_t flags = db.flags();
    if (flags != 0) {
      oprintf("status:");
      if (flags & kc::HashDB::FOPEN) oprintf(" open");
      if (flags & kc::HashDB::FFATAL) oprintf(" fatal");
      oprintf("\n");
    }
    oprintf("count: %lld\n", (long long)db.count());
    oprintf("size: %lld\n", (long long)db.size());
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform set command
static int32_t procset(const char* path, const char* kbuf, size_t ksiz,
                       const char* vbuf, size_t vsiz, int32_t oflags, int32_t mode) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  switch (mode) {
    default: {
      if (!db.set(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::set failed");
        err = true;
      }
      break;
    }
    case 'a': {
      if (!db.add(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::add failed");
        err = true;
      }
      break;
    }
    case 'r': {
      if (!db.replace(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::replace failed");
        err = true;
      }
      break;
    }
    case 'c': {
      if (!db.append(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::append failed");
        err = true;
      }
      break;
    }
    case 'i': {
      int64_t onum = db.increment(kbuf, ksiz, kc::atoi(vbuf));
      if (onum == INT64_MIN) {
        dberrprint(&db, "DB::increment failed");
        err = true;
      } else {
        oprintf("%lld\n", (long long)onum);
      }
      break;
    }
    case 'd': {
      double onum = db.increment_double(kbuf, ksiz, kc::atof(vbuf));
      if (kc::chknan(onum)) {
        dberrprint(&db, "DB::increment_double failed");
        err = true;
      } else {
        oprintf("%f\n", onum);
      }
      break;
    }
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform remove command
static int32_t procremove(const char* path, const char* kbuf, size_t ksiz, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.remove(kbuf, ksiz)) {
    dberrprint(&db, "DB::remove failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform get command
static int32_t procget(const char* path, const char* kbuf, size_t ksiz,
                       int32_t oflags, bool px, bool pz) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  size_t vsiz;
  char* vbuf = db.get(kbuf, ksiz, &vsiz);
  if (vbuf) {
    printdata(vbuf, vsiz, px);
    if (!pz) oprintf("\n");
    delete[] vbuf;
  } else {
    dberrprint(&db, "DB::get failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform list command
static int32_t proclist(const char* path, const char*kbuf, size_t ksiz, int32_t oflags,
                        int64_t max, bool pv, bool px) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  class VisitorImpl : public kc::DB::Visitor {
  public:
    explicit VisitorImpl(bool pv, bool px) : pv_(pv), px_(px) {}
  private:
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      printdata(kbuf, ksiz, px_);
      if (pv_) {
        oprintf("\t");
        printdata(vbuf, vsiz, px_);
      }
      oprintf("\n");
      return NOP;
    }
    bool pv_;
    bool px_;
  } visitor(pv, px);
  if (kbuf || max >= 0) {
    if (max < 0) max = INT64_MAX;
    kc::HashDB::Cursor cur(&db);
    if (kbuf) {
      if (!cur.jump(kbuf, ksiz) && db.error() != kc::BasicDB::Error::NOREC) {
        dberrprint(&db, "Cursor::jump failed");
        err = true;
      }
    } else {
      if (!cur.jump() && db.error() != kc::BasicDB::Error::NOREC) {
        dberrprint(&db, "Cursor::jump failed");
        err = true;
      }
    }
    while (!err && max > 0) {
      if (!cur.accept(&visitor, false, true)) {
        if (db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "Cursor::accept failed");
          err = true;
        }
        break;
      }
      max--;
    }
  } else {
    if (!db.iterate(&visitor, false)) {
      dberrprint(&db, "DB::iterate failed");
      err = true;
    }
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform clear command
static int32_t procclear(const char* path, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.clear()) {
    dberrprint(&db, "DB::clear failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform import command
static int32_t procimport(const char* path, const char* file, int32_t oflags, bool sx) {
  std::istream *is = &std::cin;
  std::ifstream ifs;
  if (file) {
    ifs.open(file, std::ios_base::in | std::ios_base::binary);
    if (!ifs) {
      eprintf("%s: %s: open error\n", g_progname, file);
      return 1;
    }
    is = &ifs;
  }
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OWRITER | kc::HashDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  int64_t cnt = 0;
  std::string line;
  std::vector<std::string> fields;
  while (!err && mygetline(is, &line)) {
    cnt++;
    kc::strsplit(line, '\t', &fields);
    if (sx) {
      std::vector<std::string>::iterator it = fields.begin();
      std::vector<std::string>::iterator itend = fields.end();
      while (it != itend) {
        size_t esiz;
        char* ebuf = kc::hexdecode(it->c_str(), &esiz);
        it->clear();
        it->append(ebuf, esiz);
        delete[] ebuf;
        it++;
      }
    }
    switch (fields.size()) {
      case 2: {
        if (!db.set(fields[0], fields[1])) {
          dberrprint(&db, "DB::set failed");
          err = true;
        }
        break;
      }
      case 1: {
        if (!db.remove(fields[0]) && db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "DB::remove failed");
          err = true;
        }
        break;
      }
    }
    oputchar('.');
    if (cnt % 50 == 0) oprintf(" (%lld)\n", (long long)cnt);
  }
  if (cnt % 50 > 0) oprintf(" (%lld)\n", (long long)cnt);
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform copy command
static int32_t proccopy(const char* path, const char* file, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  DotChecker checker(&std::cout, -100);
  if (!db.copy(file, &checker)) {
    dberrprint(&db, "DB::copy failed");
    err = true;
  }
  oprintf(" (end)\n");
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  if (!err) oprintf("%lld blocks were copied successfully\n", (long long)checker.count());
  return err ? 1 : 0;
}


// perform dump command
static int32_t procdump(const char* path, const char* file, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (file) {
    DotChecker checker(&std::cout, 1000);
    if (!db.dump_snapshot(file)) {
      dberrprint(&db, "DB::dump_snapshot");
      err = true;
    }
    oprintf(" (end)\n");
    if (!err) oprintf("%lld records were dumped successfully\n", (long long)checker.count());
  } else {
    if (!db.dump_snapshot(&std::cout)) {
      dberrprint(&db, "DB::dump_snapshot");
      err = true;
    }
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform load command
static int32_t procload(const char* path, const char* file, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OWRITER | kc::HashDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (file) {
    DotChecker checker(&std::cout, -1000);
    if (!db.load_snapshot(file)) {
      dberrprint(&db, "DB::load_snapshot");
      err = true;
    }
    oprintf(" (end)\n");
    if (!err) oprintf("%lld records were loaded successfully\n", (long long)checker.count());
  } else {
    DotChecker checker(&std::cout, -1000);
    if (!db.load_snapshot(&std::cin)) {
      dberrprint(&db, "DB::load_snapshot");
      err = true;
    }
    oprintf(" (end)\n");
    if (!err) oprintf("%lld records were loaded successfully\n", (long long)checker.count());
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform defrag command
static int32_t procdefrag(const char* path, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.defrag(0)) {
    dberrprint(&db, "DB::defrag failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform check command
static int32_t proccheck(const char* path, int32_t oflags) {
  kc::HashDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::HashDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  kc::HashDB::Cursor cur(&db);
  if (!cur.jump() && db.error() != kc::BasicDB::Error::NOREC) {
    dberrprint(&db, "DB::jump failed");
    err = true;
  }
  int64_t cnt = 0;
  while (!err) {
    size_t ksiz;
    const char* vbuf;
    size_t vsiz;
    char* kbuf = cur.get(&ksiz, &vbuf, &vsiz);
    if (kbuf) {
      cnt++;
      size_t rsiz;
      char* rbuf = db.get(kbuf, ksiz, &rsiz);
      if (rbuf) {
        if (rsiz != vsiz || std::memcmp(rbuf, vbuf, rsiz)) {
          dberrprint(&db, "DB::get failed");
          err = true;
        }
        delete[] rbuf;
      } else {
        dberrprint(&db, "DB::get failed");
        err = true;
      }
      delete[] kbuf;
      if (cnt % 1000 == 0) {
        oputchar('.');
        if (cnt % 50000 == 0) oprintf(" (%lld)\n", (long long)cnt);
      }
    } else {
      if (db.error() != kc::BasicDB::Error::NOREC) {
        dberrprint(&db, "Cursor::get failed");
        err = true;
      }
      break;
    }
    if (!cur.step() && db.error() != kc::BasicDB::Error::NOREC) {
      dberrprint(&db, "Cursor::step failed");
      err = true;
    }
  }
  oprintf(" (end)\n");
  if (db.count() != cnt) {
    dberrprint(&db, "DB::count failed");
    err = true;
  }
  kc::File::Status sbuf;
  if (kc::File::status(path, &sbuf)) {
    if (db.size() != sbuf.size && sbuf.size != kc::HDBDEFMSIZ) {
      dberrprint(&db, "DB::size failed");
      err = true;
    }
  } else {
    dberrprint(&db, "File::status failed");
    err = true;
  }
  if (db.flags() & kc::HashDB::FFATAL) {
    dberrprint(&db, "DB::flags indicated fatal error");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  if (!err) oprintf("%lld records were checked successfully\n", (long long)cnt);
  return err ? 1 : 0;
}



// END OF FILE