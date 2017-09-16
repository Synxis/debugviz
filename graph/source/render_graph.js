function setup_graph_rendering(graph)
{
	"use strict";

	var node_width = 170;
	var node_padding = 10;
	var slot_height = 40;
	var slot_radius = 10;
	var title_height = 40;
	var separator_height = 10;
	var separator_count = 8;
	var edge_strength = 60;

	var svg = document.getElementsByTagName("svg")[0];
	function create_svg(parent, tag)
	{
		var e = document.createElementNS("http://www.w3.org/2000/svg", tag);
		parent.appendChild(e);
		return e;
	}
	var root = create_svg(svg, "g");
	function svg_point(x, y) { var p = svg.createSVGPoint(); p.x = x; p.y = y; return p; }
	var screen_to_root = (x, y) => svg_point(x, y).matrixTransform(root.getCTM().inverse());

	// Setup zoom
	var view_x = 0, view_y = 0, view_scale = 1;
	function update_view() { root.setAttribute("transform", "translate(" + view_x + ", " + view_y + ") scale(" + view_scale + ")"); }
	var zoom_drag_pos = null, zoom_init_pos = null;
	var drag_mouse_pos = null, drag_node_pos = null;
	function move_svg(e)
	{
		view_x = zoom_init_pos[0] + e.clientX - zoom_drag_pos[0];
		view_y = zoom_init_pos[1] + e.clientY - zoom_drag_pos[1];
		update_view();
		return false;
	}
	function stop_drag()
	{
		window.onmousemove = null;
		window.onmouseup = null;
		return false;
	}
	svg.onmousedown = function(e)
	{
		if(e.target !== svg) return false;
		zoom_drag_pos = [e.clientX, e.clientY];
		zoom_init_pos = [view_x, view_y];
		window.onmousemove = move_svg;
		window.onmouseup = stop_drag;
		return false;
	}
	svg.onwheel = function(e)
	{
		var old_scale = view_scale;
		view_scale = Math.min(3, Math.max(0.1, view_scale * 2 ** (-e.deltaY * 0.05)));
		var s = view_scale / old_scale;
		view_x = (view_x - e.clientX) * s + e.clientX;
		view_y = (view_y - e.clientY) * s + e.clientY;
		update_view();
	}
	view_x = (document.body.clientWidth - node_width) / 2;
	view_y = document.body.clientHeight / 2;
	update_view();

	// Setup graph
	flow_layout(graph).forEach(function(p)
	{
		var n = graph.nodes.find(m => m.name === p.name);
		n.x = 1.6 * node_width * p.x;
		n.y = 0;
	});
	for(var n of graph.nodes) setup_node(n);
	for(var e of graph.connections) setup_edge(e);
	var sim = createSimulation();

	function setup_node(node)
	{
		var g = create_svg(root, "g");
		g.setAttribute("class", "node");
		var r = create_svg(g, "rect");
		r.setAttribute("width", node_width);
		r.setAttribute("height", title_height + separator_height + slot_height * Math.max(node.inputs.length, node.outputs.length));
		var t = create_svg(g, "text");
		t.setAttribute('text-anchor', 'middle');
		t.setAttribute('dominant-baseline', 'middle');
		t.setAttribute('x', node_width / 2.0);
		t.setAttribute('y', title_height / 2.0);
		t.textContent = node.name;
		var l = create_svg(g, "line");
		l.setAttribute('x1', slot_radius)
		l.setAttribute('x2', node_width - slot_radius)
		l.setAttribute('y1', title_height)
		l.setAttribute('y2', title_height)
		l.setAttribute('stroke-dasharray', (node_width - 2 * slot_radius) / (2 * separator_count - 1));
		node.element = g;
		node.drag = false;
		function drag(e)
		{
			var mouse_pos = screen_to_root(e.clientX, e.clientY);
			node.x = drag_node_pos[0] + mouse_pos.x - drag_mouse_pos.x;
			node.y = drag_node_pos[1] + mouse_pos.y - drag_mouse_pos.y;
			return false;
		}
		function stop_node_drag(e)
		{
			node.drag = false;
			sim.start(0);
			drag_mouse_pos = null;
			return stop_drag();
		}
		g.onmousedown = function(e)
		{
			sim.start(0.3);
			node.drag = true;
			drag_mouse_pos = screen_to_root(e.clientX, e.clientY);
			drag_node_pos = [node.x, node.y];
			root.appendChild(g); // Raise node
			window.onmousemove = drag;
			window.onmouseup = stop_node_drag;
			return false;
		}

		for(var s = 0; s < node.inputs.length; s++) setup_slot(g, node.inputs, s, true);
		for(var s = 0; s < node.outputs.length; s++) setup_slot(g, node.outputs, s, false);
		function setup_slot(parent, slots, index, is_input)
		{
			var g = create_svg(parent, "g");
			g.setAttribute("class", is_input ? "input" : "output");
			g.setAttribute("transform", "translate(" + (is_input ? 0 : node_width / 2) + ", " + (title_height + separator_height + slot_height * index) + ")");
			var c = create_svg(g, "circle");
			c.setAttribute('cx', is_input ? 0 : node_width / 2.0);
			c.setAttribute('cy', slot_height / 2.0);
			c.setAttribute('r', slot_radius);
			var t = create_svg(g, "text");
			t.setAttribute('x', is_input ? 2 * slot_radius : node_width / 2.0 - 2 * slot_radius);
			t.setAttribute('y', slot_height / 2.0);
			t.setAttribute('text-anchor', is_input ? 'start' : "end");
			t.setAttribute('dominant-baseline', 'middle');
			t.textContent = slots[index];
			slots[index] = { name: t.textContent, element: g };
		}
	}
	function setup_edge(edge)
	{
		var e = create_svg(root, "path");
		e.setAttribute("class", "edge");
		edge.element = e;
		edge.source = graph.nodes.find(n => n.name === edge.out).outputs.find(s => s.name === edge.out_slot).element;
		edge.target = graph.nodes.find(n => n.name === edge.in).inputs.find(s => s.name === edge.in_slot).element;
		root.insertBefore(e, root.firstChild); // Lower the node
	}
	function update()
	{
		function center_pos(d)
		{
			var c = d.getElementsByTagName("circle")[0];
			return svg_point(+c.getAttribute("cx"), +c.getAttribute("cy")).matrixTransform(root.getCTM().inverse().multiply(c.getCTM()));
		}
		for(var n of graph.nodes)
			n.element.setAttribute('transform', 'translate(' + n.x + ',' + n.y + ')');
		for(var e of graph.connections)
		{
			var src = center_pos(e.source);
			var tgt = center_pos(e.target);
			e.element.setAttribute("d", `M ${src.x} ${src.y} C ${src.x + edge_strength} ${src.y}, ${tgt.x - edge_strength} ${tgt.y}, ${tgt.x} ${tgt.y}`);
		}
	}
	function createSimulation()
	{
		var alpha = 1;
		var alphaMin = 0.001;
		var alphaDecay = 1 - Math.pow(alphaMin, 1 / 300);
		var alphaTarget = 0;
		var velocityDecay = 0.6;
		var deltaTime = 20;
		var timer;

		for(var n of graph.nodes) n.vx = n.vy = 0;

		var bbox = bbox_collisions(d => [
			[-node_padding - slot_radius * 2, -node_padding - slot_radius],
			[node_padding + node_width + slot_radius * 2, node_padding + slot_radius + title_height + separator_height + slot_height * Math.max(d.inputs.length, d.outputs.length)]
			]);
		bbox.initialize(graph.nodes);

		function stop() { clearInterval(timer); };
		function step()
		{
			alpha += (alphaTarget - alpha) * alphaDecay;
			bbox(alpha);
			for(var n of graph.nodes)
			{
				if(n.drag) { n.vx = n.vy = 0; continue; }
				n.x += n.vx *= velocityDecay;
				n.y += n.vy *= velocityDecay;
			}
			update();
			if(alpha < alphaMin) stop();
		}
		function start(a) { alphaTarget = a; stop(); timer = setInterval(step, deltaTime); };

		update();
		start(0);
		return { start: start, stop: stop };
	}
}
