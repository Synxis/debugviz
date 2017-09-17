// WARNING: auto-generated, do not modify !

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
#pragma once

#include <iostream>

#ifndef DEBUGVIZ_NO_FLOW_GRAPH

#include <type_traits>

namespace debugviz
{
namespace detail
{
	template<typename T> struct to_void { using type = void; };
	template<typename T> using void_t = typename to_void<T>::type;
	template<typename T> using no_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

	template<typename, typename T, typename = void_t<T>>
	struct is_streamable : std::false_type {};
	template<typename S, typename T>
	struct is_streamable<S, T, void_t<decltype(std::declval<S&>() << std::declval<no_cvref<T>>())>>
		: std::true_type {};

	template<class S, class T> using streamable_name     = is_streamable<S, decltype(no_cvref<T>::name)>;
	template<class S, class T> using streamable_out      = is_streamable<S, decltype(no_cvref<T>::out)>;
	template<class S, class T> using streamable_out_slot = is_streamable<S, decltype(no_cvref<T>::out_slot)>;
	template<class S, class T> using streamable_in       = is_streamable<S, decltype(no_cvref<T>::in)>;
	template<class S, class T> using streamable_in_slot  = is_streamable<S, decltype(no_cvref<T>::in_slot)>;
}

	/// Outputs a html page to visualize a flow graph
	/** This function serialize a flow graph into a stream, as an html page that can be viewed on a
		standard web browser (it does not need Internet connectivity, as all needed scripts/CSS are
		fully included in the html). Please bear in mind that no html escaping will be done!

		A flow graph is a graph where nodes possess *input* and *output* ports (or slots), which
		are used as endpoints for the edges of the graph. Connections can only go from an output
		slot to an input slot, and can be viewed as "data comes out of a specific node at some port
		and is fed into the specified input of another node".

		In the following by 'range' we mean a type that can be iterated using a
		range-based for loop, and by 'streamable' we mean a type that can be serialized into the
		given stream.

		\note for std::ostream, there exist a wrapper so you can stream a flow graph as usual:
		`stream << bdgviz::flow_graph(title, nodes, connections);`.

		\param stream The stream in which to serialize the flow graph html visualization
		\param title Title of the html page (must be streamable)
		\param nodes Nodes of the graph. This structure must be a range, and elements must have:
			- a 'name' field, streamable
			- an 'inputs' field, range of streamables
			- an 'outputs' field, range of streamables
		\param connections Links between nodes in the graph. This must be a range, and its elements
			must have four streamable fields: 'out', 'out_slot', 'in', 'in_slot'
		\return The given stream
	*/
	template<typename S, typename T, typename N, typename C>
	S& write_flow_graph(S& stream, const T& title, const N& nodes, const C& connections)
	{
		auto structure = [&]()
		{
			stream << "nodes: [";
			for(const auto& n : nodes)
			{
				static_assert(detail::streamable_name<S, decltype(n)>::value,
					"Nodes must have a 'name' field that supports 'stream << node.name'");

				stream << "{ name: \"" << n.name << "\", inputs: [";
				for(const auto& s : n.inputs)
				{
					static_assert(detail::is_streamable<S, decltype(s)>::value,
						"Node inputs slots must support 'stream << slot'");
					stream << "\"" << s << "\",";
				}
				stream << "], outputs: [";
				for(const auto& s : n.outputs)
				{
					static_assert(detail::is_streamable<S, decltype(s)>::value,
						"Node outputs slots must support 'stream << slot'");
					stream << "\"" << s << "\",";
				}
				stream << "] },";
			}
			stream << "], connections: [";
			for(const auto& c : connections)
			{
				static_assert(detail::streamable_out<S, decltype(c)>::value,
					"Connections must have a 'out' field that supports 'stream << connection.out'");
				static_assert(detail::streamable_out_slot<S, decltype(c)>::value,
					"Connections must have a 'out_slot' field that supports 'stream << connection.out_slot'");
				static_assert(detail::streamable_in<S, decltype(c)>::value,
					"Connections must have a 'in' field that supports 'stream << connection.in'");
				static_assert(detail::streamable_in_slot<S, decltype(c)>::value,
					"Connections must have a 'in_slot' field that supports 'stream << connection.in_slot'");

				stream << "{ out: \"" << c.out << "\", out_slot: \"" << c.out_slot
					<< "\", in: \"" << c.in << "\", in_slot: \"" << c.in_slot << "\" },";
			}
			stream << "]";
		};

		stream << "<!DOCTYPE html><meta charset='utf-8'><script>function flow_layout(t){'use stric"
			"t';function e(e){return[...new Set(t.connections.filter(t=>t.out===e).map(t=>t.in))]}"
			"function n(e){return[...new Set(t.connections.filter(t=>t.in===e).map(t=>t.out))]}fun"
			"ction r(t){return a.findIndex(e=>e.name===t)}function o(t){var n=r(t);for(var i of e("
			"t)){var u=r(i);i===t||(a[u].x,0)||(a[u].x=Math.max(a[n].x+1,isNaN(a[u].x)?-1/0:a[u].x"
			"),o(i))}}function i(t){var e=r(t);for(var o of n(t)){var u=r(o);o===t||(a[u].x,0)||(a"
			"[u].x=Math.min(a[e].x-1,isNaN(a[u].x)?1/0:a[u].x),i(o))}}for(var a=t.nodes.map(t=>({n"
			"ame:t.name,x:NaN,y:NaN})),u=t.nodes.filter(t=>0===n(t.name).length).map(t=>t.name),s="
			"t.nodes.filter(t=>0===e(t.name).length).map(t=>t.name),l=0;l<u.length;l++)a[r(u[l])]."
			"x=0,o(u[l]);for(l=0;l<s.length;l++)i(s[l]);var f=a.map(t=>t.x),c=(Math.max(...f)-Math"
			".min(...f))/2;for(var m of a)m.x-=c;return a.index=r,a.find=function(t){return a[a.in"
			"dex(t)]},a}function bbox_collisions(t){'use strict';function e(){for(var t=0;t<o.leng"
			"th;t++)for(var e=t+1;e<o.length;e++){var a=n([[r[t].x+o[t][0][0],r[t].y+o[t][0][1]],["
			"r[t].x+o[t][1][0],r[t].y+o[t][1][1]]],[[r[e].x+o[e][0][0],r[e].y+o[e][0][1]],[r[e].x+"
			"o[e][1][0],r[e].y+o[e][1][1]]]);r[t].vx+=i*a.fA[0],r[t].vy+=i*a.fA[1],r[e].vx+=i*a.fB"
			"[0],r[e].vy+=i*a.fB[1]}}function n(t,e){var n=e[1][0]-t[0][0],r=t[1][0]-e[0][0],o=e[1"
			"][1]-t[0][1],i=t[1][1]-e[0][1],a=n>0&&r>0,u=o>0&&i>0;if(a&&u){var s=n>r?r:-n,l=o>i?i:"
			"-o;return Math.abs(s)<=Math.abs(l)?{fA:[-s/(t[1][0]-t[0][0]),0],fB:[s/(e[1][0]-e[0][0"
			"]),0]}:{fA:[0,-l/(t[1][1]-t[0][1])],fB:[0,l/(e[1][1]-e[0][1])]}}return{fA:[0,0],fB:[0"
			",0]}}var r,o,i=10;return e.initialize=function(e){var n,i=(r=e).length;for(o=Array(i)"
			",n=0;n<i;++n)o[n]=t(r[n],n,r)},e}function setup_graph_rendering(t){'use strict';funct"
			"ion e(t,e){var n=document.createElementNS('http://www.w3.org/2000/svg',e);return t.ap"
			"pendChild(n),n}function n(t,e){var n=d.createSVGPoint();return n.x=t,n.y=e,n}function"
			" r(){v.setAttribute('transform','translate('+g+', '+h+') scale('+b+')')}function o(t)"
			"{return g=A[0]+t.clientX-y[0],h=A[1]+t.clientY-y[1],r(),!1}function i(){return window"
			".onmousemove=null,window.onmouseup=null,!1}function a(){function e(t){var e=t.getElem"
			"entsByTagName('circle')[0];return n(+e.getAttribute('cx'),+e.getAttribute('cy')).matr"
			"ixTransform(v.getCTM().inverse().multiply(e.getCTM()))}for(var r of t.nodes)r.element"
			".setAttribute('transform','translate('+r.x+','+r.y+')');for(var o of t.connections){v"
			"ar i=e(o.source),a=e(o.target);o.element.setAttribute('d',`M ${i.x} ${i.y} C ${i.x+m}"
			" ${i.y}, ${a.x-m} ${a.y}, ${a.x} ${a.y}`)}}var u=170,s=40,l=10,f=40,c=10,m=60,d=docum"
			"ent.getElementsByTagName('svg')[0],v=e(d,'g'),x=(t,e)=>n(t,e).matrixTransform(v.getCT"
			"M().inverse()),g=0,h=0,b=1,y=null,A=null,p=null,w=null;d.onmousedown=function(t){retu"
			"rn t.target===d&&(y=[t.clientX,t.clientY],A=[g,h],window.onmousemove=o,window.onmouse"
			"up=i,!1)},d.onwheel=function(t){var e=b,n=(b=Math.min(3,Math.max(.1,b*2**(.05*-t.delt"
			"aY))))/e;g=(g-t.clientX)*n+t.clientX,h=(h-t.clientY)*n+t.clientY,r()},g=(document.bod"
			"y.clientWidth-u)/2,h=document.body.clientHeight/2,r(),flow_layout(t).forEach(function"
			"(e){var n=t.nodes.find(t=>t.name===e.name);n.x=1.6*u*e.x,n.y=0});for(var M of t.nodes"
			")!function(t){function n(e){var n=x(e.clientX,e.clientY);return t.x=w[0]+n.x-p.x,t.y="
			"w[1]+n.y-p.y,!1}function r(e){return t.drag=!1,C.start(0),p=null,i()}function o(t,n,r"
			",o){var i=e(t,'g');i.setAttribute('class',o?'input':'output'),i.setAttribute('transfo"
			"rm','translate('+(o?0:u/2)+', '+(f+c+s*r)+')');var a=e(i,'circle');a.setAttribute('cx"
			"',o?0:u/2),a.setAttribute('cy',s/2),a.setAttribute('r',l);var m=e(i,'text');m.setAttr"
			"ibute('x',o?2*l:u/2-2*l),m.setAttribute('y',s/2),m.setAttribute('text-anchor',o?'star"
			"t':'end'),m.setAttribute('dominant-baseline','middle'),m.textContent=n[r],n[r]={name:"
			"m.textContent,element:i}}var a=e(v,'g');a.setAttribute('class','node');var m=e(a,'rec"
			"t');m.setAttribute('width',u),m.setAttribute('height',f+c+s*Math.max(t.inputs.length,"
			"t.outputs.length));var d=e(a,'text');d.setAttribute('text-anchor','middle'),d.setAttr"
			"ibute('dominant-baseline','middle'),d.setAttribute('x',u/2),d.setAttribute('y',f/2),d"
			".textContent=t.name;var g=e(a,'line');for(g.setAttribute('x1',l),g.setAttribute('x2',"
			"u-l),g.setAttribute('y1',f),g.setAttribute('y2',f),g.setAttribute('stroke-dasharray',"
			"(u-2*l)/15),t.element=a,t.drag=!1,a.onmousedown=function(e){return C.start(.3),t.drag"
			"=!0,p=x(e.clientX,e.clientY),w=[t.x,t.y],v.appendChild(a),window.onmousemove=n,window"
			".onmouseup=r,!1},h=0;h<t.inputs.length;h++)o(a,t.inputs,h,!0);for(var h=0;h<t.outputs"
			".length;h++)o(a,t.outputs,h,!1)}(M);for(var N of t.connections)!function(n){var r=e(v"
			",'path');r.setAttribute('class','edge'),n.element=r,n.source=t.nodes.find(t=>t.name=="
			"=n.out).outputs.find(t=>t.name===n.out_slot).element,n.target=t.nodes.find(t=>t.name="
			"==n.in).inputs.find(t=>t.name===n.in_slot).element,v.insertBefore(r,v.firstChild)}(N)"
			";var C=function(){function e(){clearInterval(o)}function n(){b(i+=(v-i)*d);for(var n "
			"of t.nodes)n.drag?n.vx=n.vy=0:(n.x+=n.vx*=x,n.y+=n.vy*=x);a(),i<m&&e()}function r(t){"
			"v=t,e(),o=setInterval(n,g)}var o,i=1,m=.001,d=1-Math.pow(m,1/300),v=0,x=.6,g=20;for(v"
			"ar h of t.nodes)h.vx=h.vy=0;var b=bbox_collisions(t=>[[-10-2*l,-10-l],[10+u+2*l,10+l+"
			"f+c+s*Math.max(t.inputs.length,t.outputs.length)]]);return b.initialize(t.nodes),a(),"
			"r(0),{start:r,stop:e}}()}</script><style>body,html,svg{margin:0;width:100%;height:100"
			"%;overflow:hidden}</style><title>" << title << "</title><body><svg><style>.edge,.node"
			"{stroke:#555;fill:#555;stroke-width:3}.edge,.node>line,.node>rect{fill:#ffffff88}circ"
			"le{stroke:#fff}.node text{stroke-width:1;font-family:Verdana;cursor:default}</style><"
			"/svg><script>setup_graph_rendering({ ";
		structure();
		stream << " });</script></body>";

		return stream;
	}

namespace detail
{
	template<typename T, typename N, typename C>
	struct flow_graph_data
	{
		const T& title;
		const N& nodes;
		const C& connections;
	};

	template<typename T, typename N, typename C>
	std::ostream& operator<<(std::ostream& os, const flow_graph_data<T, N, C>& g)
	{
		return write_flow_graph<std::ostream, T, N, C>(os, g.title, g.nodes, g.connections);
	}
}

	/// Convenience function for use with standard output streams
	/** A wrapper on write_flow_graph for std::ostream allowing constructs like:
		`std::cout << debugviz::flow_graph("Some title", nodes, connections);`.
		\see write_flow_graph for a more complete explanation of the parameters. */
	template<typename T, typename N, typename C>
	detail::flow_graph_data<T, N, C> flow_graph(const T& title, const N& nodes, const C& connections)
	{
		return { title, nodes, connections };
	}
}

#else

namespace debugviz
{
namespace detail
{
	struct empty {};
	std::ostream& operator<<(std::ostream& os, empty) { return os; }
}
	template<typename T, typename N, typename C>
	detail::empty flow_graph(const T&, const N&, const C&) { return {}; }

	template<typename S, typename T, typename N, typename C>
	S& write_flow_graph(S& s, const T&, const N&, const C&) { return s; }
}

#endif
