function flow_layout(graph)
{
	"use strict";

	var coords = graph.nodes.map(n => { return { name: n.name, x: NaN, y: NaN }; });
	function successors(node_name)
	{
		return [...new Set(graph.connections.filter(c => c.out === node_name).map(c => c.in))];
	}
	function predecessors(node_name)
	{
		return [...new Set(graph.connections.filter(c => c.in === node_name).map(c => c.out))];
	}
	function index(node_name)
	{
		return coords.findIndex(n => n.name === node_name);
	}

	// Ordering on X
	function horiz_right(node_name)
	{
		var i = index(node_name);
		for(var n of successors(node_name))
		{
			var j = index(n);
			if(n !== node_name && (isNaN(coords[j].x) || true/* !ancestor */))
			{
				coords[j].x = Math.max(coords[i].x + 1, isNaN(coords[j].x) ? -Infinity : coords[j].x);
				horiz_right(n);
			}
		}
	}

	function horiz_left(node_name)
	{
		var i = index(node_name);
		for(var n of predecessors(node_name))
		{
			var j = index(n);
			if(n !== node_name && (isNaN(coords[j].x) || true/* !successor */))
			{
				coords[j].x = Math.min(coords[i].x - 1, isNaN(coords[j].x) ? Infinity : coords[j].x);
				horiz_left(n);
			}
		}
	}

	var roots = graph.nodes.filter(n => predecessors(n.name).length === 0).map(n => n.name);
	var leaves = graph.nodes.filter(n => successors(n.name).length === 0).map(n => n.name);
	for(var t = 0; t < roots.length; t++)
	{
		coords[index(roots[t])].x = 0;
		horiz_right(roots[t]);
	}
	for(t = 0; t < leaves.length; t++)
		horiz_left(leaves[t]);

	var all_x = coords.map(n => n.x);
	var decal = (Math.max(...all_x) - Math.min(...all_x)) / 2;
	for(var n of coords)
		n.x -= decal;

	// Ordering on Y

	// We're done
	coords.index = index;
	coords.find = function(n) { return coords[coords.index(n)]; };
	return coords;
}