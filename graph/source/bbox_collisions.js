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
function bbox_collisions(bbox)
{
	'use strict';

	var nodes, boxes, strength = 10;
	function force()
	{
		for(var i = 0; i < boxes.length; i++)
			for(var j = i + 1; j < boxes.length; j++)
			{
				var bA = [
					[nodes[i].x + boxes[i][0][0], nodes[i].y + boxes[i][0][1]],
					[nodes[i].x + boxes[i][1][0], nodes[i].y + boxes[i][1][1]]
				];
				var bB = [
					[nodes[j].x + boxes[j][0][0], nodes[j].y + boxes[j][0][1]],
					[nodes[j].x + boxes[j][1][0], nodes[j].y + boxes[j][1][1]]
				];
				var cf = collision_force(bA, bB);
				nodes[i].vx += strength * cf.fA[0];
				nodes[i].vy += strength * cf.fA[1];
				nodes[j].vx += strength * cf.fB[0];
				nodes[j].vy += strength * cf.fB[1];
			}
	}
	function collision_force(bA, bB)
	{
		var left   = bB[1][0] - bA[0][0];
		var right  = bA[1][0] - bB[0][0];
		var top    = bB[1][1] - bA[0][1];
		var bottom = bA[1][1] - bB[0][1];
		var intersectX = (left > 0 && right > 0);
		var intersectY = (top > 0 && bottom > 0);
		if(intersectX && intersectY)
		{
			var dX = left > right ? right : -left;
			var dY = top > bottom ? bottom : -top;
			if(Math.abs(dX) <= Math.abs(dY))
			{
				var wA = bA[1][0] - bA[0][0];
				var wB = bB[1][0] - bB[0][0];
				return { fA: [-dX / wA, 0], fB: [dX / wB, 0] };
			}
			else
			{
				var hA = bA[1][1] - bA[0][1];
				var hB = bB[1][1] - bB[0][1];
				return { fA: [0, -dY / hA], fB: [0, dY / hB] };
			}
		}
		return { fA: [0, 0], fB: [0, 0] };
	}
	force.initialize = function(_)
	{
		var i, n = (nodes = _).length; boxes = new Array(n);
		for(i = 0; i < n; ++i) boxes[i] = bbox(nodes[i], i, nodes);
	};
	return force;
}
