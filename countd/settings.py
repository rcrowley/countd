# TODO Make these command-line configurable where appropriate.

# Data directory.
DIRNAME = "/tmp/countd"

# Protocol field sizes.
KEYSPACE = 256
KEY = 256
COUNT = INCREMENT = 8 # int64_t (DO NOT CHANGE THIS!)

# Minimum number of commit logs.
FILES = 4

# Size of each commit log (bytes).
FILESIZE = 2 << 20
