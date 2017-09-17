// BSD 3-Clause Licence //////////////////////////////////////////////////////////////////////// //
// Copyright (c) 2017 Thibault Lescoat, All rights reserved.                                     //
//                                                                                               //
// Redistribution and use in source and binary forms, with or without modification, are          //
// permitted provided that the following conditions are met:                                     //
//                                                                                               //
// * Redistributions of source code must retain the above copyright notice, this list of         //
//   conditions and the following disclaimer.                                                    //
//                                                                                               //
// * Redistributions in binary form must reproduce the above copyright notice, this list of      //
//   conditions and the following disclaimer in the documentation and/or other materials         //
//   provided with the distribution.                                                             //
//                                                                                               //
// * Neither the name of the copyright holder nor the names of its contributors may be used to   //
//   endorse or promote products derived from this software without specific prior written       //
//   permission.                                                                                 //
//                                                                                               //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   //
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    //
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE //
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    //
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     //
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF          //
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                    //
// ///////////////////////////////////////////////////////////////////////////////////////////// //
function flow_layout(graph)
{
	'use strict';

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