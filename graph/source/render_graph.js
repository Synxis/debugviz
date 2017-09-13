function setup_graph_rendering(graph)
{
	var node_width = 170;
	var node_padding = 10;
	var slot_height = 40;
	var slot_radius = 10;
	var title_height = 40;
	var separator_height = 10;
	var separator_count = 8;
	var edge_strength = 60;

	// Base frame
	var svg = d3.select('svg');
	var root = svg.append('g');
	var zoom = d3.zoom()
		.scaleExtent([0.05, 2])
		.on('zoom', function()
			{
				root.attr('transform', d3.zoomTransform(svg.node()).toString());
				update();
			});
	svg.call(zoom);

	// Nodes
	var nodes = root.selectAll('.node')
		.data(graph.nodes)
		.enter().append('g')
		.attr('class', 'node')
		.call(d3.drag()
			.on('start', start_drag)
			.on('drag', on_drag)
			.on('end', end_drag));
	nodes.append('rect')
		.attr('width', node_width)
		.attr('height', d => title_height + separator_height + slot_height * Math.max(d.inputs.length, d.outputs.length));
	nodes.append('text')
		.attr('text-anchor', 'middle')
		.attr('dominant-baseline', 'middle')
		.attr('x', node_width / 2.0)
		.attr('y', title_height / 2.0)
		.text(d => d.name);
	nodes.append('line')
		.attr('x1', slot_radius)
		.attr('x2', node_width - slot_radius)
		.attr('y1', title_height)
		.attr('y2', title_height)
		.attr('stroke-dasharray', (node_width - 2 * slot_radius) / (2 * separator_count - 1));

	var inputs = nodes.selectAll('.input')
		.data(d => d.inputs.map((e, i) => { return { 'name': e, 'index': i }; }))
		.enter().append('g')
		.attr('class', 'input')
		.attr('transform', d => 'translate(0, ' + (title_height + separator_height + slot_height * d.index) + ')');
	inputs.append('circle')
		.attr('cx', 0)
		.attr('cy', slot_height / 2.0)
		.attr('r', slot_radius);
	inputs.append('text')
		.attr('x', 2 * slot_radius)
		.attr('y', slot_height / 2.0)
		.attr('text-anchor', 'start')
		.attr('dominant-baseline', 'middle')
		.text(d => d.name);

	var outputs = nodes.selectAll('.output')
		.data(d => d.outputs.map((e, i) => { return { 'name': e, 'index': i }; }))
		.enter().append('g')
		.attr('class', 'output')
		.attr('transform', d => 'translate(' + (node_width / 2.0) + ', ' + (title_height + separator_height + slot_height * d.index) + ')');
	outputs.append('circle')
		.attr('cx', node_width / 2.0)
		.attr('cy', slot_height / 2.0)
		.attr('r', slot_radius);
	outputs.append('text')
		.attr('x', node_width / 2.0 - 2 * slot_radius)
		.attr('y', slot_height / 2.0)
		.attr('text-anchor', 'end')
		.attr('dominant-baseline', 'middle')
		.text(d => d.name);

	// Connections between nodes
	var edges = root.selectAll('.edge')
		.data(graph.connections.map(d => { return { source: select(d.out, d.out_slot, false), target: select(d.in, d.in_slot, true) }; }))
		.enter().append('path')
		.attr('class', 'edge')
		.lower();

	// Avoid collisions
	var simulation = d3.forceSimulation()
		.nodes(graph.nodes)
		.force('bbox_collide', bbox_collisions(d => [
			[-node_padding - slot_radius * 2, -node_padding - slot_radius],
			[node_padding + node_width + slot_radius * 2, node_padding + slot_radius + title_height + separator_height + slot_height * Math.max(d.inputs.length, d.outputs.length)]
			]))
		.on('tick', update);
	zoom.translateBy(svg, (document.body.clientWidth - node_width) / 2.0, document.body.clientHeight / 2.0);

	// Initial layout
	var node_positions = flow_layout(graph);
	nodes.data().forEach(function(d)
	{
		d.x = 1.6 * node_width * node_positions.find(d.name).x;
	});

	function update()
	{
		function center_pos(d)
		{
			var c = d.select('circle');
			var p = svg.node().createSVGPoint(); p.x = c.attr('cx'); p.y = c.attr('cy');
			return p.matrixTransform(root.node().getCTM().inverse().multiply(c.node().getCTM()));
		}
		edges.attr('d', function(d)
			{
				var src = center_pos(d.source);
				var tgt = center_pos(d.target);
				return `M ${src.x} ${src.y} C ${src.x + edge_strength} ${src.y}, ${tgt.x - edge_strength} ${tgt.y}, ${tgt.x} ${tgt.y}`;
			});
		nodes.attr('transform', d => 'translate(' + d.x + ',' + d.y + ')');
	}

	function select(node, slot, is_input)
	{
		return svg.selectAll('.node')
			.filter(d => d.name === node)
			.selectAll(is_input ? '.input' : '.output')
			.filter(d => d.name === slot);
	}

	function start_drag(d)
	{
		if (!d3.event.active) simulation.alphaTarget(0.3).restart();
		d.fx = d.x;
		d.fy = d.y;
		d3.select(this).raise();
	}

	function on_drag(d)
	{
		d.fx = d3.event.x;
		d.fy = d3.event.y;
	}

	function end_drag(d)
	{
		if (!d3.event.active) simulation.alphaTarget(0);
		d.fx = null;
		d.fy = null;
	}
}
