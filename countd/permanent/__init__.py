"""
Switchable permanent storage implementation.  See countd.permanent.abstract
for details on the interface that Keyspace, Index, and Deltas must expose.
Take care when mixing implementations that they share the same file formats.
"""

from countd.permanent.naive import Keyspace, Index, Deltas
