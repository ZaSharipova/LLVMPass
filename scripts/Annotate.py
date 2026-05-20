#!/usr/bin/env python3

import os
import re
import sys
import pygraphviz as pgv
from collections import defaultdict

def read_log(path: str) -> tuple[dict[int, list[int]], list[tuple[int, int]]]:
    values = defaultdict(list)
    edges = []
    with open(path) as f:
        for line_no, raw in enumerate(f, 1):
            parts = raw.split()
            if not parts:
                continue
            
            if parts[0] == "edge":
                if len(parts) != 3:
                    print(f"Warning: bad edge line {line_no}: {raw!r}", file = sys.stderr)
                    continue
                try:
                    edges.append((int(parts[1]), int(parts[2])))
                except ValueError:
                    print(f"Warning: bad edge line {line_no}: {raw!r}", file = sys.stderr)
                continue

            if len(parts) != 2:
                continue
            try:
                node_id = int(parts[0])
                value = int(parts[1], 0)
            except ValueError:
                print(f"Warning: bad line {line_no}: {raw!r}.", file = sys.stderr)
                continue

            values[node_id].append(value)

    return values, edges

def format_values(vals: list[int]) -> str:
    if not vals:
        return "values: (not executed)"
    return "values: " + ", ".join(str(v) for v in vals)

def main():
    log_path = os.environ.get("MYPASS_LOG_FILE", "runtime_log.txt")
    graph_in = sys.argv[1] if len(sys.argv) > 1 else "dots/graph.dot"
    graph_out = sys.argv[2] if len(sys.argv) > 2 else "dots/graph_annotated.dot"

    values, edges = read_log(log_path)

    Graph = pgv.AGraph(graph_in)

    for node in Graph.nodes():
        node_id = int(node.name[1:])
        label = node.attr["label"]
        node.attr["label"] = label + "\\n" + format_values(values.get(node_id, []))

    existing_nodes = {node.name for node in Graph.nodes()}
    added = set()
    for from_id, to_id in edges:
        from_name = f"n{from_id}"
        to_name = f"n{to_id}"
        if from_name not in existing_nodes or to_name not in existing_nodes:
            continue
        
        key = (from_name, to_name)
        if key in added:
            continue
        
        added.add(key)
        Graph.add_edge(from_name, to_name, color = "red", style = "dashed", label = "call")

    Graph.write(graph_out)
    png_out = "images/" + os.path.basename(graph_out).replace(".dot", ".png")
    Graph.draw(png_out, prog = "dot")
    print(f"Written {graph_out} ({sum(len(v) for v in values.values())} value records).")

if __name__ == "__main__":
    main()