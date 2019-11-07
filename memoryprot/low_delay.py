#!/usr/bin/env python3

# Created as part of my memory.low proportional reclaim work, which went into
# the kernel in 5.4, comparing old reclaim pressure calculations against the
# new ones to check they are sane.

import sys

lows = [2000]
mins = [1000]
cgroup_sizes = list(range(0, 5001, 500))
lruvec_size = 500

for memcg_low_reclaim in [True, False]:
    for low in lows:
        for min_ in mins:
            for cgroup_size in cgroup_sizes:
                if cgroup_size <= min_:
                    old = 0
                    new = old
                elif cgroup_size <= low and not memcg_low_reclaim:
                    old = 0
                    new = old
                else:
                    # Replicated logic
                    if memcg_low_reclaim:
                        old = lruvec_size * (cgroup_size - min_) // (low - min_)
                        new = lruvec_size - lruvec_size * min_ // cgroup_size
                    else:
                        old = lruvec_size * cgroup_size // max(min_, low) - lruvec_size
                        new = lruvec_size - lruvec_size * max(min_, low) // cgroup_size

                old = min(old, lruvec_size)
                new = min(new, lruvec_size)

                print("{},{},{},{}".format(cgroup_size, memcg_low_reclaim, old, new))
